# Magisk との違い

LangSU モジュールと Magisk モジュールには多くの共通点がありますが、実装の仕組みが全く異なるため、必然的にいくつかの相違点が存在します。Magisk と LangSU の両方でモジュールを動作させたい場合、これらの違いを理解する必要があります。

## 似ているところ

- モジュールファイルの形式：どちらもzip形式でモジュールを整理しており、モジュールの形式はほぼ同じです。
- モジュールのインストールディレクトリ：どちらも `/data/adb/modules` に配置されます。
- システムレス：どちらもモジュールによるシステムレスな方法で /system を変更できます。
- post-fs-data.sh: 実行時間と意味は全く同じです。
- service.sh: 実行時間と意味は全く同じです。
- system.prop：全く同じです。
- sepolicy.rule：全く同じです。
- BusyBox：スクリプトは BusyBox で実行され、どちらの場合も「スタンドアロンモード」が有効です。

## 違うところ

違いを理解する前に、モジュールが LangSU で動作しているか Magisk で動作しているかを区別する方法を知っておく必要があります。環境変数 `LANGSU` を使うとモジュールスクリプトを実行できるすべての場所 (`customize.sh`, `post-fs-data.sh`, `service.sh`) で区別できます。LangSU では、この環境変数に `true` が設定されます。

以下は違いです：

- LangSU モジュールは、リカバリーモードではインストールできません。
- LangSU モジュールには Zygisk のサポートが組み込まれていません（ただし[ZygiskNext](https://github.com/Dr-TSNG/ZygiskNext)を使うと Zygisk モジュールを使用できます）。
- **モジュールマウントアーキテクチャ**：LangSU は [metamodule システム](metamodule.md) を使用し、マウントをプラグ可能な metamodule (`meta-overlayfs`など) に委任します。一方、Magisk はマウントをコアに組み込んでいます。LangSU はモジュールマウントを有効にするために metamodule のインストールが必要です。
- LangSU モジュールにおけるファイルの置換や削除の方法は、Magisk とは全く異なります。LangSU は `.replace` メソッドをサポートしていません。その代わり、`mknod filename c 0 0` で同名のファイルを作成し、対応するファイルを削除する必要があります。
- BusyBox 用のディレクトリが違います。LangSU の組み込み BusyBox は `/data/adb/langsu/bin/busybox` に、Magisk では `/data/adb/magisk/busybox` に配置されます。**これは LangSU の内部動作であり、将来的に変更される可能性があることに注意してください!**
- LangSU は `.replace` ファイルをサポートしていません。しかし、LangSU はファイルやフォルダを削除したり置き換えたりするための `REMOVE` と `REPLACE` 変数をサポートしています。
- LangSU は `boot-completed.sh` スクリプトを追加し、Android システムの起動完了後にタスクを実行できます。
- LangSU は `post-mount.sh` スクリプトを追加し、モジュールマウント完了後にタスクを実行できます。
