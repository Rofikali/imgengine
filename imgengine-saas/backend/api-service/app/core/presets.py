from typing import Literal


PresetName = Literal["passport-45x35", "passport-38x35", "printready-6x6"]

PRESETS = (
    {
        "name": "passport-45x35",
        "label": "Passport 45 × 35 mm",
        "description": "Six-by-six grid with print-safe border, bleed, and crop marks.",
    },
    {
        "name": "passport-38x35",
        "label": "Passport 38 × 35 mm",
        "description": "Six-by-six grid for the 38 × 35 mm passport format.",
    },
    {
        "name": "printready-6x6",
        "label": "Print-ready 6 × 6",
        "description": "Six-by-six print sheet with border, bleed, and crop marks.",
    },
)
