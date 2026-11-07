[English](README.md) | [Español](README_ES.md) | [简体中文](README_CN.md) | [繁體中文](README_TW.md) | **日本語** | [한국어](README_KR.md) | [Polski](README_PL.md) | [Português (Brasil)](README_PT-BR.md) | [Türkçe](README_TR.md) | [Русский](README_RU.md) | [Tiếng Việt](README_VI.md) | [Indonesia](README_ID.md) | [עברית](README_IW.md) | [हिंदी](README_IN.md) | [Italiano](README_IT.md)

# LangSU

<img src="https://langsu.dev/logo.png" style="width: 96px;" alt="logo">

Android におけるカーネルベースの root ソリューションです。

[![Latest release](https://img.shields.io/github/v/release/langsu-dev/LangSU?label=Release&logo=github)](https://github.com/langsu-dev/LangSU/releases/latest)
[![Weblate](https://img.shields.io/badge/Localization-Weblate-teal?logo=weblate)](https://hosted.weblate.org/engage/kernelsu)
[![Channel](https://img.shields.io/badge/Follow-Telegram-blue.svg?logo=telegram)](https://t.me/KernelSU)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-orange.svg?logo=gnu)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![GitHub License](https://img.shields.io/github/license/langsu-dev/LangSU?logo=gnu)](/LICENSE)

## 特徴

1. カーネルベースの `su` と権限管理。
2. [metamodules](https://langsu.dev/ja_JP/guide/metamodule.html) に基づくモジュールシステム: プラグイン可能なシステムレス変更インフラストラクチャ。
3. [アプリのプロファイル](https://langsu.dev/guide/app-profile.html): root の権限をケージ内に閉じ込めます。

## 対応状況

LangSU は GKI 2.0 デバイス（カーネルバージョン 5.10 以上）を公式にサポートしています。古いカーネル（4.14以上）とも互換性がありますが、自分でカーネルをビルドする必要があります。

WSA 、ChromeOS とコンテナ上で動作する Android でも LangSU を統合して動かせます。

現在、`arm64-v8a` および `x86_64` アーキテクチャがサポートされています。

> [!CAUTION]
> 最近のカーネルバージョンでは、LangSU が機能しなくなり、`x86_64` 上でカーネルパニックを引き起こす可能性のある破壊的な変更が実装されています！詳細についてはウェブサイトを確認してください！

## 使用方法

- [インストール方法はこちら](https://langsu.dev/ja_JP/guide/installation.html)
- [ビルド方法はこちら](https://langsu.dev/guide/how-to-build.html)
- [公式サイト](https://langsu.dev/ja_JP/)

## 翻訳

LangSU をあなたの言語に翻訳するか、既存の翻訳を改善するには、[Weblate](https://hosted.weblate.org/engage/kernelsu/) を使用してください。Manager翻訳した PR は、Weblate と競合するため受け入れられなくなりました。

## ディスカッション

- Telegram: [@LangSU](https://t.me/KernelSU)

## ライセンス

- `kernel` ディレクトリの下にあるすべてのファイル： [GPL-2-only](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)。
- `kernel` ディレクトリ以外のすべてのファイル： [GPL-3.0-or-later](https://www.gnu.org/licenses/gpl-3.0.html)。

## クレジット

- [kernel-assisted-superuser](https://git.zx2c4.com/kernel-assisted-superuser/about/)：LangSU のアイデア元。
- [Magisk](https://github.com/topjohnwu/Magisk)：強力な root ツール。
- [genuine](https://github.com/brevent/genuine/)：apk v2 の署名検証。
- [Diamorphine](https://github.com/m0nad/Diamorphine): rootkit のスキル。
