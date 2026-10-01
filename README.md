# <img src="https://github.com/user-attachments/assets/b81b9503-948e-4cba-b0a1-f5f809588aad" width="48"> SwitchWave
A hardware-accelerated media player for the Nintendo Switch, built on mpv and FFmpeg.

## Features
- Custom hardware acceleration backend for FFmpeg, with dynamic frequency scaling. The following codecs can be decoded:
    - MPEG1/2/4
    - VC1
    - H.264/AVC (10+ bit not supported by hardware)
    - H.265/HEVC (12+ bit not supported by hardware)
    - VP8
    - VP9 (10+ bit not supported by hardware)
- Custom graphics backend for mpv using [deko3d](https://github.com/devkitPro/deko3d), supporting:
    - Playback at 4k60fps
    - Direct rendering (faster software decoding)
    - Custom post-processing shaders
- Custom audio backend for mpv using native Nintendo APIs, supporting layouts up to 5.1 surround
- Network playback through HTTP/S, Samba, NFS or SFTP
- External drive support using [libusbhsfs](https://github.com/DarkMatterCore/libusbhsfs)
- Rich and responsive user interface, even under load

## Screenshots

<p float="center">
    <img src="https://github.com/user-attachments/assets/09aed446-148a-4276-8b07-336890c224a3" width="413" />
    <img src="https://github.com/user-attachments/assets/6e354511-47bc-4898-881c-348d5a9e6fbc" width="413" />
    <img src="https://github.com/user-attachments/assets/b86eb7c6-4229-46c6-8709-86d1a6ee8eed" width="413" />
    <img src="https://github.com/user-attachments/assets/70f4be3e-fa1e-434a-b76c-4fb6b671f80e" width="413" />
</p>

## Setup
- Download the [latest release](https://github.com/averne/SwitchWave/releases/latest), and extract it to the root of your sd card (be careful to merge and not overwrite folders)
- Network shares can be configured through the app, as can mpv settings via the built-in editor (refer to the [manual](https://mpv.io/manual/master/))
- Most relevant runtime parameters can be dynamically adjusted during playback through the menu, or failing that, the console ([manual](https://mpv.io/manual/master/#console))

## Chinese text support (this fork)
- Simplified and Traditional Chinese filenames, metadata, network share names and console text use the Switch shared fonts, regardless of the system language.
- CJK characters in the Unicode BMP are requested from the font atlas, including Extension A, punctuation and Bopomofo. Actual coverage depends on the fonts installed on the console; supplementary-plane ideographs and arbitrary emoji are not guaranteed.
- The configuration editor, network settings and console convert keyboard UTF-16 cursor positions to UTF-8 byte offsets. Chinese insertion, deletion and cursor movement keep complete character boundaries.
- At startup, missing shared fonts are cached locally in `sdmc:/switch/SwitchWave/fonts/`. A Chinese fallback `subfont.ttf` is created only if one does not already exist; user-provided fallback fonts are preserved. No Nintendo font files are distributed by this repository.
- The locally cached `subfont.ttf` is used as mpv/libass's fallback font. Exact ASS font matches and embedded fonts still take precedence.
- mpv is built with libiconv. Valid UTF-8 subtitles remain UTF-8; other text subtitles default to GB18030 (including GBK). For Big5 subtitles, put `sub-codepage=big5` in `mpv.conf`. Encoding cannot recover characters already replaced with literal `?` upstream.
- This is text-rendering/input support, not a translation of the English menus. Existing audio/subtitle language preferences are not changed.

See [the Chinese analysis and device checklist](docs/chinese-support.md) for architecture, root causes and limitations.

## Building

### Docker (recommended)
```sh
./build-docker.sh
```
This builds the toolchain image and compiles everything automatically. Output will be found in `build/`.

To build with GIMP 3 instead of the default GIMP 2:
```sh
GIMP_VERSION=3 ./build-docker.sh
```

### Manual
- Set up a [devkitpro](https://devkitpro.org/wiki/devkitPro_pacman) environment for Switch homebrew development.
- Install the following packages: `switch-bzip2`, `switch-dav1d`, `switch-freetype`, `switch-glm`, `switch-harfbuzz`, `switch-libarchive`, `switch-libass`, `switch-libfribidi`, `switch-libjpeg-turbo`, `switch-libpng`, `switch-libwebp`, `switch-curl`, `switch-libssh2`, `switch-mbedtls`, `switch-ntfs-3g` and `switch-lwext4`. In addition, the following build dependencies are required: `switch-pkg-config`, `dkp-meson-scripts`, `dkp-toolchain-vars`, and [GIMP](https://www.gimp.org/) (2 or 3).
- Compile and install a GPL build of [libusbhsfs](https://github.com/DarkMatterCore/libusbhsfs).
- Compile and install [libsmb2](misc/libsmb2/) and [libnfs](misc/libnfs/).
- Compile and install [libiconv](misc/libiconv/) before configuring mpv. The supplied `PKGBUILD` targets the devkitPro environment; Docker performs this step automatically.
- Configure, compile and install FFmpeg: `make configure-ffmpeg && make build-ffmpeg -j$(nproc)`.
- Configure, compile and install libuam: `make configure-uam && make build-uam`.
- Configure, compile and install mpv: `make configure-mpv && make build-mpv`.
- Finally, compile the main application and build the release distribution: `make dist -j$(nproc)`.
- Output will be found in `build/`.

## Credits
- [Behemoth](https://github.com/HookedBehemoth) for the screenshot button overriding method.
