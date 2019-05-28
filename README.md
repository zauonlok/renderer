# Software Renderer

This is a shader-based software renderer written from scratch. It is written in C89 with minimal dependencies, available for Windows, macOS, and Linux.

## Features

* Cross-platform (Windows, macOS, and Linux)
* Shader-based
* Real-time
* Blinn–Phong shading
* Physically based rendering (PBR)
* Metalness workflow
* Specular workflow
* Image-based lighting (IBL)
* Normal mapping
* Shadow mapping
* Depth testing
* Alpha testing
* Blending
* Clipping
* Back-face culling
* Perspective-correct interpolation
* Skybox
* Texturing (2D, Cubemap)
* Skeletal animation
* Orbit camera control
* Mesh loading (obj, glTF)
* Image loading (tga, hdr)

## Download

[Binaries](https://github.com/zauonlok/renderer/releases) for Windows, macOS, and Linux are available.

## Build

To build the renderer from source, a C89 compiler and development files for your window system are required.

### Windows

Install [Visual Studio](https://visualstudio.microsoft.com/downloads/) and run `build_win32.bat`.

### macOS

Install [Xcode](https://itunes.apple.com/us/app/xcode/id497799835?mt=12) and run `build_macos.sh`.

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

If the renderer is launched without arguments, one of the available scenes (see below) will be chosen randomly. To display a specific scene, additional arguments should be supplied. The command line syntax is:

```
./Viewer [test_name [scene_name]]
```

### Controls

* Orbit: left click and drag
* Pan: right click and drag
* Zoom: scroll up or down
* Rotate lighting: <kbd>A</kbd> <kbd>D</kbd> <kbd>S</kbd> <kbd>W</kbd>
* Reset everything: <kbd>Space</kbd>

## Samples

| Scene                                        | Screenshot                                                      | Command                    |
| -------------------------------------------- | --------------------------------------------------------------- | -------------------------- |
| [Azura](assets/azura/README.md)              | <img src="assets/azura/screenshot.png" width="480">             | `./Viewer blinn azura`     |
| [Centaur](assets/centaur/README.md)          | <img src="assets/centaur/screenshot.png" width="480">           | `./Viewer blinn centaur`   |
| [Craftsman](assets/craftsman/README.md)      | <img src="assets/craftsman/screenshot.png" width="480">         | `./Viewer blinn craftsman` |
| [Elf Girl](assets/elfgirl/README.md)         | <img src="assets/elfgirl/screenshot.png" width="480">           | `./Viewer blinn elfgirl`   |
| [Kgirl](assets/kgirl/README.md)              | <img src="assets/kgirl/screenshot.gif" width="480">             | `./Viewer blinn kgirl`     |
| [McCree](assets/mccree/README.md)            | <img src="assets/mccree/screenshot.png" width="480">            | `./Viewer blinn mccree`    |
| [NieR 2B](assets/nier2b/README.md)           | <img src="assets/nier2b/screenshot.png" width="480">            | `./Viewer blinn nier2b`    |
| [Phoenix](assets/phoenix/README.md)          | <img src="assets/phoenix/screenshot.gif" width="480">           | `./Viewer blinn phoenix`   |
| [Witch](assets/witch/README.md)              | <img src="assets/witch/screenshot.png" width="480">             | `./Viewer blinn witch`     |
| [Assassin](assets/assassin/README.md)        | <img src="assets/assassin/screenshot.gif" width="480">          | `./Viewer pbr assassin`    |
| [Crab](assets/crab/README.md)                | <img src="assets/crab/screenshot.gif" width="480">              | `./Viewer pbr crab`        |
| [Dieselpunk](assets/dieselpunk/README.md)    | <img src="assets/dieselpunk/screenshot.png" width="480">        | `./Viewer pbr dieselpunk`  |
| [Drone](assets/drone/README.md)              | <img src="assets/drone/screenshot.gif" width="480">             | `./Viewer pbr drone`       |
| [Helmet](assets/helmet/README.md)            | <img src="assets/helmet/screenshot.png" width="480">            | `./Viewer pbr helmet`      |
| [Junkrat](assets/junkrat/README.md)          | <img src="assets/junkrat/screenshot.gif" width="480">           | `./Viewer pbr junkrat`     |
| [Ornitier](assets/ornitier/README.md)        | <img src="assets/ornitier/screenshot.png" width="480">          | `./Viewer pbr ornitier`    |
| [Pony Car](assets/ponycar/README.md)         | <img src="assets/ponycar/screenshot.png" width="480">           | `./Viewer pbr ponycar`     |
| [Spheres](assets/common/footprint/README.md) | <img src="assets/common/footprint/screenshot2.png" width="480"> | `./Viewer pbr sphere`      |

## License

[MIT](LICENSE)
