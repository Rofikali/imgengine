# 📸 IMGENGINE CLI

### Professional Photo Layout & Print Pipeline

![Build](https://img.shields.io/badge/build-passing-brightgreen)
![Platform](https://img.shields.io/badge/platform-linux--windows-blue)
![License](https://img.shields.io/badge/license-MIT-lightgrey)

`imgengine_cli` is a high-performance command-line tool for generating **print-ready photo sheets** such as passport photos, ID cards, and studio layouts.

Designed for **print shops, studios, and automation pipelines**, it delivers precise, DPI-aware outputs with professional-grade features like bleed and crop marks.

---

## 🔷 Overview

`imgengine` is built with a **systems-level architecture in C**, focused on:

- ⚡ Speed (SIMD / AVX2 optimized)
- 🧠 Memory efficiency (custom memory pool)
- 🖨️ Print accuracy (DPI + real-world dimensions)
- 🔧 Extensibility (plugin-based pipeline)

---

## ⚡ Features

- ✅ Grid-based layout engine (rows × columns)
- ✅ DPI-aware resizing (real-world print accuracy)
- ✅ Automatic scaling (fit-to-page)
- ✅ Per-photo crop marks (lab standard)
- ✅ Bleed support (cut-safe output)
- ✅ PNG + PDF export
- ✅ CLI-first (automation ready)
- ✅ SIMD accelerated (AVX2)

---

## 📦 Installation

### Requirements

- CMake ≥ 3.16
- GCC / Clang / MSVC
- Linux / Windows (WSL supported)

---

### Build

    --- bash ---
    git clone https://github.com/Rofikali/imgengine
    cd imgengine

    mkdir build && cd build
    cmake ..
    make

## Run

    ./imgengine_cli --help

## 🧑‍💻 Usage

    Basic Syntax
    imgengine_cli --input <file> [--output <file>] [OPTIONS]

## 📥 Input / Output

    Type Supported Formats
    Input JPG, PNG
    Output PNG, PDF

## ⚙️ Configuration Options

    🔹 Required
    Option Description
    --input Input image file
    🔹 Output
    Option Description Default
    --output Output file (png/pdf) output.png
    --quiet Disable logs off

## Layout

    Option Description Default
    --cols Number of columns 2
    --rows Number of rows 3
    --gap Space between photos (px) 15
    --padding Page margin (px) 20

## 🔹 Photo Settings

    Option Description Default
    --width Photo width (cm) 4.5
    --height Photo height (cm) 3.5
    --dpi Print DPI 300
    --border Border thickness (px) 2

## 🔹 Professional Print

    Option Description Default
    --bleed Extra pixels for cutting safety 10
    --crop-mark Crop mark length 20
    --crop-thickness Crop mark thickness 2
    --crop-offset Distance from image edge 8

## 🧠 Processing Pipeline

    Input Image
       ↓
    Center Crop
       ↓
    DPI Resize
       ↓
    Border Apply
       ↓
    Grid Layout Engine
       ↓
    Plugin System (Bleed + Crop Marks)
       ↓
    Final Output (PNG / PDF)

## 🧪 Examples

    1. Passport Sheet
    imgengine_cli --input photo.jpg --cols 3 --rows 2
    ./imgengine_cli --input ../input.jpg --output final.png --cols 4 --rows 2 --gap 120 --padding 80 --bleed 20 --crop-offset 10 --crop-mark 40 --width 4.5 --height 3.5 --dpi 300 --border 2 --crop-thickness 22

    2. PDF EXPORT
    ./imgengine_cli --input ../input.jpg --output final.pdf --cols 4 --rows 2 --gap 120 --padding 80 --bleed 20 --crop-offset 10 --crop-mark 40 --width 4.5 --height 3.5 --dpi 300 --border 2 --crop-thickness 22

## ⚡ Performance

    Memory pool allocation (no malloc overhead)
    SIMD optimized blitting and resizing
    Designed for batch processing workflows

## 📝 Best Practices

    Use 300 DPI for professional printing
    Always enable:
    --bleed
    --crop-mark
    Keep gap ≥ 10px for safe cutting

## 🔮 Roadmap

     Batch multi-image input
     ISO passport presets
     GUI (Electron / Web UI)
     CMYK color support
     Cloud API (FastAPI integration)

## 🤝 Contributing

    Pull requests are welcome.

## 📄 License

    MIT License
