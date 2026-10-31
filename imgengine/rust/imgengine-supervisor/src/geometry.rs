//! Pure, deterministic physical-sheet layout and crop geometry for MVP 1.0.
//!
//! This module performs no decoding, allocation of image buffers, filesystem
//! work, or native calls. It converts validated physical dimensions to pixels
//! and produces geometry that later trusted layers must independently validate.

use std::fmt;

/// Stable validation failures for physical layout and crop calculations.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum GeometryError {
    InvalidDimension,
    InvalidDpi,
    InvalidCopies,
    DoesNotFit,
    ArithmeticOverflow,
    ResourceLimit,
}

/// Convert a finite positive millimetre measurement to pixels using the
/// contract's deterministic half-up rounding rule.
pub fn mm_to_px(mm: f64, dpi: u32) -> Result<u32, GeometryError> {
    if !mm.is_finite() || mm <= 0.0 {
        return Err(GeometryError::InvalidDimension);
    }
    if dpi == 0 {
        return Err(GeometryError::InvalidDpi);
    }
    let pixels = (mm * f64::from(dpi) / 25.4 + 0.5).floor();
    if !pixels.is_finite() || pixels < 1.0 || pixels > f64::from(u32::MAX) {
        return Err(GeometryError::ArithmeticOverflow);
    }
    Ok(pixels as u32)
}

/// Convert a finite non-negative millimetre spacing to pixels.
pub fn mm_spacing_to_px(mm: f64, dpi: u32) -> Result<u32, GeometryError> {
    if !mm.is_finite() || mm < 0.0 {
        return Err(GeometryError::InvalidDimension);
    }
    if dpi == 0 {
        return Err(GeometryError::InvalidDpi);
    }
    let pixels = (mm * f64::from(dpi) / 25.4 + 0.5).floor();
    if !pixels.is_finite() || pixels < 0.0 || pixels > f64::from(u32::MAX) {
        return Err(GeometryError::ArithmeticOverflow);
    }
    Ok(pixels as u32)
}

/// Requested sheet geometry. All fields are resolved integer pixels.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct MvpLayoutInput {
    pub sheet_width: u32,
    pub sheet_height: u32,
    pub photo_width: u32,
    pub photo_height: u32,
    pub copies: u32,
    pub margin: u32,
    pub gap_x: u32,
    pub gap_y: u32,
}

/// One photo placement on the requested sheet.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct Placement {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
}

/// Deterministic single-sheet plan with a centred occupied grid.
#[derive(Clone, Debug, Eq, PartialEq)]
pub struct MvpLayoutPlan {
    pub sheet_width: u32,
    pub sheet_height: u32,
    pub max_columns: u32,
    pub max_rows: u32,
    pub capacity: u32,
    pub columns_used: u32,
    pub rows_used: u32,
    pub placements: Vec<Placement>,
}

impl MvpLayoutPlan {
    const MAX_COPIES: u32 = 64;
    const MAX_DIMENSION: u32 = 4096;
    const MAX_CANVAS_PIXELS: u64 = 10_000_000;

    /// Calculates maximum physical capacity and rejects copy overflow.
    ///
    /// The solver uses the maximum feasible column count, row-major placement,
    /// and centres the occupied grid within the printable area. It never
    /// changes the requested copy count or sheet dimensions.
    pub fn solve(input: MvpLayoutInput) -> Result<Self, GeometryError> {
        if input.sheet_width == 0
            || input.sheet_height == 0
            || input.photo_width == 0
            || input.photo_height == 0
        {
            return Err(GeometryError::InvalidDimension);
        }
        if input.copies == 0 {
            return Err(GeometryError::InvalidCopies);
        }
        if input.copies > Self::MAX_COPIES
            || input.sheet_width > Self::MAX_DIMENSION
            || input.sheet_height > Self::MAX_DIMENSION
            || u64::from(input.sheet_width) * u64::from(input.sheet_height) > Self::MAX_CANVAS_PIXELS
        {
            return Err(GeometryError::ResourceLimit);
        }

        let twice_margin = input.margin.checked_mul(2).ok_or(GeometryError::ArithmeticOverflow)?;
        let printable_width = input.sheet_width.checked_sub(twice_margin).ok_or(GeometryError::DoesNotFit)?;
        let printable_height = input.sheet_height.checked_sub(twice_margin).ok_or(GeometryError::DoesNotFit)?;

        let columns_numerator = printable_width.checked_add(input.gap_x).ok_or(GeometryError::ArithmeticOverflow)?;
        let columns_denominator = input.photo_width.checked_add(input.gap_x).ok_or(GeometryError::ArithmeticOverflow)?;
        let rows_numerator = printable_height.checked_add(input.gap_y).ok_or(GeometryError::ArithmeticOverflow)?;
        let rows_denominator = input.photo_height.checked_add(input.gap_y).ok_or(GeometryError::ArithmeticOverflow)?;
        let max_columns = columns_numerator / columns_denominator;
        let max_rows = rows_numerator / rows_denominator;
        if max_columns == 0 || max_rows == 0 {
            return Err(GeometryError::DoesNotFit);
        }
        let capacity = max_columns.checked_mul(max_rows).ok_or(GeometryError::ArithmeticOverflow)?;
        if input.copies > capacity {
            return Err(GeometryError::DoesNotFit);
        }

        let columns_used = input.copies.min(max_columns);
        let rows_used = input.copies
            .checked_add(columns_used - 1)
            .ok_or(GeometryError::ArithmeticOverflow)? / columns_used;
        let grid_width = columns_used.checked_mul(input.photo_width)
            .and_then(|v| v.checked_add((columns_used - 1).checked_mul(input.gap_x)?))
            .ok_or(GeometryError::ArithmeticOverflow)?;
        let grid_height = rows_used.checked_mul(input.photo_height)
            .and_then(|v| v.checked_add((rows_used - 1).checked_mul(input.gap_y)?))
            .ok_or(GeometryError::ArithmeticOverflow)?;
        if grid_width > printable_width || grid_height > printable_height {
            return Err(GeometryError::DoesNotFit);
        }
        let start_x = input.margin.checked_add((printable_width - grid_width) / 2)
            .ok_or(GeometryError::ArithmeticOverflow)?;
        let start_y = input.margin.checked_add((printable_height - grid_height) / 2)
            .ok_or(GeometryError::ArithmeticOverflow)?;

        let capacity_usize = usize::try_from(input.copies).map_err(|_| GeometryError::ArithmeticOverflow)?;
        let mut placements = Vec::with_capacity(capacity_usize);
        for index in 0..input.copies {
            let row = index / columns_used;
            let column = index % columns_used;
            let x = start_x
                .checked_add(column.checked_mul(input.photo_width).ok_or(GeometryError::ArithmeticOverflow)?)
                .and_then(|v| v.checked_add(column.checked_mul(input.gap_x)?))
                .ok_or(GeometryError::ArithmeticOverflow)?;
            let y = start_y
                .checked_add(row.checked_mul(input.photo_height).ok_or(GeometryError::ArithmeticOverflow)?)
                .and_then(|v| v.checked_add(row.checked_mul(input.gap_y)?))
                .ok_or(GeometryError::ArithmeticOverflow)?;
            placements.push(Placement { x, y, width: input.photo_width, height: input.photo_height });
        }

        Ok(Self {
            sheet_width: input.sheet_width,
            sheet_height: input.sheet_height,
            max_columns,
            max_rows,
            capacity,
            columns_used,
            rows_used,
            placements,
        })
    }
}

/// Integer source rectangle selected for deterministic centred crop-to-fill.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct CropRect {
    pub x: u32,
    pub y: u32,
    pub width: u32,
    pub height: u32,
}

/// Computes the largest centred source rectangle with the target aspect ratio.
/// Crop dimensions use integer floor division; odd excess pixels are assigned
/// deterministically to the right or bottom side.
pub fn centered_crop(
    source_width: u32,
    source_height: u32,
    target_width: u32,
    target_height: u32,
) -> Result<CropRect, GeometryError> {
    if source_width == 0 || source_height == 0 || target_width == 0 || target_height == 0 {
        return Err(GeometryError::InvalidDimension);
    }

    let source_cross = u64::from(source_width)
        .checked_mul(u64::from(target_height))
        .ok_or(GeometryError::ArithmeticOverflow)?;
    let target_cross = u64::from(target_width)
        .checked_mul(u64::from(source_height))
        .ok_or(GeometryError::ArithmeticOverflow)?;

    let (width, height) = if source_cross > target_cross {
        let width = u64::from(source_height)
            .checked_mul(u64::from(target_width))
            .ok_or(GeometryError::ArithmeticOverflow)?
            / u64::from(target_height);
        (u32::try_from(width).map_err(|_| GeometryError::ArithmeticOverflow)?, source_height)
    } else if source_cross < target_cross {
        let height = u64::from(source_width)
            .checked_mul(u64::from(target_height))
            .ok_or(GeometryError::ArithmeticOverflow)?
            / u64::from(target_width);
        (source_width, u32::try_from(height).map_err(|_| GeometryError::ArithmeticOverflow)?)
    } else {
        (source_width, source_height)
    };

    if width == 0 || height == 0 || width > source_width || height > source_height {
        return Err(GeometryError::DoesNotFit);
    }
    Ok(CropRect {
        x: (source_width - width) / 2,
        y: (source_height - height) / 2,
        width,
        height,
    })
}

impl fmt::Display for GeometryError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        f.write_str(match self {
            Self::InvalidDimension => "invalid_dimension",
            Self::InvalidDpi => "invalid_dpi",
            Self::InvalidCopies => "invalid_copies",
            Self::DoesNotFit => "does_not_fit",
            Self::ArithmeticOverflow => "arithmetic_overflow",
            Self::ResourceLimit => "resource_limit",
        })
    }
}

impl std::error::Error for GeometryError {}

#[cfg(test)]
mod tests {
    use super::*;

    fn input(copies: u32) -> MvpLayoutInput {
        MvpLayoutInput {
            sheet_width: 1000,
            sheet_height: 700,
            photo_width: 200,
            photo_height: 300,
            copies,
            margin: 20,
            gap_x: 10,
            gap_y: 10,
        }
    }

    #[test]
    fn mm_conversion_uses_half_up_rounding() {
        assert_eq!(mm_to_px(25.4, 300), Ok(300));
        assert_eq!(mm_to_px(35.0, 300), Ok(413));
        assert_eq!(mm_to_px(45.0, 300), Ok(531));
        assert_eq!(mm_spacing_to_px(0.0, 300), Ok(0));
        assert_eq!(mm_to_px(0.04233333333333333, 300), Ok(1)); // exact 0.5 rounds up
        assert_eq!(mm_to_px(0.12699999999999997, 300), Ok(1)); // just below 1.5 rounds down
        assert_eq!(mm_to_px(0.127, 300), Ok(2));
        assert_eq!(mm_to_px(f64::NAN, 300), Err(GeometryError::InvalidDimension));
        assert_eq!(mm_to_px(1.0, 0), Err(GeometryError::InvalidDpi));
    }

    #[test]
    fn exact_capacity_and_partial_final_row_are_deterministic() {
        let plan = MvpLayoutPlan::solve(input(8)).expect("fits");
        assert_eq!((plan.max_columns, plan.max_rows, plan.capacity), (4, 2, 8));
        assert_eq!((plan.columns_used, plan.rows_used), (4, 2));
        assert_eq!(plan.placements.len(), 8);
        assert_eq!(plan.placements[0].x, 85);
        assert_eq!(plan.placements[0].y, 45);
        let partial = MvpLayoutPlan::solve(input(5)).expect("partial row fits");
        assert_eq!((partial.columns_used, partial.rows_used), (4, 2));
        assert_eq!(partial.placements[4].x, 85);
        assert_eq!(partial.placements[4].y, 355);
    }

    #[test]
    fn a4_300_dpi_sheet_is_admitted_under_the_separate_p12_budget() {
        let plan = MvpLayoutPlan::solve(MvpLayoutInput {
            sheet_width: 2480,
            sheet_height: 3508,
            photo_width: 413,
            photo_height: 531,
            copies: 8,
            margin: 118,
            gap_x: 24,
            gap_y: 24,
        })
        .expect("A4 at 300 DPI fits the P12 10 MP geometry budget");
        assert_eq!((plan.sheet_width, plan.sheet_height), (2480, 3508));
        assert_eq!(plan.placements.len(), 8);

        let mut at_limit = MvpLayoutInput {
            sheet_width: 4096,
            sheet_height: 2441,
            photo_width: 1,
            photo_height: 1,
            copies: 1,
            margin: 0,
            gap_x: 0,
            gap_y: 0,
        };
        assert!(MvpLayoutPlan::solve(at_limit).is_ok());
        at_limit.sheet_height = 2442;
        assert_eq!(MvpLayoutPlan::solve(at_limit), Err(GeometryError::ResourceLimit));
    }

    #[test]
    fn overflow_and_zero_fit_are_rejected_without_truncation() {
        assert_eq!(MvpLayoutPlan::solve(input(9)), Err(GeometryError::DoesNotFit));
        let mut too_small = input(1);
        too_small.sheet_width = 100;
        assert_eq!(MvpLayoutPlan::solve(too_small), Err(GeometryError::DoesNotFit));
    }

    #[test]
    fn grid_is_centred_and_preserves_requested_sheet_canvas() {
        let plan = MvpLayoutPlan::solve(input(3)).expect("fits");
        assert_eq!((plan.sheet_width, plan.sheet_height), (1000, 700));
        let last = plan.placements[2];
        assert!(last.x + last.width <= plan.sheet_width - 20);
        assert!(last.y + last.height <= plan.sheet_height - 20);
    }

    #[test]
    fn crop_handles_landscape_portrait_square_and_odd_excess() {
        assert_eq!(centered_crop(400, 200, 1, 1), Ok(CropRect { x: 100, y: 0, width: 200, height: 200 }));
        assert_eq!(centered_crop(200, 400, 1, 1), Ok(CropRect { x: 0, y: 100, width: 200, height: 200 }));
        assert_eq!(centered_crop(101, 100, 1, 1), Ok(CropRect { x: 0, y: 0, width: 100, height: 100 }));
        assert_eq!(centered_crop(100, 100, 1, 1), Ok(CropRect { x: 0, y: 0, width: 100, height: 100 }));
        assert_eq!(centered_crop(1, 1000, 1000, 1), Err(GeometryError::DoesNotFit));
    }

    #[test]
    fn crop_and_layout_reject_invalid_dimensions_and_checked_overflow() {
        assert_eq!(centered_crop(0, 1, 1, 1), Err(GeometryError::InvalidDimension));
        let mut huge = input(1);
        huge.margin = u32::MAX;
        assert_eq!(MvpLayoutPlan::solve(huge), Err(GeometryError::ArithmeticOverflow));
        let mut zero = input(1);
        zero.photo_width = 0;
        assert_eq!(MvpLayoutPlan::solve(zero), Err(GeometryError::InvalidDimension));
        let mut too_many = input(65);
        too_many.sheet_width = 4096;
        too_many.sheet_height = 4096;
        assert_eq!(MvpLayoutPlan::solve(too_many), Err(GeometryError::ResourceLimit));
    }
}
