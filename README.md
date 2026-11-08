# LangSU

A modified build of [KernelSU](https://github.com/tiann/KernelSU) 3.3.1 with rebranded naming,
a pink theme, and a few extra hiding options.

KernelSU is a kernel-based `su` and root manager for Android. LangSU keeps that
functionality and changes the identifiers, so it does not collide with an installed
KernelSU.

**This is an unofficial fork. It is not affiliated with, endorsed by, or supported by the
KernelSU authors.** Please do not report LangSU problems to the upstream project.

## Download

- 下载页（管理器 APK + SHA-256）：<https://guliang.me/langsu/updates/>
- 更新清单：<https://guliang.me/langsu/updates/latest.json>
- 源码：<https://codeberg.org/GuLiang/LangSU>

管理器内置 8 个 KMI 版本的内核模块（android12-5.10 到 android17-6.18），拿到 root 后
直接在管理器里安装，不用另外找 `.ko`。

## What is different

| | KernelSU 3.3.1 | LangSU |
| --- | --- | --- |
| Manager package | `me.weishu.kernelsu` | `com.langsu.manager` (overridable) |
| Kernel module | `kernelsu.ko`, `CONFIG_KSU` | `langsu.ko`, `CONFIG_LANGSU` |
| Userspace daemon | `ksud` | `langsud` |
| Data directory | `/data/adb/ksu` | `/data/adb/langsu` |
| SELinux domain | `u:r:ksu:s0` | `u:r:langsu:s0` |
| URL scheme | `ksu://` | `langsu://` |

Added on top of upstream:

- **Custom or randomised package name at build time**, plus a launcher label override.
- **Hideable launcher icon** (settings toggle). Bring it back by dialling
  `*#*#5264#*#*`, which also re-enables the icon and posts a notification.
- **Different hook signature**: the install-fd magic and the anon-inode names are not the
  KernelSU ones, so scans for the upstream constants do not match.
- **Pink theme**: sakura accent colour by default, pink icon and splash background.
- **Catgirl tone (Chinese only)**: headings and status text end in "喵~", while versions,
  paths, error details and technical terms stay plain so the UI remains readable. The
  bottom navigation uses short labels to avoid overflow.
- **No backup export**: `allowBackup` and `fullBackupContent` are off.

Everything else - modules, metamodules, App Profile, su compat, SELinux hiding, su log -
works as in upstream. Module ZIPs and `module.prop` semantics are unchanged.

## Requirements

- Android device with a GKI kernel (5.10+). Older kernels need a manual kernel build.
- `arm64-v8a`, `x86_64` or `riscv64`.
- Unlocked bootloader for flashing.

## Status

The Manager app builds and runs; it was checked on an Android 15 (API 35) x86_64 emulator:
cold start, launcher hiding, the secret-code entry, the notification entry and the
self-hosted update check were verified. The kernel modules are built by CI
(`.forgejo/workflows`, DDK containers, 8 KMIs) and are bundled inside the released APK;
they have **not** been flashed here - no device with a matching KMI was available.

## Building

The project path must not contain non-ASCII characters; the Android Gradle plugin refuses
to build otherwise.

```bash
# Kernel module (needs a kernel tree / DDK environment)
cd kernel && CONFIG_LANGSU=m make LANGSU_PACKAGE_NAME=com.langsu.manager

# Userspace daemon (needs cargo-ndk and the NDK)
cd userspace/langsud && cargo ndk -t arm64-v8a build --release --platform 31

# Manager
mkdir -p manager/app/src/main/jniLibs/arm64-v8a
cp userspace/langsud/target/aarch64-linux-android/release/langsud \
   manager/app/src/main/jniLibs/arm64-v8a/liblangsud.so
cd manager && ./gradlew assembleRelease
```

Stealth build options, all optional:

```bash
./gradlew assembleRelease -PLANGSU_PACKAGE_NAME=com.example.app
./gradlew assembleRelease -PLANGSU_NAME="Weather"
./gradlew assembleRelease -PLANGSU_RANDOM_PACKAGE=true
./gradlew assembleRelease -PLANGSU_RANDOM_PACKAGE=true -PLANGSU_PACKAGE_SEED=my-device
```

The kernel must be built with the same `LANGSU_PACKAGE_NAME`, otherwise the Manager is not
recognised. See [REFACTOR_NOTES.md](REFACTOR_NOTES.md) for the full list of changes and
the remaining caveats.

Release builds are signed with the key described in `signing/` (not in this repository).
The kernel pins that certificate through `LANGSU_EXPECTED_SIZE` / `LANGSU_EXPECTED_HASH`
in `kernel/Kbuild`, so a build signed with a different key will not be treated as the
manager. Replace both values if you swap the key.

## Licences and credits

- KernelSU is licensed under GPL-2.0; this fork keeps that licence (see `LICENSE`).
- The artwork in `scripts/icon/source.png` was supplied by the project owner.
- Translation files and documentation were inherited from upstream and have not been
  re-checked for accuracy.
