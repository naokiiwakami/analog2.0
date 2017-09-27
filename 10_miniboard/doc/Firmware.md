# ファームウェア

MiniBoard 2 を動かすには、AVR マイクロプロセッサ ATTiny2313 にファームウェアを書き込む必要があります。
ファームウェアは、ビルド済みのものをこの[リポジトリからダウンロードする](../firmware/releases/miniboard_20110226.zip)か、
ソースコードからビルドすることによって入手できます。

### ファームウェアのダウンロード
リポジトリ内のディレクトリ [10_miniboard/firmware/releases](../10_miniboard/firmware/releases/)
からファイル名についている日付の一番新しい zip ファイルをダウンロードしてください。現在の最新版は miniboard_20110226.zip です。
zip ファイル内には、以下のファイルが含まれています。
- a20_midi.hex ファームウェアイメージ
- a20_midi.c ソースコード

### ビルド方法

ビルドには、ソフトウェア Atmel Studio 7 が必要です。

http://www.atmel.com/microsite/atmel-studio/

ディレクトリ `10_miniboard\firmware\miniboard2_firmware` の下の Atmel Studio ソリューションファイル firmware.atsln を開いて、
プロジェクト `miniboard2_firmware` をビルドしてください。ファームウェアはプロジェクトディレクトリの下の `Debug` ディレクトリにビルドされます。
ファイル名は miniboard2_firmware.hex です。
