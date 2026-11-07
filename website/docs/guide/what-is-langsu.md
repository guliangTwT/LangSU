# What is LangSU?

LangSU is a root solution for Android GKI devices. It works in kernel mode and grants root permission to userspace apps directly in kernel space.

## Features

The main feature of LangSU is that it's **kernel-based**. LangSU works in kernel mode, enabling it to provide a kernel interface that we never had before. For example, it's possible to add hardware breakpoints to any process in kernel mode, access the physical memory of any process invisibly, intercept any system call (syscall) within the kernel space, among other functionalities.

Additionally, LangSU provides a [metamodule system](metamodule.md), which is a pluggable architecture for module management. Unlike traditional root solutions that bake mounting logic into their core, LangSU delegates this to metamodules. This allows you to install metamodules like [meta-overlayfs](https://github.com/tiann/KernelSU/tree/main/userspace/meta-overlayfs) to provide systemless modifications to the `/system` partition and other partitions.

## How to use LangSU?

See [Installation](installation.md).

## How to build LangSU?

See [How to build](how-to-build.md).

## Discussion

- Telegram: [@LangSU](https://t.me/KernelSU)
