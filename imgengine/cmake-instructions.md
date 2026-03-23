### BUILD COMMANDS

    🪟 Windows (MinGW / MSYS / VS Code)
    mkdir build
    cd build

    cmake ..
    cmake --build .

### Linux / WSL

    mkdir build
    cd build

    cmake ..
    make

### RUN YOUR ENGINE After build

    ./imgengine_cli

    or Windows:

    imgengine_cli.exe

### downlaod image form INTERNET

    curl -L -o input.jpg https://picsum.photos/800

### RUN LIKE THIS

    ./imgengine_cli \
  --input ../input.jpg \
  --cols 6 \
  --rows 6 \
  --gap 10 \
  --width 3.5 \
  --height 3.0 \
  --padding 10

 ./imgengine_cli \
  --input ../input.jpg \
  --output finel_output.png \
  --cols 6 \
  --rows  \
  --gap 10 \
  --width 3.5 \
  --height 3.0 \
  --padding 10



  @Rofikali ➜ /workspaces/imgengine/imgengine/build (main) $ ./imgengine_cli   --input input.jpg   --output final_putput.png   --cols 1   --rows 6   --gap 20 --width 4.0 --height 3.0 --border 5
Loading image: input.jpg
✅ Output generated: final_putput.png



rm -rf build
mkdir build && cd build

cmake ..
make

rm -rf build 
rm -rf build && mkdir build && cd build && cmake .. && make