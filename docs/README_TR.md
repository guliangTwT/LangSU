[English](README.md) | [Español](README_ES.md) | [简体中文](README_CN.md) | [繁體中文](README_TW.md) | [日本語](README_JP.md) | [한국어](README_KR.md) | [Polski](README_PL.md) | [Português (Brasil)](README_PT-BR.md) | **Türkçe** | [Русский](README_RU.md) | [Tiếng Việt](README_VI.md) | [Indonesia](README_ID.md) | [עברית](README_IW.md) | [हिंदी](README_IN.md) | [Italiano](README_IT.md)

# LangSU

<img src="https://langsu.dev/logo.png" style="width: 96px;" alt="logo">

Android cihazlar için kernel tabanlı root çözümü.

[![Latest release](https://img.shields.io/github/v/release/langsu-dev/LangSU?label=Release&logo=github)](https://github.com/langsu-dev/LangSU/releases/latest)
[![Weblate](https://img.shields.io/badge/Localization-Weblate-teal?logo=weblate)](https://hosted.weblate.org/engage/kernelsu)
[![Channel](https://img.shields.io/badge/Follow-Telegram-blue.svg?logo=telegram)](https://t.me/KernelSU)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-orange.svg?logo=gnu)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![GitHub License](https://img.shields.io/github/license/langsu-dev/LangSU?logo=gnu)](/LICENSE)

## Özellikler

1. Kernel-tabanlı `su` ve root erişimi yönetimi.
2. [metamodules](https://langsu.dev/guide/metamodule.html)'ye dayalı modül sistemi: Systemless modifikasyonlar için takılabilir altyapı.
3. [Uygulama profili](https://langsu.dev/guide/app-profile.html): Root gücünü bir kafese kapatın.

## Uyumluluk Durumu

LangSU resmi olarak Android GKI 2.0 cihazlarını (5.10+ kernelli) destekler, eski kernellerle de (4.14+) uyumludur, ancak kerneli kendinizin derlemeniz gerekir.

Bununla birlikte; WSA, ChromeOS ve konteyner tabanlı Android'in tamamı desteklenmektedir.

Şu anda `arm64-v8a` ve `x86_64` mimarileri desteklenmektedir.

> [!CAUTION]
> Yeni çekirdek sürümleri, LangSU'nun çökmesine ve `x86_64` üzerinde kernel panic tetiklemesine neden olabilecek uyumsuz bir değişiklik uyguladı! Daha fazla bilgi için web sitesini kontrol edin!

## Kullanım

- [Yükleme yönergeleri](https://langsu.dev/guide/installation.html)
- [Nasıl derlenir?](https://langsu.dev/guide/how-to-build.html)
- [Resmi WEB sitesi](https://langsu.dev/)

## Çeviri

LangSU'nun başka dillere çevrilmesine veya mevcut çevirilerin iyileştirilmesine yardımcı olmak için lütfen [Weblate](https://hosted.weblate.org/engage/kernelsu/) kullanın. Yönetici uygulamasının PR ile çevirisi, Weblate ile çakışacağından artık kabul edilmeyecektir.

## Tartışma

- Telegram: [@LangSU](https://t.me/KernelSU)

## Güvenlik

LangSU'daki güvenlik açıklarını bildirme hakkında bilgi için, bkz [SECURITY.md](/SECURITY.md).

## Lisans

- `kernel` klasöründeki dosyalar [GPL-2-only](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) lisansı altındadır.
- `kernel` klasörü dışındaki bütün diğer bölümler [GPL-3-veya-sonraki](https://www.gnu.org/licenses/gpl-3.0.html) lisansı altındadır.

## Krediler

- [kernel-assisted-superuser](https://git.zx2c4.com/kernel-assisted-superuser/about/): LangSU fikri.
- [Magisk](https://github.com/topjohnwu/Magisk): güçlü root aracı.
- [genuine](https://github.com/brevent/genuine/): apk v2 imza doğrulaması.
- [Diamorphine](https://github.com/m0nad/Diamorphine): bazı rootkit becerileri.
