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

支持切换简体中文界面，并改善中文文件名、媒体标题、网络共享名称、字幕的显示和中文输入。

### 切换中文界面

打开 `Settings` → `Language`，选择 `简体中文`。文件管理、设置、播放菜单、画质/音频/字幕选项、统计标签、帮助和删除提示会切换为中文。语言立即生效，并自动保存到 `sdmc:/switch/SwitchWave/SwitchWave.conf`，重启后保留；选择 `English` 可切回英文。首次安装默认使用英文。

也可以在应用配置 `SwitchWave.conf` 中设置 `language=zh-CN` 或 `language=en`，不要写入 `mpv.conf`。mpv 参数名、配置语法、媒体标题、编码/协议名称和第三方日志保持原样。

### 已支持

- 中文文件名、文件夹名、媒体标题、网络共享名称和控制台文本使用主机提供的中文字体显示。
- 配置编辑器、网络设置和控制台支持中文输入、删除和光标移动。
- 首次启动会准备字体缓存。如果没有自定义字幕字体，程序会创建 `sdmc:/switch/SwitchWave/subfont.ttf`；已有文件不会被覆盖。
- 支持 UTF-8 和 GBK/GB18030 文本字幕；非 UTF-8 字幕默认按 GB18030 读取。
- Big5 字幕可以在 `mpv.conf` 中加入以下配置：

  ```ini
  sub-codepage=big5
  ```

### 使用限制

- 中文支持取决于主机实际提供的共享字体。字体本身没有对应字形时，仍可能显示为缺字方框或问号。
- 部分生僻汉字和 emoji 可能无法显示；原始文件中已经被替换为 `?` 的文字无法恢复。
- ASS 字幕中的精确字体匹配和字幕内嵌字体仍然优先于回退字体。
- NFS 和 SFTP 服务器应使用 UTF-8 文件名；服务器返回的文件名编码不正确时，仍可能出现乱码。
- 中文界面不改变原有音轨和字幕轨道的语言选择偏好，也不翻译第三方日志或媒体内容。

## 安装与使用

### 安装已编译版本

1. 准备一台已安装兼容自制系统环境的 Nintendo Switch，并确保 SD 卡可以正常访问。
2. 从[发布页](https://github.com/chiya695/SwitchWave/releases)下载 `SwitchWave-*.zip` 安装包。`Source code` 是源代码，不是安装包。
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

默认配置优先选择英文字幕（`slang=en,eng`）。需要优先选择带中文语言标签的字幕轨道时，可以在配置中设置 `slang=zh,zho,chi,en,eng`，也可以在播放菜单中手动选择字幕。该选项决定轨道选择，`sub-codepage` 决定文本解码，二者不是同一功能。

### 字幕仍然显示问号时

1. 确认字幕文件原文没有已经被替换成字面 `?`，并选择正确的字幕轨道。
2. 检查字幕编码：GBK/GB18030 使用默认配置，Big5 使用 `sub-codepage=big5`；不确定时，先用可信的文本编辑器正确识别原编码，再另存为 UTF-8，并保留原文件。
3. 检查 `subfont.ttf`。旧版本生成的字体也会被保留；如果它缺少中文，请退出应用，把原字体备份到别处，再重新启动，让程序在该文件不存在时生成中文回退字体。也可以放入自己有权使用、确实包含所需汉字的 TTF 字体并命名为 `subfont.ttf`。
4. ASS 内嵌字体和指定字体仍可能影响显示；字体缓存目录不代表已实现跨所有字体的自动回退。

本仓库不分发 Nintendo 系统字体，请勿将主机导出的字体上传到公开仓库。

## 编译

首次获取源码应使用 `git clone --recursive --branch chinese-support https://github.com/chiya695/SwitchWave.git`。已克隆的仓库先执行 `git submodule update --init --recursive`，以获取构建所需的固定版本子模块。

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

- 如果仍然出现问号，请先确认原始文件名或字幕文件本身是否已经损坏，并检查使用的字体是否包含目标汉字。
- 如果反馈字幕乱码或网络文件名显示问题，请提供字幕编码、协议类型及可复现的样例。

欢迎通过 GitHub Issues 提交问题，并附上 Switch 系统版本、SwitchWave 版本、文件来源/协议、字幕编码和复现步骤。

## 文件管理：删除文件

1. 在 `Explorer`（中文界面为「文件管理」）中切换到 SD 卡、USB 或已连接的 SMB/NFS/SFTP 共享。
2. 用手柄导航焦点选中一个普通文件，按 **X**，或点击「删除文件」按钮。
3. 在确认框中核对完整路径。默认焦点为「取消」；选择「永久删除」后才执行删除。
4. 删除成功后目录自动刷新；失败会显示错误。SMB 账号必须拥有服务器授予的删除权限。

**删除不会经过回收站，不能撤销。** 请备份重要文件；首次使用远程删除时，先用不需要保留的文件试用。

- 只删除单个普通文件，不递归删除文件夹，不删除挂载根目录或链接。
- HTTP/HTTPS 和「最近播放」列表不提供删除；内部 `user:` 存储不开放删除功能。
- 播放中的文件会被保护；播放菜单中的字幕/着色器文件选择器不提供删除按钮。
- 只读 USB、无删除权限的共享或连接已断开的服务器无法删除文件。
- 尚未在 Switch 实机和真实 SMB/NFS/SFTP 服务器上测试，远程删除的兼容性仍需确认。

## 隐私：清空最近播放

在「文件管理」的存储来源中选择 `recent`（最近播放），点击「清空最近播放」，核对说明后选择「清空列表」。默认确认焦点为「取消」。

该操作会立即清空列表及已保存的 `sdmc:/switch/SwitchWave/history.txt`，不删除媒体文件、网络共享文件或播放进度。设置页的「清空历史记录」按钮也可清空列表。保存失败时会显示错误。

之后播放新文件仍会产生新记录；如不希望保留记录，可将「设置」→「历史记录」→「最大条目数」设为 `0`，然后点击「保存到文件」。

此按钮只清空最近播放记录，不清理播放进度、截图、服务器日志或备份，也不提供存储介质上的安全擦除。播放进度可在设置页使用「清除播放进度」单独清理。

## 开发文档

字体、文本输入和文件管理的实现说明见：[docs/chinese-support.md](docs/chinese-support.md)。

## 致谢

- 感谢 [Behemoth](https://github.com/HookedBehemoth) 提供截图按钮覆盖方法。
