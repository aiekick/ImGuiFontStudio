![Win](https://github.com/aiekick/ImGuiFontStudio/workflows/Win/badge.svg) ![Linux](https://github.com/aiekick/ImGuiFontStudio/workflows/Linux/badge.svg) ![Osx](https://github.com/aiekick/ImGuiFontStudio/workflows/Osx/badge.svg)

Last Github Actions Builds : [Win](https://github.com/aiekick/ImGuiFontStudio/actions?query=workflow%3AWin) / [Linux](https://github.com/aiekick/ImGuiFontStudio/actions?query=workflow%3ALinux) / [Osx](https://github.com/aiekick/ImGuiFontStudio/actions?query=workflow%3AOsx)

# ImGuiFontStudio

ImGuiFontStudio is a tool for Subset font and extract glyph names for use embbeded or not in a software, especially for use with ImGui for embedded way.

Greatly inspired / based on the project [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders)

This is my first big opensource software.

As a self learning man i always developped my softs in solo.

So maybe you will found some weird things, bad design pattern, 
bad optimization, or others bullshits :)

Please send me your feedback.
I enjoy all constructive comments and help is welcome.

ImGuiFontStudio url : https://github.com/aiekick/ImGuiFontStudio

Succesfully tested on my side :

* on Win 7 x64 (in exe version x86/x64)
* On Linux Debian/Ubuntu (in exe version x86)
* MacOs Mojave (in exe version x86)
 
## The features :

* can open ttf or otf font file
* can subset font file (one file same time or by batch)
* can generate header with corresponding glyph names and codepoint
* can generate cpp file with compressed data or for embedded use
* can merge many font file in one (the glyphs will be resized)
* can edit codepoint and glyph names
* have a project file
* many tool available for select glyphs (by zone, by line, by codepoint range)
* Cross Platform, tested on Win/Osx/Linux 
* Change / define ImGui app Theme
* No lib, one executable only

## How to Build :

You need to use cMake.
For the 3 Os (Win, Linux, MacOs), the cMake usage is exactly the same, 


1) Choose a build directory. (called here my_build_directory for instance)
2) Choose a Build Mode : "Release" / "MinSizeRel" / "RelWithDebInfo" / "Debug" (called here BuildMode for instance)
3) Run cMake in console : (the first for generate cmake build files, the second for build the binary)
```cpp
cmake my_build_directory -DCMAKE_BUILD_TYPE=BuildMode
cmake --build my_build_directory --config BuildMode
```

Some cMake version need Build mode define via the directive CMAKE_BUILD_TYPE or via --Config when we launch the build. 
This is why i put the boths possibilities

By the way you need before, to make sure, you have needed dependencies.

### On Windows :

You need to have the opengl library installed

### On Linux :

You need many lib : (X11, xrandr, xinerama, xcursor, mesa)

If you are on debian you can run :  

```cpp
sudo apt-get update 
sudo apt-get install libgl1-mesa-dev libx11-dev libxi-dev libxrandr-dev libxinerama-dev libxcursor-dev
```

### On MacOs :

you need many lib : opengl and cocoa framework

## Screenshots (with the default theme)

Main View : Source pane
![Source pane](doc/src.png)

Main View : Final pane with two fonts
![Final pane with two fonts](doc/dst_two_font_merge.png)

Main View : Final pane for edition
![Final pane for edition](doc/dst_edit.png)

## Contributions / Issues / Features request

You can use the issue tab for report issues or for features request.

## License :

ImGuiFontStudio is an open source software under [license apache 2.0](LICENSE)
 
## Library used :

* [Glfw - ZLIB](http://www.glfw.org/)
* [Dear ImGui - Docking branch - MIT](https://github.com/ocornut/imgui)
* [Glad - MIT](https://github.com/Dav1dde/glad)
* [Stb - MIT](https://github.com/nothings/stb)
* [tinyxml2 - ZLIB]( https://github.com/leethomason/tinyxml2)
* [dirent - MIT]( https://github.com/tronkko/dirent)
* [sfntly - Apache 2.0](https://github.com/rillig/sfntly)
* [cTools - MIT](https://github.com/aiekick/cTools)
* [ImGuiFileDialog - MIT](https://github.com/aiekick/ImGuiFileDialog)
