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

| Screenshot                                                                                           | Command                    |
| ---------------------------------------------------------------------------------------------------- | -------------------------- |
| [<img src="assets/azura/screenshot.png" width="360">](assets/assassin/README.md)                     | `./Viewer blinn azura`     |
| [<img src="assets/centaur/screenshot.png" width="360">](assets/centaur/README.md)                    | `./Viewer blinn centaur`   |
| [<img src="assets/craftsman/screenshot.png" width="360">](assets/craftsman/README.md)                | `./Viewer blinn craftsman` |
| [<img src="assets/elfgirl/screenshot.png" width="360">](assets/elfgirl/README.md)                    | `./Viewer blinn elfgirl`   |
| [<img src="assets/kgirl/screenshot.gif" width="360">](assets/kgirl/README.md)                        | `./Viewer blinn kgirl`     |
| [<img src="assets/mccree/screenshot.png" width="360">](assets/mccree/README.md)                      | `./Viewer blinn mccree`    |
| [<img src="assets/nier2b/screenshot.png" width="360">](assets/nier2b/README.md)                      | `./Viewer blinn nier2b`    |
| [<img src="assets/phoenix/screenshot.gif" width="360">](assets/phoenix/README.md)                    | `./Viewer blinn phoenix`   |
| [<img src="assets/witch/screenshot.png" width="360">](assets/witch/README.md)                        | `./Viewer blinn witch`     |
| [<img src="assets/assassin/screenshot.gif" width="360">](assets/assassin/README.md)                  | `./Viewer pbr assassin`    |
| [<img src="assets/crab/screenshot.gif" width="360">](assets/crab/README.md)                          | `./Viewer pbr crab`        |
| [<img src="assets/dieselpunk/screenshot.png" width="360">](dieselpunk/assassin/README.md)            | `./Viewer pbr dieselpunk`  |
| [<img src="assets/drone/screenshot.gif" width="360">](assets/drone/README.md)                        | `./Viewer pbr drone`       |
| [<img src="assets/helmet/screenshot.png" width="360">](assets/helmet/README.md)                      | `./Viewer pbr helmet`      |
| [<img src="assets/junkrat/screenshot.gif" width="360">](assets/junkrat/README.md)                    | `./Viewer pbr junkrat`     |
| [<img src="assets/ornitier/screenshot.png" width="360">](assets/ornitier/README.md)                  | `./Viewer pbr ornitier`    |
| [<img src="assets/ponycar/screenshot.png" width="360">](assets/ponycar/README.md)                    | `./Viewer pbr ponycar`     |
| [<img src="assets/common/footprint/screenshot2.png" width="360">](assets/common/footprint/README.md) | `./Viewer pbr sphere`      |

## License

[MIT](LICENSE)
