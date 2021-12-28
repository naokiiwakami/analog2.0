# ファームウェア

MiniBoard 2 を動かすには、AVR マイクロプロセッサ ATTiny2313 にファームウェアを書き込む必要があります。
ファームウェアは、ビルド済みのものをこのリポジトリからダウンロードするか、ソースコードからビルドすることによって入手できます。

## ファームウェアのダウンロード
リポジトリ内のディレクトリ [10_miniboard/firmware/releases](../firmware/releases/)
からファイル名についている日付の一番新しい zip ファイルをダウンロードしてください。

zip ファイル内には、以下のファイルが含まれています。

- a20_midi.elf : ファームウェアイメージ 
- a20_midi.hex : プログラム領域の hex データファイル
- a20_midi.c : ソースコード
- Readme.txt : Fuse バイト情報など
- LICENSE : ライセンスファイル

## ビルド方法

ビルドには、ソフトウェア Atmel Studio 7 が必要です。

http://www.atmel.com/microsite/atmel-studio/

ディレクトリ [10_miniboard\firmware\miniboard2_firmware](../firmware/miniboard2_firmware) の下の Atmel Studio ソリューションファイル [firmware.atsln](../firmware/firmware.atsln) を開いて、
プロジェクト `miniboard2_firmware` をビルドしてください。ファームウェアはプロジェクトディレクトリの下の `Release` ディレクトリにビルドされます。
ファイル名は `a20_midi.elf` と `a20_midi.hex`です。

## ファームウェアのプロセッサへの書き込み

### 書き込みに必要なもの

ファームウェアの書き込みには、以下のものが必要です。

- **ファームウェアイメージ** <br />
  上述のとおり

- **AVRプログラマ (ハードウェア)** <br />
  「AVRライタ」とも呼ばれます。AVRプロセッサに書き込みを行うためのデバイスです。
  Atmel / Microchip の安価な純正品だった [AVRISP mkII](http://www.atmel.com/tools/avrispmkii.aspx) は残念ながら販売終了してしまいました。ほかに入手可能な純正品には、以下のようなものがあります。

  - [AVR Dragon](http://www.atmel.com/tools/avrdragon.aspx)
  - [ATAtmel-ICE](http://www.atmel.com/tools/atatmel-ice.aspx)

  純正品でない安価なプログラマも入手可能です。

  以下は、書き込みができることを確認したプログラマです。

  - Atmel / Microchip <br />
    - AVRISP mkII http://www.atmel.com/tools/avrispmkii.aspx
  - サードパーティー <br />
    - Aitendo USB-ASP2 http://www.aitendo.com/product/10259<br />
      元となったプログラマ http://www.fischl.de/usbasp/

- **AVRプログラマ (ソフトウェア)** <br />
  実際に書き込みを行うソフトウェアです。Atmel Studio に付属しているので、Atmel Studio が認識できるプログラマなら別にソフトウェアを用意する必要はありません。そうでない場合、[AVRDUDE](http://www.nongnu.org/avrdude/) がよく使われるソフトウェアです。このページの書き込み例でも AVRDUDE を使います。

- **AVRプログラマとMiniboard II プロセッサをつなぐケーブル**

### プログラマ用ドライバのインストール
ほとんどの AVR プログラマが USB で接続するもので、何らかのドライバが必要です。以下は必要なドライバの概略です。

| プログラマ       | Windows                                 | Mac OS                               | Linux |
| ----------- | --------------------------------------- | ------------------------------------ | ----- |
| AVRISP mkII | Atmel Studio インストール時にUSBドライバもインストールする   | TBD                                  | TBD   |
| AVR Dragon  | Atmel Studio インストール時にUSBドライバもインストールする   | TBD                                  | TBD   |
| USB-ASP2    | [USBasp ドライバのインストール](#usbasp_driver) 参照 | 新たなドライバのインストールは不要 (10.11 El Capitan) | TBD   |

### AVRDUDE のインストール

#### Windows

この例では、AVRDUDE を C:\avrdude ディレクトリにインストールします。

1. [AVRDUDE](http://www.nongnu.org/avrdude/) の [ダウンロードエリア](http://download.savannah.gnu.org/releases/avrdude/) に行き、最新版の Windows アーカイブをダウンロードします。(例: avrdude-6.3-mingw32.zip)
2. ディレクトリ C:\avrdude を作成します。
3. AVRDUDE アーカイブに含まれているファイル avrdude.conf と avrdude.exe をディレクトリ C:\avrdude に解凍します。
4. コントロールパネルを起動し、"edit environment variables" を検索します。見つかったリンクのうち、"Edit the system environment variable" をクリックすると、システムプロパティのダイアログが現れます。「環境変数」ボタンを押して、環境変数エディタを起動します。
5. System variables の Path に C:\avrdude を追加します。
6. コマンドプロンプトを立ち上げて、"avrdude -v" とタイプしてください。正しくインストールされていれば、AVRDUDE のバージョンが表示されます。

![avrdude](avrdude_win.png)

#### MacOS

[Homebrew](https://brew.sh/) というソフトウェアを使っていれば以下のように簡単にインストールできます。

```
$ brew install avrdude
```

参考： http://macappstore.org/avrdude/

#### Linux

TBD

### AVRプログラマを MiniBoard に接続

Miniboard 2 の基板には、6本ピンのプログラミング用端子がついています。これは Atmel AVRISP のピン配置に準拠しているので、AVRISP II からプログラミングする場合は、このプログラマについているケーブルの6ピンソケットをそのままさしてください。このピン配置に準拠していないプログラマを使う場合には、そのプログラマのピンと Miniboard 2 のプログラミング端子のピン名が一致するように接続してください。

![target_pinout](target_pinout.png)

Miniboard 上のプログラミング端子を使わず、プロセッサ ATTiny2313 に直接書き込む場合には、以下のようにピンをつないでください。

| プログラマ | プロセッサのピン番号 |
| ----- | ---------- |
| +Vcc  | 20         |
| SCK   | 19         |
| MISO  | 18         |
| MOSI  | 17         |
| GND   | 10         |
| RESET | 1          |

### 書き込み

ここでは、Aitendo USB-ASP2 を MacOS 上で AVRDUDE を使って書き込む例を書きます。この場合には、Windows 以外では特別にドライバソフトウェアなどをインストール必要がないので、比較的簡単に書き込みができます。

USB-ASP2 の出力は 6 ピンではなく 10 ピンです。本体側（オス）コネクタのピン配置は以下の通りです。

![ten_pins_pinout.png]

まずは、ファームウェアイメージのあるディレクトリに移動して、イメージファイルがあることを確認します。

```
$ ls a20_midi.hex
a20_midi.hex
```

次に、AVRDUDE を使ってファームウェアを書き込みます。hex ファイルには、プログラム領域のデータしか入っていないので fuse バイトも明示して書き込みます。

```
$ avrdude -c usbasp -p t2313 -U lfuse:w:0xff:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m -U flash:w:a20_midi.hex
```

a20_midi.elf ファイルには fuse バイト情報も含まれているので、fuse にどんな値を入れるのか気にせず以下のように書き込むこともできます。ただし AVRDUDE はバージョン6以上のものを使う必要があります。

```
$ avrdude -c usbasp -p t2313 -U lfuse:w:a20_midi.elf -U hfuse:w:a20_midi.elf -U efuse:w:a20_midi.elf -U flash:w:a20_midi.elf
```

この操作で書き込みの確認も行われるので、実行が成功すればこれで書き込みは完了です。

参考までに、AVRDUDE の簡単なヘルプは -h オプション (`avrdude -h`) で見られますがあまり詳しくありません。
詳しいマニュアルは https://www.nongnu.org/avrdude/user-manual/avrdude.html で読むことができます。

特にコマンドラインオプションはこちらのページに詳しく書かれています。
https://www.nongnu.org/avrdude/user-manual/avrdude_3.html#Option-Descriptions

### 動作確認
Miniboard のファンクションスイッチを一秒程度押し続けてください。書き込みが成功していれば 440Hz の基準音がスピーカから鳴ります。さらに一秒ほど押し続けると音は止まります。

## 付録

### <a name="usbasp_driver"></a>USBasp ドライバのインストール (Windows)

参考：<br />
http://www.fischl.de/usbasp/<br />
http://ht-deko.com/arduino/usbasp.html

USBasp 互換のデバイスを Windows で使うためには、ドライバをインストールする必要があります。インストールには [Zadig](http://zadig.akeo.ie/) というツールを使います。

Zadig exe ファイルをダウンロードして、USBasp デバイスが USB にささっていない状態で実行してください。

以下のようなユーザ・アクセス・コントロールの承認ウィンドウが現れるので、yes ボタンを押して承認します。

![uac](zadig_uac.png)



承認すると、Zadig アプリケーションが起動します。

![initial](zadig_initial.png)

ここで、USBasp デバイスを PC に接続してください。以下のように、デバイスが認識されます。デバイスのドライバはまだインストールしていないので NONE になっています。インストールするドライバとして libusbK を選んで、"Install Driver" ボタンを押してください。

![before_install](zadig_before_install.png)

インストールに成功すると、以下のような画面になります。

![after_install](zadig_after_install.png)

インストールが行われたかどうかの確認をするためには、コントロールパネル -> ハードウェアの設定 -> デバイスマネージャを呼び出してください。libusbK USB Devices の項目に USBasp が含まれていれば、正しくインストールされています。

![device_check](zadig_device_check.png)
