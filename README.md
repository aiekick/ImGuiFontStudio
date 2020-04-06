# ImGuiFontStudio

ImGuiFontStudio is a tool for Subset font and extract glyph names for use embbeded or not in a software, especially for use with ImGui for embedded way.

Greatly inspired / based on the project [IconFontCppHeaders](https://github.com/juliettef/IconFontCppHeaders)

This is my first big opensource software.

As a self learning man i always developped my softs in solo.

So maybe you will found some weird things, bad design pattern, 
bad optimization, or others bullshits :)

Please send me your feedback.
I enjoy all constructive comments and help is welcome.

## The features :

* can open ttf or otf font file
* can subset font file (one file same time or by batch)
* can generate header with corresponding glyph names and codepoint
* can generate cpp file with compressed data o=foir embedded use
* can merge many font file in one (WIP, there is some pending issues about this last)
* can edit codepoint and glyph names
* have a project file
* many tool available for select glyphs (by zone, by line, by codepoint range)
* Cross Platform, tested on Win/Osx/Linux 
* Change / define ImGui app Theme
* No lib, one executable only

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

ImGuiFontStudio if an open soruce software under [license apache 2.0](LICENE)
 
## Library used :

* [Glfw - ZLIB](http://www.glfw.org/)
* [Dear ImGui - Docking branch - MIT](ghttps://github.com/ocornut/imgui)
* [Glad - MIT](https://github.com/Dav1dde/glad)
* [Stb - MIT](https://github.com/nothings/stb)
* [tinyxml2 - ZLIB]( https://github.com/leethomason/tinyxml2)
* [dirent - MIT]( https://github.com/tronkko/dirent)
* [sfntly - Apache 2.0](https://github.com/rillig/sfntly)
* [cTools - MIT](https://github.com/aiekick/cTools)
* [ImGuiFileDialog - MIT](https://github.com/aiekick/ImGuiFileDialog)

