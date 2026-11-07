# LangSU 改造说明

本工程由 KernelSU 3.3.1 改造而来，所有品牌、包名、daemon 名称均已改为 LangSU。

- 原始工程备份：`../_backup_KernelSU-3.3.1/`
- 工程目录：`KernelSU-3.3.1/`（目录名保持原样，方便与你已有的构建脚本对接；如需改名可整体重命名）

## 1. 命名对照

| 项目 | 原 KernelSU | LangSU |
| --- | --- | --- |
| Manager 包名 | `me.weishu.kernelsu` | `com.langsu.manager` |
| Manager 代码命名空间 | `me.weishu.kernelsu` | `com.langsu.manager` |
| 内核模块 | `kernelsu.ko` | `langsu.ko` |
| 内核 Kconfig | `CONFIG_KSU` | `CONFIG_LANGSU` |
| 用户态守护进程 | `ksud` | `langsud` |
| 早期 init | `ksuinit` | `langsuinit` |
| 数据目录 | `/data/adb/ksu` | `/data/adb/langsu` |
| daemon 路径 | `/data/adb/ksud` | `/data/adb/langsud` |
| SELinux domain | `u:r:ksu:s0` | `u:r:langsu:s0` |
| 深链接 scheme | `ksu://` | `langsu://` |
| 驱动 anon inode 名 | `[ksu_driver]` | `[langsu_driver]` |
| 内核 JNI 库 | `libkernelsu.so` | `liblangsu.so` |

## 2. 隐蔽性改造

### 2.1 桌面包名与显示名可定制

`manager/app/build.gradle.kts` 支持以下 Gradle 参数：

```bash
# 自定义包名（内核必须用同一个包名编译，否则 Manager 不被识别）
./gradlew assembleRelease -PLANGSU_PACKAGE_NAME=com.example.weather

# 自定义桌面显示名
./gradlew assembleRelease -PLANGSU_NAME="天气"

# 生成随机包名（每次都不同）
./gradlew assembleRelease -PLANGSU_RANDOM_PACKAGE=true

# 生成"看起来随机但可复现"的包名
./gradlew assembleRelease -PLANGSU_RANDOM_PACKAGE=true -PLANGSU_PACKAGE_SEED="my-device-2026"
```

内核侧对应：

```bash
make CONFIG_LANGSU=m LANGSU_PACKAGE_NAME=com.example.weather
# 随机包名模式下，还需要同时传入签名指纹，见 2.4
```

> 注意：自定义包名后，请把 `kernel/Kbuild` 的 `LANGSU_EXPECTED_SIZE` / `LANGSU_EXPECTED_HASH`
> 换成你自己的签名证书指纹（见 2.4），否则内核不会把 Manager 认作管理器。

### 2.2 隐藏桌面图标 + 暗码启动

- 设置页新增「隐藏桌面图标」（`settings_hide_launcher_icon`）。
- 开启后通过 `activity-alias` 的 `setComponentEnabledSetting` 关闭启动器入口，应用从桌面与应用列表消失。
- 重新打开：在拨号盘输入 `*#*#5264#*#*`（`SecretCodeReceiver`），或从系统设置 → 应用 → LangSU 打开。

> **实现要点（实测踩坑）**
>
> 1. **组件名必须是全限定名。** `AndroidManifest.xml` 里 `activity-alias` 的 `android:name`
>    若写成 `.ui.LauncherActivity`，清单合并器会按 `namespace` 展开为
>    `com.langsu.manager.ui.LauncherActivity`；而自定义 `applicationId` 时运行期拼接
>    `packageName + ".ui.LauncherActivity"` 会得到 `com.example.weather.ui.LauncherActivity`，
>    两者不等 → `setComponentEnabledSetting` 静默失效。现在两侧都使用同一个全限定名。
> 2. **Receiver 里不能直接 `startActivity`。** Android 10+ 会以
>    `BAL_BLOCK`（background activity launch）拦截，日志为
>    `Abort background activity starts` —— 即使用户正在拨号也一样。
>    因此 `SecretCodeReceiver` 依次尝试：`startActivity`（有悬浮窗权限时才成功）→
>    重新启用 launcher 别名（保证可达）→ 发一条通知，用户点通知进入（点击是允许的前台手势）。

### 2.3 备份与数据外泄面收敛

- `AndroidManifest.xml`：`allowBackup=false`、`fullBackupContent=false`，避免 root 工具的配置随系统备份外流。

### 2.4 内核签名校验指纹

`kernel/Kbuild` 默认值仍是 KernelSU 官方证书指纹，**必须替换为你自己的**：

```make
LANGSU_EXPECTED_SIZE ?= <your cert der length, hex>
LANGSU_EXPECTED_HASH ?= <sha256 of cert der>
```

获取方式（APK 的 v2 签名证书 DER）：

```bash
# 用 apksigner 查看证书
apksigner verify --print-certs your.apk
# 然后对证书 DER 做 sha256，size 为 DER 字节数
```

调试期也可用内核参数临时指定 manager appid（`CONFIG_LANGSU_DEBUG=y` 时）：

```bash
echo <appid> > /sys/module/langsu/parameters/langsu_debug_manager_appid
```

### 2.5 其它

- 驱动 anon inode 名、超级调用 magic（`0x4C414E47` / `0x5355A5A5`）已与 KernelSU 不同，
  基于 KernelSU 特征串的检测无法直接命中。
- 内核模块描述、作者、日志前缀均为 LangSU。
- 文档域名为 `langsu.dev`（若你有自己的域名，建议全局替换）。

## 3. 主题与图标

- 默认强调色改为樱花粉 `#FF9CA8`（`key_color` 默认值）。
- 图标底色 `#FFD6E3`，启动屏底色浅粉 `#FFE4EE`（深色 `#2B171C`）。
- 图标资源为位图，按密度生成（`drawable-{m,h,xh,xxh,xxxh}dpi`）：
  | 资源 | 用途 |
  | --- | --- |
  | `ic_launcher_foreground.png` | 自适应图标前景（透明底，内容占画布 72%） |
  | `ic_launcher_monochrome.png` | 主题图标（系统取 alpha 上色） |
  | `ic_launcher_splash.png` | 启动屏图标（完整圆角卡片） |
  | `ic_app_logo.png` | 应用内 LOGO（关于页、模块快捷方式预览） |
  | `ic_notification.png` | 通知小图标（纯白剪影，24/36/48/72/96） |
- 原矢量图 `ic_launcher_foreground.xml`、`ic_launcher_monochrome.xml`、`ic_logo_vector.xml`
  与 `res/animator/` 的 4 个启动动画已删除（位图图标不需要，空 `drawable/` 目录也会让 aapt 报错）。
- 关于页 Miuix 版对 LOGO 使用 `ColorFilter.tint`，彩色图会被压成色块，因此那里改用单色层。

## 4. 构建

```bash
# 1) 内核（以 LKM 为例，需 DDK 环境）
cd kernel && CONFIG_LANGSU=m make

# 2) userspace daemon
cd userspace/langsud && cargo ndk -t arm64-v8a build --release --platform 31

# 3) 放入 Manager 的 jniLibs
mkdir -p manager/app/src/main/jniLibs/arm64-v8a
cp userspace/langsud/target/aarch64-linux-android/release/langsud \
   manager/app/src/main/jniLibs/arm64-v8a/liblangsud.so

# 4) Manager
cd manager && ./gradlew assembleRelease
```

> **注意：工程路径不能含非 ASCII 字符。** AGP 会直接拒绝（
> `Your project path contains non-ASCII characters`），
> 本次验证即因路径含「改造」而改在 `D:\langsu_build` 下编译。

## 5. 验证记录

已于 Android 15 / API 35（x86_64 模拟器）实机验证：

| 项目 | 结果 |
| --- | --- |
| `:app:compileDebugKotlin` | 通过 |
| `:app:assembleDebug` / `:app:assembleRelease`（含 R8 + CMake 三 ABI） | 通过 |
| 自定义包名 `com.example.weather` + 名称 `Weather` | applicationId、label、launcher alias 均正确 |
| 冷启动（经 `LauncherActivity` 别名） | `Status: ok`，无崩溃 |
| 隐藏图标 | `resolve-activity` 返回 `No activity found` |
| 暗码 `*#*#5264#*#*` | 图标恢复 + 通知（id 5264，渠道「暗码入口」）发出 |
| 图标打包 | 5 个密度 × 5 个资源全部进 APK |

修复的两个实测 bug 见 2.2。

## 6. 已知事项

- `manager/app/src/main/cpp/uapi` 是指向仓库根 `uapi/` 的链接，请勿在复制工程时丢失。
- 若要与官方 KernelSU 模块生态共存，注意模块目录与 `module.prop` 的 `metamodule` 语义未变。
- 内核 `LANGSU_EXPECTED_SIZE/HASH` 仍是上游证书指纹，正式分发前必须替换。
