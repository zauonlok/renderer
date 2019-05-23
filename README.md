# Software Renderer

This is a shader-based software renderer written from scratch.

## Features

* Cross-platform (Windows, macOS, and Linux)
* Shader-based
* Blinn–Phong shading
* Physically based rendering
* Image-based lighting
* Normal mapping
* Shadow mapping
* Depth testing
* Blending
* Face culling
* Skybox
* Texture (2D, Cubemap)
* Animation

## Build

### Windows

Install [Visual Studio](https://visualstudio.microsoft.com/downloads/) and run `build_win32.bat`.

### macOS

Install [Xcode](https://itunes.apple.com/us/app/xcode/id497799835?mt=12) and run `build_macos.sh`.

### Linux

Install GCC and Xlib with the following commands and run `build_linux.sh`.

#### Ubuntu

```
sudo apt install gcc libx11-dev
```

#### Fedora

```
sudo dnf install gcc libX11-devel
```

## Usage

* Orbit: left click and drag
* Pan: right click and drag
* Zoom: scroll up/down
* Rotate lighting: <kbd>A</kbd> <kbd>D</kbd> <kbd>S</kbd> <kbd>W</kbd>
* Reset everything: <kbd>Space</kbd>

## Samples

![assassin](assets/assassin/screenshot.gif)

![azura](assets/azura/screenshot.png)

![centaur](assets/centaur/screenshot.png)

![crab](assets/crab/screenshot.gif)

![craftsman](assets/craftsman/screenshot.png)

![dieselpunk](assets/dieselpunk/screenshot.png)

![drone](assets/drone/screenshot.gif)

![elfgirl](assets/elfgirl/screenshot.png)

![helmet](assets/helmet/screenshot.png)

![junkrat](assets/junkrat/screenshot.gif)

![kgirl](assets/kgirl/screenshot.gif)

![mccree](assets/mccree/screenshot.png)

![nier2b](assets/nier2b/screenshot.png)

![ornitier](assets/ornitier/screenshot.png)

![phoenix](assets/phoenix/screenshot.gif)

![ponycar](assets/ponycar/screenshot.png)

![witch](assets/witch/screenshot.png)

## License

[MIT](LICENSE)
