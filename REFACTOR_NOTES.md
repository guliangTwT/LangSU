# LangSU 改造说明

把 KernelSU 3.3.1 改成 LangSU，并加入隐藏入口与粉色主题。本文只记录**实际做过并验证过**的内容。

- 改造后工程：`KernelSU-3.3.1/`
- 原工程备份：`../_backup_KernelSU-3.3.1/`（未改动）
- 仓库：<https://github.com/GuLiang-I/LangSU>

---

## 1. 命名对照

| 项目 | 原 KernelSU | LangSU |
| --- | --- | --- |
| Manager 包名 / 命名空间 | `me.weishu.kernelsu` | `com.langsu.manager` |
| 内核模块 | `kernelsu.ko`、`CONFIG_KSU` | `langsu.ko`、`CONFIG_LANGSU` |
| 用户态守护进程 | `ksud` | `langsud` |
| 早期 init | `ksuinit` | `langsuinit` |
| 数据目录 | `/data/adb/ksu` | `/data/adb/langsu` |
| daemon 路径 | `/data/adb/ksud` | `/data/adb/langsud` |
| SELinux domain | `u:r:ksu:s0` | `u:r:langsu:s0` |
| 深链接 scheme | `ksu://` | `langsu://` |
| 驱动 anon inode | `[ksu_driver]` | `[langsu_driver]` |
| Manager JNI 库 | `libkernelsu.so` | `liblangsu.so` |

## 2. 隐藏相关改动

### 2.1 构建期改包名 / 应用名

`manager/app/build.gradle.kts` 支持这些参数：

```bash
./gradlew assembleRelease -PLANGSU_PACKAGE_NAME=com.example.weather
./gradlew assembleRelease -PLANGSU_NAME="Weather"
./gradlew assembleRelease -PLANGSU_RANDOM_PACKAGE=true
./gradlew assembleRelease -PLANGSU_RANDOM_PACKAGE=true -PLANGSU_PACKAGE_SEED=my-device
```

内核必须用同一个 `LANGSU_PACKAGE_NAME` 构建，否则 Manager 不被识别。

### 2.2 隐藏桌面图标 + 暗码找回

设置页新增「隐藏桌面图标」。开启后关闭 `activity-alias`，应用从桌面和应用列表消失。
拨号盘输入 `*#*#5264#*#*` 找回。

**两个实测踩到的坑（已修）：**

1. **组件名必须是全限定名。**
   清单里写 `.ui.LauncherActivity` 时，清单合并器会按 `namespace` 展开；而运行期若用
   `packageName + ".ui.LauncherActivity"` 拼接，在自定义 `applicationId` 下两者不相等，
   `setComponentEnabledSetting` 会静默失效。现在两侧统一使用
   `com.langsu.manager.ui.LauncherActivity`。

2. **Receiver 里不能直接 `startActivity`。**
   Android 10+ 会以 `BAL_BLOCK` 拦截，日志为 `Abort background activity starts`。
   现在按可靠性依次尝试：`startActivity`（有悬浮窗权限时才成功）→ 重新启用桌面图标 →
   发一条通知（点击通知是允许的前台手势）。

### 2.3 其他

- `allowBackup=false`、`fullBackupContent=false`，避免配置随系统备份外流。
- 安装 fd 的 magic 改为 `0x4C414E47` / `0x5355A5A5`，与 KernelSU 的
  `0xDEADBEEF` / `0xCAFEBABE` 不同，特征串扫描无法直接命中。

## 3. 内核签名指纹（正式分发前必须改）

`kernel/Kbuild` 里的默认值仍是 KernelSU 官方证书：

```make
LANGSU_EXPECTED_SIZE ?= 0x033b
LANGSU_EXPECTED_HASH ?= c371061b...
```

用自己的证书签名后，必须换成对应值，否则内核不认这个 Manager：

```bash
apksigner verify --print-certs your.apk   # 取证书 DER
# 对 DER 做 sha256 -> EXPECTED_HASH；DER 字节数 -> EXPECTED_SIZE（十六进制）
```

## 4. 主题与图标

- 默认强调色樱花粉 `#FF9CA8`；图标底 `#FFD6E3`；启动屏 `#FFE4EE`（深色 `#2B171C`）。
- 图标为位图，5 个密度 x 5 类资源：

  | 资源 | 用途 |
  | --- | --- |
  | `ic_launcher_foreground.png` | 自适应图标前景（透明底，占画布 72%） |
  | `ic_launcher_monochrome.png` | 主题图标（系统取 alpha 上色） |
  | `ic_launcher_splash.png` | 启动屏 |
  | `ic_app_logo.png` | 应用内 LOGO |
  | `ic_notification.png` | 通知小图标（纯白剪影） |

- 原矢量图与 `res/animator/` 的启动动画已删除（位图不需要；空的 `drawable/` 会让 aapt 报错）。
- 重新生成：`powershell -File scripts/icon/make_icons.ps1 -MagickPath <magick.exe>`（需 ImageMagick 7）。
- 关于页 Miuix 版对 LOGO 用 `ColorFilter.tint`，彩色图会被压成色块，因此那里改用单色层。

## 5. 文案与链接的处理原则

原工程的文案是在宣传 KernelSU 本身，直接沿用会误导用户，已做这些修改：

| 位置 | 原内容 | 现在 |
| --- | --- | --- |
| 首页「支持开发」 | 跳 `patreon.com/weishu` | 跳上游项目仓库，并注明本项目无捐赠渠道 |
| 首页「了解更多」 | 指向 `langsu.dev`（不存在） | 指向本仓库 |
| 关于页 | 链接上游 GitHub / Telegram 群 | 只链接本仓库，许可证写 GPL-2.0 |
| 更新检查 | `api.github.com/repos/tiann/KernelSU` | 本仓库 releases |
| `SECURITY.md` | 让用户发给 weishu 私人邮箱 | 指向本仓库安全公告，并说明是 fork |
| `docs/README_*.md` | 15 份上游宣传文案 | 已删除 |
| 虚构的 `langsu.dev` / `langsu-dev` | 全项目 57+ 处 | 改为真实地址 |

> 注意：`modules.kernelsu.org`、`mui.kernelsu.org`、`kernelsu.org/templates` 是上游**真实运行**的服务，
> 模块浏览、模板下载、WebUI 首页依赖它们，因此**保留原样未改**。

## 6. 构建

> **工程路径不能含非 ASCII 字符。** AGP 的 AIDL 编译器用平台默认编码读写生成文件，
> 中文路径会报 `MalformedInputException`。设 `android.overridePathCheck=true` 只能压掉提示，
> 不能让构建通过——已实测验证，所以没有设这个开关。

```bash
# 1) 内核模块（需要内核源码树 / DDK 环境）
cd kernel && CONFIG_LANGSU=m make LANGSU_PACKAGE_NAME=com.langsu.manager

# 2) userspace daemon
cd userspace/langsud && cargo ndk -t arm64-v8a build --release --platform 31

# 3) 放进 jniLibs
mkdir -p manager/app/src/main/jniLibs/arm64-v8a
cp userspace/langsud/target/aarch64-linux-android/release/langsud \
   manager/app/src/main/jniLibs/arm64-v8a/liblangsud.so

# 4) Manager
cd manager && ./gradlew assembleRelease
```

## 7. 验证记录

环境：Windows 11、JDK 21、Gradle 9.8.2、AGP 9.4.1、NDK 29.0.14206865；
设备：MuMu 模拟器 Android 15 / API 35 / x86_64。

| 项目 | 结果 |
| --- | --- |
| `:app:assembleDebug` / `:app:assembleRelease`（R8 + CMake 三 ABI） | 通过 |
| Release APK 体积 / 签名 | 约 4 MB，debug 密钥 v2 签名，可直接安装 |
| 冷启动（经 `LauncherActivity` 别名） | `Status: ok`，无崩溃 |
| 隐藏图标 | `resolve-activity` 返回 `No activity found` |
| 暗码 `*#*#5264#*#*` | 图标恢复 + 通知（id 5264）发出 |
| 自定义包名 `com.example.weather` + `Weather` | applicationId / label / alias 均正确 |
| 全新克隆从零构建 | 111 个任务全部成功 |
| `scripts/check_project.ps1` | 全部通过 |

**未验证的部分：**

- 内核模块 `langsu.ko` 未编译、未刷入（本机没有内核源码树，也没有对应 KMI 的设备）。
- Rust daemon `langsud` 未纳入 Release 产物（未完成构建验证）。
- 因此 Manager 里显示「未安装 / 点击安装」是**预期**的：没有内核模块它就是未安装状态。

## 8. 已知事项

- 模块、metamodule、App Profile、sucompat、selinux_hide、sulog 等逻辑与上游一致，未改动。
- 若与官方 KernelSU 共存，两者包名与数据目录已不同，但同一内核只能装一个 root 方案。
- 多语言 strings.xml 是从上游继承的翻译，未逐条校对准确性。
