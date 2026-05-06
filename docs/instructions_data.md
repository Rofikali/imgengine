### install packeges

    sudo apt update -y && sudo apt upgrade -y
    sudo apt install -y libturbojpeg0-dev libnuma-dev 
    sudo apt install -y liburing-dev 
    sudo apt update -y; sudo apt upgrade -y; sudo apt install -y libturbojpeg0-dev libnuma-dev liburing-dev; 
    sudo apt-get update && sudo apt-get install -y clang-format;

    
    sudo apt install -y build-essential
    sudo apt install graphviz -y
    sudo apt install libclang-dev clang
    pip install networkx
    pip3 install pyvis
    pip install clang==18.1.8

    sudo apt install -y graphviz     ( for graph generation )
    
    `` from the official nothings/stb repository using curl
         curl -o stb_image.h https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

### That’s the difference between

    engineer ❌
    performance engineer ✅
    systems architect 🚀

   cd imgengine && [ -d "build" ] || mkdir build && cd build

   rm -rf *
   cmake .. && make -j

### Formatting check
    cmake --build . --target format        # runs clang-format -i (requires clang-format)
    cmake --build . --target format-check  # runs the Python verifier (requires python3 + clang-format)

### How to Run ./imgengine_cli --help

   ./imgengine_cli -i ../test.jpg -o output.jpg

# Passport — fits perfectly

./imgengine_cli --input ../photo.jpg --output passport.jpg \
    --cols 6 --rows 3 --gap 20 --padding 20

# Studio — auto-scales 6 photos to fit with gaps

./imgengine_cli --input ../photo.jpg --output studio.jpg \
    --cols 6 --rows 2 --gap 15 --padding 40

# Print-ready with bleed (use fewer cols to leave room)

 ./imgengine_cli --input ../photo.jpg --output printready.jpg   \
   --cols 6 --rows 6 --gap 15 --padding 20    \
   --bleed 1 --crop-mark 5 --crop-offset 5 --crop-thickness 10 --border 2

### How to test 4k_Image

   build/bench_lat ../tests/samples/4k_test.jpg

### find out the keyboard

    grep -r "struct img_ctx" .  img_pipeline_desc_t;

### Reset to clean commit

   git reset --hard f5e48956708f6674122fee0aeaa3f5b5a6ff6dad
