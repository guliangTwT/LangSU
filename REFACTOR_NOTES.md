# LangSU 改造说明

把 KernelSU 3.3.1 改成 LangSU，并加入隐藏入口与粉色主题。本文只记录**实际做过并验证过**的内容。

- 改造后工程：`KernelSU-3.3.1/`
- 原工程备份：`../_backup_KernelSU-3.3.1/`（未改动）
- 仓库：<https://github.com/guliangTwT/LangSU>（镜像：<https://codeberg.org/GuLiang/LangSU>）

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

## 3. 内核签名指纹（已完成）

已生成 LangSU 专用签名证书并写入内核，无需再手工替换：

```make
# kernel/Kbuild
LANGSU_EXPECTED_SIZE ?= 0x0560
LANGSU_EXPECTED_HASH ?= adfae3276367abc271533f19e1e60b46681e477781f090b3da43f0a80ed8ffab
```

对应证书：

| 项 | 值 |
| --- | --- |
| 密钥库 | `signing/langsu-release.jks`（PKCS12，RSA 4096，有效期 30 年） |
| 别名 | `langsu` |
| 证书 DN | `CN=LangSU, OU=Release, O=LangSU, L=NA, ST=NA, C=CN` |
| DER 长度 | 1376 字节（`0x0560`） |
| SHA-256 | `adfae327...ffab`（与 APK v2 签名指纹一致，已实测核对） |

**密钥库和口令不在仓库里**（`.gitignore` 已排除 `/signing`），需要自己保管并备份。
构建时会自动读取 `signing/langsu-release.properties` 并使用该证书签名；文件不存在时
回退到 debug 签名（那样内核不会认这个 Manager）。

如果要换成别的证书，按下面重算并同步改 `kernel/Kbuild`：

```bash
keytool -exportcert -alias <alias> -keystore <ks> -file cert.der
# 大小：cert.der 的字节数（十六进制），哈希：该文件的 sha256
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

### 4.1 猫娘语气（中文）

中文界面采用"喵~"语气，但**只用在不影响理解的地方**，共 64 条（`scripts/catgirl_strings.ps1`）：

- 加尾巴：标题、状态词、按钮、空状态、刷新提示（「设置喵~」「还没装喵…」「刷新好啦喵~」）。
- **不加尾巴**：版本号、设备型号、路径、错误详情、`%s` 格式化字符串、技术术语
  （`SELinux`、`App Profile`、`WebView`、`KSU` 等）——这些需要一眼看懂和复制粘贴。

配合的排版调整：

- `Type.kt` 补全了 body/title/label 的字号、行高与字距，中文加尾巴后每行多一个字符，
  行高放宽 1~2sp，避免列表行显得拥挤。
- 底部导航另设 `nav_home` / `nav_superuser` / `nav_module` / `nav_settings` 短标签，
  因为底栏是 11sp 且 `softWrap=false`，带尾巴会溢出窄屏。其余位置仍显示猫娘版。

## 5. 文案与链接的处理原则

原工程的文案是在宣传 KernelSU 本身，直接沿用会误导用户，已做这些修改：

| 位置 | 原内容 | 现在 |
| --- | --- | --- |
| 首页「支持开发」 | 跳 `patreon.com/weishu` | 跳上游项目仓库，并注明本项目无捐赠渠道 |
| 首页「了解更多」 | 指向 `langsu.dev`（不存在） | 指向本仓库 |
| 关于页 | 链接上游 GitHub / Telegram 群 | 只链接本仓库，许可证写 GPL-2.0 |
| 更新检查 | `api.github.com/repos/tiann/KernelSU` | 自建清单 `guliang.me/langsu/updates/latest.json`，Codeberg Releases 兜底 |
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
| Release APK 体积 / 签名 | 约 4.4 MB，LangSU 专用证书 v2 签名，可直接安装 |
| APK 签名指纹与内核 `LANGSU_EXPECTED_HASH` | 完全一致（`adfae327...ffab`） |
| 冷启动（经 `LauncherActivity` 别名） | `Status: ok`，无崩溃 |
| 隐藏图标 | `resolve-activity` 返回 `No activity found` |
| 暗码 `*#*#5264#*#*` | 图标恢复 + 通知（id 5264）发出 |
| 自定义包名 `com.example.weather` + `Weather` | applicationId / label / alias 均正确 |
| 猫娘文案渲染 | 首页与设置页实测显示正常，无截断 |
| 全新克隆从零构建 | 111 个任务全部成功 |
| `scripts/check_project.ps1` | 全部通过 |

**未验证的部分：**

- 内核模块 `langsu.ko` 未在本机编译（本机没有内核源码树），只由 CI 的 DDK 容器产出。
- 本机只有 MuMu x86_64 模拟器，没有可以刷内核的真机，所以「刷入内核模块后 Manager 显示已安装」
  这一步没有实测；当前 Manager 显示未安装属于预期。
- 底栏点击在 MuMu 多显示器环境下无法自动化验证，设置页是通过临时改初始页截图确认的。

## 8. 已知事项

- 模块、metamodule、App Profile、sucompat、selinux_hide、sulog 等逻辑与上游一致，未改动。
- 若与官方 KernelSU 共存，两者包名与数据目录已不同，但同一内核只能装一个 root 方案。
- 多语言 strings.xml 是从上游继承的翻译，未逐条校对准确性。

## 9. 发布渠道

GitHub 为主源，Codeberg 做镜像；任何一个不可用都不影响另一个。版本号从 `1.0.0` 重新起算。

| 位置 | 角色 | 内容 |
| --- | --- | --- |
| GitHub Releases（`guliangTwT/LangSU`） | 主源 | 管理器 APK、`lkm-*_langsu.ko`、`langsud-*`、`langsuinit` |
| Codeberg Releases（`GuLiang/LangSU`） | 镜像 | 同一批产物 |
| 自建通道 `https://guliang.me/langsu/updates/` | 补充 | 管理器 APK + `latest.json` 清单 |

客户端更新检查的顺序写死在 `Downloader.kt`：GitHub Releases → Codeberg Releases →
自建清单。三者都失败就静默不提示，不打扰用户。

自建通道目录结构：

- `latest.json`：清单，字段为 `version`、`notes`、`assets[].name` / `size` / `sha256` / `urls`
- `files/<版本>/<文件名>`：APK 实体
- `notes/<版本>.md`：该版本更新日志
- `versions/<版本>.json`：该版本清单快照

生成与上传：

```bash
python scripts/publish_update.py              # 生成 dist/latest.json、notes/、versions/、index.html
python scripts/publish_codeberg_release.py    # 用 git 已存的 Codeberg 凭据建 Release 并传附件
# 自建通道的上传见 REFACTOR_NOTES 里 hk-vhost 的用法（FTP 覆盖写）
```

CI 保留在 `.github/workflows`（GitHub Actions）。`release.yml` 打 tag 时发 GitHub Release，
再用 `CODEBERG_TOKEN` 把同一批产物镜像到 Codeberg；没配 secret 就跳过镜像，不影响 GitHub 发布。
`.github/workflows/deploy-website.yml` 之前删掉了，需要发布文档站时按本地 `yarn docs:build`
后手工上传。

版本号规则：version name 来自 git tag（`v1.0.0` → `1.0.0`），version code 为
`40000 + major*10000 + minor*100 + patch`。那个 40000 的底数是必要的：早期对外发布过
`30000 + 提交数` 的包（30010），如果 version code 掉回 10000，装过旧版的设备无法升级。
