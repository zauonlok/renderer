# Software Renderer

This is a shader-based software renderer written from scratch. It is written
in C89 with minimal dependencies, available for Windows, macOS, and Linux.

## Features

* Cross-platform (Windows, macOS, and Linux)
* Minimal dependencies
* Shader-based
* Real-time
* Blinn–Phong reflection model
* Physically based rendering (PBR)
* Metalness workflow
* Specular workflow
* Image-based lighting (IBL)
* Normal mapping
* Shadow mapping
* Cubemapped skybox
* Skeletal animation
* Orbit camera control
* Back-face culling
* Homogeneous clipping
* Perspective-correct interpolation
* Depth testing
* Alpha testing
* Alpha blending
* Platform abstraction layer (window, event, and timer)
* Math library (vector, matrix, and quaternion)
* Mesh loading (obj, gltf)
* Image loading (tga, hdr)

## Download

[Binaries](https://github.com/zauonlok/renderer/releases) for Windows, macOS,
and Linux are available.

## Build

To build the renderer from source, a C89 compiler and development files for
your window system are required.

### Windows

Install [Visual Studio](https://visualstudio.microsoft.com/downloads/) and
run `build_win32.bat`.

### macOS

Install [Xcode](https://itunes.apple.com/us/app/xcode/id497799835?mt=12) and
run `build_macos.sh`.

### Linux

Install GCC and Xlib with the following commands and run `build_linux.sh`.

#### Ubuntu / Debian

```
sudo apt install gcc libx11-dev
```

#### Fedora / RHEL

```
sudo dnf install gcc libX11-devel
```

#### openSUSE / SUSE

```
sudo zypper install gcc libX11-devel
```

## Usage

### Launch

If the renderer is launched without arguments, one of the available scenes
(see below) will be chosen randomly. To display a specific scene, additional
arguments should be supplied. The command line syntax is:

```
Viewer [test_name [scene_name]]
```

### Controls

* Orbit: left click and drag
* Pan: right click and drag
* Zoom: scroll up or down
* Rotate lighting: <kbd>A</kbd> <kbd>D</kbd> <kbd>S</kbd> <kbd>W</kbd>
* Reset everything: <kbd>Space</kbd>

## Samples

| Scene                                                                                                | Command                  |
| ---------------------------------------------------------------------------------------------------- | ------------------------ |
| [<img src="assets/azura/screenshot.png" width="600">](assets/azura/README.md)                        | `Viewer blinn azura`     |
| [<img src="assets/centaur/screenshot.png" width="600">](assets/centaur/README.md)                    | `Viewer blinn centaur`   |
| [<img src="assets/craftsman/screenshot.png" width="600">](assets/craftsman/README.md)                | `Viewer blinn craftsman` |
| [<img src="assets/elfgirl/screenshot.png" width="600">](assets/elfgirl/README.md)                    | `Viewer blinn elfgirl`   |
| [<img src="assets/kgirl/screenshot.gif" width="600">](assets/kgirl/README.md)                        | `Viewer blinn kgirl`     |
| [<img src="assets/mccree/screenshot.png" width="600">](assets/mccree/README.md)                      | `Viewer blinn mccree`    |
| [<img src="assets/nier2b/screenshot.png" width="600">](assets/nier2b/README.md)                      | `Viewer blinn nier2b`    |
| [<img src="assets/phoenix/screenshot.gif" width="600">](assets/phoenix/README.md)                    | `Viewer blinn phoenix`   |
| [<img src="assets/witch/screenshot.png" width="600">](assets/witch/README.md)                        | `Viewer blinn witch`     |
| [<img src="assets/assassin/screenshot.gif" width="600">](assets/assassin/README.md)                  | `Viewer pbr assassin`    |
| [<img src="assets/crab/screenshot.gif" width="600">](assets/crab/README.md)                          | `Viewer pbr crab`        |
| [<img src="assets/dieselpunk/screenshot.png" width="600">](assets/dieselpunk/README.md)              | `Viewer pbr dieselpunk`  |
| [<img src="assets/drone/screenshot.gif" width="600">](assets/drone/README.md)                        | `Viewer pbr drone`       |
| [<img src="assets/helmet/screenshot.png" width="600">](assets/helmet/README.md)                      | `Viewer pbr helmet`      |
| [<img src="assets/junkrat/screenshot.gif" width="600">](assets/junkrat/README.md)                    | `Viewer pbr junkrat`     |
| [<img src="assets/ornitier/screenshot.png" width="600">](assets/ornitier/README.md)                  | `Viewer pbr ornitier`    |
| [<img src="assets/ponycar/screenshot.png" width="600">](assets/ponycar/README.md)                    | `Viewer pbr ponycar`     |
| [<img src="assets/common/footprint/screenshot2.png" width="600">](assets/common/footprint/README.md) | `Viewer pbr sphere`      |

## License

[MIT](LICENSE)
