# 📸 IMGENGINE CLI

### Professional Photo Layout & Print Pipeline

[![License: MIT](https://img.shields.io)](https://opensource.org)
![Build Status](https://img.shields.io)
![Platform](https://img.shields.io)

`imgengine_cli` is a high-performance command-line tool designed for generating print-ready photo sheets. Whether you are running a professional print shop or automating a studio workflow, it provides a robust pipeline for passport photos, ID cards, and bulk layouts.

---

## 🔷 Overview

Designed for speed and precision, the engine handles the heavy lifting of print preparation:

* **Automatic Layout Generation:** Smart grid placement.
* **DPI-Aware Resizing:** Ensures physical dimensions match print requirements.
* **Bleed-Safe Printing:** Eliminates white edges during the cutting process.
* **Memory Efficient:** Optimized with a custom memory pool and SIMD (AVX2).

---

## ⚡ Features

* [x] **Grid Engine:** Configurable rows × columns.
* [x] **Smart Scaling:** Automatic "fit to page" with aspect ratio preservation.
* [x] **Pro Print Tools:** Per-photo crop marks and bleed support.
* [x] **Optimized Performance:** SIMD-accelerated image processing.
* [x] **Format Versatility:** Export to high-quality PNG or print-ready PDF.
* [x] **Scriptable:** CLI-first design for easy integration into automation pipelines.

---

## 📦 Installation

### Build from Source

Ensure you have `cmake` and a C++ compiler installed.

```bash
# Clone the repository
git clone https://github.com/Rofikali/imgengine
cd imgengine

# Build the project
mkdir build && cd build
cmake ..
make

# Run help
./imgengine_cli --help


## Usage Guide
    Basic Syntax
    bash
    ./imgengine_cli --input <file> [--output <file>] [OPTIONS]
    Use code with caution.

##  Input & Output
    The output format is auto-detected based on the file extension.
    Type Formats Supported
    Input JPG, PNG
    Output PNG, PDF
## ⚙️ Configuration Options
    🔹 Required & Global
    Option Description
    --input <file> Path to the source image.
    --output <file> Path to save result (Default: output.png).
    --quiet Suppress console output.
    --help Show help message.
## 🔹 Layout & Sizing
    Option Description Default
    --cols Number of columns 2
    --rows Number of rows 3
    --gap Gap between photos (pixels) 15
    --padding Page margin (pixels) 20
    --width Photo width (cm) 4.5
    --height Photo height (cm) 3.5
    --dpi Print DPI 300
## 🔹 Professional Print Settings
    Option Description Default
    --bleed Extra pixels for cutting safety 10
    --crop-mark Length of crop marks 20
    --crop-offset Distance from image edge (ISO) 8
    --border Border thickness (pixels) 2
## 🧠 How It Works
    1. Processing Pipeline
    Input Image → 2. Center Crop → 3. DPI-Aware Resize → 4. Border Apply → 5. Grid Layout → 6. Plugins (Bleed/Marks) → 7. Final Output.
    2. The Bleed & Crop System
    Bleed: Extends edge pixels outward to prevent white gaps after physical cutting.
    Crop Marks: Drawn per-photo (not global) to ensure precision for manual or machine cutting.
## 🧪 Examples
    1. Standard Passport Photo Sheet
    bash
    imgengine_cli --input photo.jpg --cols 3 --rows 2
    Use code with caution.
    
    2. Studio A4 Layout (Specific Dimensions)
    bash
    imgengine_cli --input photo.jpg --output output.png --cols 6 --rows 2 --gap 15 --width 3.5 --height 3.0
    Use code with caution.
    
    3. Print-Ready PDF with Bleed
    bash
    imgengine_cli --input photo.jpg --output print.pdf --cols 6 --rows 2 --bleed 10 --crop-mark 25
    Use code with caution.

## 🚀 Architecture & Performance
    Context System: Centralized memory pool and layout metadata.
    Plugin System: Decoupled logic for Crop Marks and Bleed processing.
    SIMD Optimized: Leverages AVX2 instructions for high-speed batch processing.
    Validation: Strict enforcement of physical dimensions (DPI/Width/Height > 0).
## 📝 Best Practices
    DPI: Always use 300 DPI for professional printing.
    Cutting Safety: Enable --bleed and --crop-mark for manual trimming.
    Spacing: Keep a --gap of at least 10px to prevent image overlap during cutting.
## 🔮 Roadmap
    Multi-image batch input.
    ISO-compliant layout presets (Visa, ID, etc.).
    GUI Frontend for desktop users.
    CMYK color profile support.
## 🤝 Contribution & License
    Contributions are welcome! Please feel free to submit a Pull Request or open an Issue.
    This project is licensed under the MIT License.


### downlaod image form INTERNET

    curl -L -o input.jpg https://picsum.photos/800

### Your case (A4 passport grid)

    ./imgengine_cli --input ../input.jpg --output final_output.png --cols 6 --rows 2 --gap 15 --width 3.5 --height 3.0 --padding 20 --bleed 10 --crop-mark 25 --crop-offset 8
### ✔ PRO PRINT MODE (recommended) WITH ( PDF )

    ./imgengine_cli \
    --input ../input.jpg \
    --output final.png \      
    --cols 6 \
    --rows 2 \
    --gap 15 \
    --padding 20 \
    --width 3.5 \
    --height 3.0 \
    --bleed 10 \
    --crop-mark 25 \
    --crop-offset 8

    *** PDF ***

    ./imgengine_cli \
    --input ../input.jpg \
    --output final.pdf \      
    --cols 6 \
    --rows 2 \
    --gap 15 \
    --padding 20 \
    --width 3.5 \
    --height 3.0 \
    --bleed 10 \
    --crop-mark 25 \
    --crop-offset 8


rm -rf build
mkdir build && cd build
cmake .. -DBUILD_SHARED_LIBS=ON
make

# Check if the .so was created correctly
ls -lh libimgengine.so


cd build
rm -rf *
cmake .. -DBUILD_SHARED_LIBS=ON
make