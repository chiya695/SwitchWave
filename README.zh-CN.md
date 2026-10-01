# <img src="https://github.com/user-attachments/assets/b81b9503-948e-4cba-b0a1-f5f809588aad" width="48"> SwitchWave 中文说明

SwitchWave 是一款运行在 Nintendo Switch 自制系统上的硬件加速媒体播放器，基于 mpv 和 FFmpeg。

English：[README.md](README.md)

## 功能概览

- 使用 Switch 硬件加速解码 MPEG1/2/4、VC1、H.264/AVC、H.265/HEVC、VP8、VP9 和 MJPEG 等格式。
- 基于 deko3d 的 mpv 图形后端，支持 4K 60fps 播放、直接渲染和自定义后处理着色器。
- 使用 Switch 原生音频接口，支持最高 5.1 声道布局。
- 支持 HTTP/HTTPS、Samba、NFS 和 SFTP 网络播放。
- 支持通过 `libusbhsfs` 访问外接存储设备。
- 播放过程中可以通过界面或控制台调整常用 mpv 参数。

具体解码能力仍受视频编码参数和 Switch 硬件限制影响。例如，H.264/AVC 的 10 bit 及以上、H.265/HEVC 的 12 bit 及以上，以及 VP9 的 10 bit 及以上不由硬件解码支持。

## 中文支持

本分支重点修复中文文件名、中文元数据、中文网络共享名称、中文字幕和中文输入经常显示为问号或无法编辑的问题。它不是完整的界面翻译，菜单仍然以英文为主。

### 已支持

- 中文文件名、文件夹名、媒体标题、网络共享名称和控制台文本使用 Switch 共享字体渲染。
- 扩大 UI 字体图集的中文字符范围，覆盖常用 CJK 基本区字符、扩展 A、中文标点、注音和全角字符。
- 配置编辑器、网络设置和控制台输入使用 UTF-8 字节边界处理中文字符，中文输入、删除和光标移动不会把一个汉字拆开。
- 启动时将可用的 Switch 共享字体缓存到 `sdmc:/switch/SwitchWave/fonts/`。
- 如果用户没有自己的字体，则创建 `sdmc:/switch/SwitchWave/subfont.ttf` 作为 mpv/libass 字幕回退字体；已有的 `subfont.ttf` 不会被覆盖。
- mpv 编译时启用 GNU libiconv。UTF-8 字幕保持 UTF-8；其他文本字幕默认按 GB18030（包含 GBK）读取。
- Big5 字幕可以在 `mpv.conf` 中加入以下配置：

  ```ini
  sub-codepage=big5
  ```

### 使用限制

- 中文支持取决于主机实际提供的共享字体。字体本身没有对应字形时，仍可能显示为缺字方框或问号。
- 生僻的补充平面汉字、任意 emoji 和上游内容中已经被写成字面 `?` 的字符无法保证恢复。
- ASS 字幕中的精确字体匹配和字幕内嵌字体仍然优先于回退字体。
- SMB 的非 UTF-8 文件名转码仍由 `libsmb2` 负责；NFS 和 SFTP 应由服务器提供 UTF-8 文件名。本分支不会根据乱码外观盲猜网络协议编码。
- 中文支持不代表所有英文菜单都已翻译成中文。
- 当前没有 Nintendo Switch 实机验收结果，因此启动耗时、内存占用、不同系统字体覆盖率和输入法候选词流程仍建议在自己的主机上确认。

## 安装与使用

### 安装已编译版本

1. 准备一台已安装兼容自制系统环境的 Nintendo Switch，并确保 SD 卡可以正常访问。
2. 打开[中文支持分支的构建列表](https://github.com/chiya695/SwitchWave/actions/workflows/build.yml?query=branch%3Achinese-support)，选择已成功的 `Build`，在页面底部 `Artifacts` 中下载 `SwitchWave`。下载的产物压缩包内还有安装 ZIP，需要继续解压。也可以使用项目源码自行编译。Actions 产物需要登录 GitHub，且只保留有限时间；上游原版 Releases 不包含本分支的中文修复。
3. 先备份已有的 `switch/SwitchWave/` 目录和配置，再将安装 ZIP 解压到 SD 卡根目录。合并目录时不要把整个 `switch` 目录删除或覆盖；只复制包内对应文件。安装后的程序路径应为 `switch/SwitchWave/SwitchWave.nro`；保留原有配置时，请合并所需配置项而非直接覆盖 `mpv.conf`。
4. 启动 SwitchWave。首次启动可能会准备字体缓存，随后中文文件名和字幕会使用相应字体。

应用本身会提示 applet 模式可能导致稳定性问题；建议使用完整应用模式启动，尤其是播放高分辨率视频时。

### 配置网络共享

在应用的 `Settings` → `Network` 中添加网络共享，填写服务器地址、端口、共享名、用户名和密码，然后点击 `Connect`。中文共享名称和其他文本字段的编辑会保留 UTF-8 字符；能否连接仍取决于服务器、协议和账户设置，服务器地址应填写实际有效的地址。点击 `Save to file` 保存应用设置。

支持的协议包括：

- Samba（SMB）
- NFS
- SFTP
- HTTP/HTTPS

### 配置 mpv

发行包中的 `mpv.conf` 会作为默认配置。可以在应用内置编辑器中修改，也可以直接编辑 SD 卡上的配置文件。常见位置为：

```text
sdmc:/switch/SwitchWave/mpv.conf
```

字幕编码示例：

```ini
# GBK/GB18030 字幕（默认设置）
sub-codepage=gb18030

# Big5 字幕
# sub-codepage=big5
```

更完整的 mpv 参数说明请参考 [mpv 官方手册](https://mpv.io/manual/master/)。

默认配置仍优先选择英文字幕（`slang=en,eng`），本分支没有自动改动语言偏好。需要优先选择带中文语言标签的字幕轨道时，可以在配置中设置 `slang=zh,zho,chi,en,eng`，也可以在播放菜单中手动选择字幕。该选项决定轨道选择，`sub-codepage` 决定文本解码，二者不是同一功能。

### 字幕仍然显示问号时

1. 确认字幕文件原文没有已经被替换成字面 `?`，并选择正确的字幕轨道。
2. 检查字幕编码：GBK/GB18030 使用默认配置，Big5 使用 `sub-codepage=big5`；不确定时，先用可信的文本编辑器正确识别原编码，再另存为 UTF-8，并保留原文件。
3. 检查 `subfont.ttf`。旧版本生成的字体也会被保留；如果它缺少中文，请退出应用，把原字体备份到别处，再重新启动，让程序在该文件不存在时生成中文回退字体。也可以放入自己有权使用、确实包含所需汉字的 TTF 字体并命名为 `subfont.ttf`。
4. ASS 内嵌字体和指定字体仍可能影响显示；字体缓存目录不代表已实现跨所有字体的自动回退。

本仓库不分发 Nintendo 系统字体，请勿将主机导出的字体上传到公开仓库。

## 编译

### Docker（推荐）

项目提供 Docker 构建脚本，可以自动准备工具链并完成编译：

```sh
./build-docker.sh
```

构建完成后，发行包会生成在 `build/` 目录。默认使用 GIMP 2；如需使用 GIMP 3：

```sh
GIMP_VERSION=3 ./build-docker.sh
```

### 手动编译

手动编译需要 devkitPro Switch 开发环境，以及项目 Makefile 中列出的 Switch 库和构建依赖，包括 `switch-freetype`、`switch-libass`、`switch-harfbuzz`、`switch-curl`、`switch-libssh2`、`switch-mbedtls`、`switch-ntfs-3g`、`switch-lwext4`、`switch-pkg-config`、`dkp-meson-scripts`、`dkp-toolchain-vars` 和 GIMP。

还需要先编译并安装以下依赖：

- `libusbhsfs`
- `libsmb2`（源码位于 `misc/libsmb2/`）
- `libnfs`（源码位于 `misc/libnfs/`）
- `libiconv`（源码位于 `misc/libiconv/`）

主要编译步骤：

```sh
make configure-ffmpeg
make build-ffmpeg -j$(nproc)
make configure-uam
make build-uam
make configure-mpv
make build-mpv
make dist -j$(nproc)
```

发行包会输出到 `build/` 目录。

## 已知问题与反馈

- 本分支主要改善中文显示、中文文本输入和字幕编码，不改变 SwitchWave 原有的播放后端和网络协议实现。
- 如果仍然出现问号，请先确认原始文件名或字幕文件本身是否已经损坏，并检查使用的字体是否包含目标汉字。
- 对于 Big5、其他本地编码或网络服务器返回的非 UTF-8 文件名，请提供可复现的文件样例、协议类型和日志；不要只根据屏幕上的问号判断编码。
- 没有 Switch 实机时，桌面端字体测试只能验证字符范围和 UTF-8 处理，不能证明主机上的每个字形都存在。

欢迎通过 GitHub Issues 提交问题，并附上 Switch 系统版本、SwitchWave 版本、文件来源/协议、字幕编码和复现步骤。

## 项目分析

本分支的中文支持设计、修改范围、验证记录和已知边界见：[docs/chinese-support.md](docs/chinese-support.md)。

## 致谢

- 感谢 [Behemoth](https://github.com/HookedBehemoth) 提供截图按钮覆盖方法。
