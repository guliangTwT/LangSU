# 隐藏功能

## langsurc

默认情况下，`/system/bin/sh` 会加载 `/system/etc/mkshrc`。

可以通过创建 `/data/adb/langsu/.langsurc` 文件来让 su 加载该文件而不是 `/system/etc/mkshrc`。