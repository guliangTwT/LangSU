[English](README.md) | [Español](README_ES.md) | [简体中文](README_CN.md) | [繁體中文](README_TW.md) | [日本語](README_JP.md) | [한국어](README_KR.md) | **Polski** | [Português (Brasil)](README_PT-BR.md) | [Türkçe](README_TR.md) | [Русский](README_RU.md) | [Tiếng Việt](README_VI.md) | [Indonesia](README_ID.md) | [עברית](README_IW.md) | [हिंदी](README_IN.md) | [Italiano](README_IT.md)

# LangSU

<img src="https://langsu.dev/logo.png" style="width: 96px;" alt="logo">

Rozwiązanie root oparte na jądrze dla urządzeń z systemem Android.

[![Latest release](https://img.shields.io/github/v/release/langsu-dev/LangSU?label=Release&logo=github)](https://github.com/langsu-dev/LangSU/releases/latest)
[![Weblate](https://img.shields.io/badge/Localization-Weblate-teal?logo=weblate)](https://hosted.weblate.org/engage/kernelsu)
[![Channel](https://img.shields.io/badge/Follow-Telegram-blue.svg?logo=telegram)](https://t.me/KernelSU)
[![License: GPL v2](https://img.shields.io/badge/License-GPL%20v2-orange.svg?logo=gnu)](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html)
[![GitHub License](https://img.shields.io/github/license/langsu-dev/LangSU?logo=gnu)](/LICENSE)

## Cechy

1. Oparte na jądrze `su` i zarządzanie dostępem roota.
2. System modułów oparty na [metamodules](https://langsu.dev/guide/metamodule.html): Wtykowa infrastruktura dla modyfikacji systemless.

## Kompatybilność

LangSU oficjalnie obsługuje urządzenia z Androidem GKI 2.0 (z jądrem 5.10+), starsze jądra (4.14+) są również kompatybilne, ale musisz sam skompilować jądro.

WSA i Android oparty na kontenerach również powinny działać ze zintegrowanym LangSU.

Obecnie obsługiwane są architektury `arm64-v8a` oraz `x86_64`.

> [!CAUTION]
> Nowsze wersje jądra wprowadzają zmianę, która powoduje awarię LangSU i może wywołać kernel panic na `x86_64`! Sprawdź stronę internetową po więcej informacji!

## Użycie

- [Instalacja](https://langsu.dev/guide/installation.html)
- [Jak skompilować?](https://langsu.dev/guide/how-to-build.html)

## Tłumaczenie

Aby pomóc w tłumaczeniu LangSU lub ulepszyć istniejące tłumaczenia, użyj [Weblate](https://hosted.weblate.org/engage/kernelsu/). PR tłumaczenia Managera nie jest już akceptowany, ponieważ będzie kolidował z Weblate.

## Dyskusja

- Telegram: [@LangSU](https://t.me/KernelSU)

## Bezpieczeństwo

Informacje na temat zgłaszania luk w zabezpieczeniach w LangSU można znaleźć w pliku [SECURITY.md](/SECURITY.md).

## Licencja

- Pliki w katalogu `kernel` są na licencji [GPL-2-only](https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html).
- Wszystkie inne części poza katalogiem `kernel` są na licencji [GPL-3-or-later](https://www.gnu.org/licenses/gpl-3.0.html).

## Podziękowania

- [kernel-assisted-superuser](https://git.zx2c4.com/kernel-assisted-superuser/about/): pomysłodawca LangSU.
- [Magisk](https://github.com/topjohnwu/Magisk): implementacja sepolicy.
- [genuine](https://github.com/brevent/genuine/): walidacja podpisu apk v2.
- [Diamorphine](https://github.com/m0nad/Diamorphine): cenna znajomość rootkitów.
