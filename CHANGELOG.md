# 更新日志

版本号从 1.0.0 重新开始。之前的 1.0.0 / 1.0.1 / 1.0.3 是迁移过程中的中间产物，
不再作为对外版本；对应 APK 留在 `dist/archive/` 里备查。

## 1.0.0（2026-11-08）

首个正式版本。

- 基于 KernelSU 3.3.1 完成改名：包名 `com.langsu.manager`、内核模块 `langsu.ko`、
  daemon `langsud`、数据目录 `/data/adb/langsu`、SELinux domain `u:r:langsu:s0`
- 桌面图标隐藏，拨号盘 `*#*#5264#*#*` 找回（重新启用图标 + 发通知）
- 萌粉主题，中文界面改为猫娘语气，技术信息（版本号、路径、报错）保持原样
- 内置 8 个 KMI 版本的内核模块（android12-5.10 ~ android17-6.18），拿到 root 后
  在管理器里直接安装，自动按设备 KMI 匹配
- 更新检查：GitHub Releases 为主源，GitHub 不可达时退到 Codeberg Releases，
  两者都失败再读自建清单 `guliang.me/langsu/updates/latest.json`
- 更新提示显示版本号，并附带发布产物的 SHA-256
- 源码：GitHub 主仓，Codeberg 镜像
