# ファームウェア

MiniBoard 2 を動かすには、AVR マイクロプロセッサ ATTiny2313 にファームウェアを書き込む必要があります。
ファームウェアは、ビルド済みのものをこの[リポジトリからダウンロードする](../firmware/releases/miniboard_20170927.zip)か、
ソースコードからビルドすることによって入手できます。

### ファームウェアのダウンロード
リポジトリ内のディレクトリ [10_miniboard/firmware/releases](../firmware/releases/)
からファイル名についている日付の一番新しい zip ファイルをダウンロードしてください。現在の最新版は miniboard_20110226.zip です。
zip ファイル内には、以下のファイルが含まれています。
- a20_midi.elf : ファームウェアイメージ 
- a20_midi.hex : プログラム領域の hex データファイル
- a20_midi.c : ソースコード
- Readme.txt : Fuse バイト情報など
- LICENSE : ライセンスファイル

### ビルド方法

ビルドには、ソフトウェア Atmel Studio 7 が必要です。

http://www.atmel.com/microsite/atmel-studio/

ディレクトリ [10_miniboard\firmware\miniboard2_firmware](../firmware/miniboard2_firmware) の下の Atmel Studio ソリューションファイル [firmware.atsln](../firmware/firmware.atsln) を開いて、
プロジェクト `miniboard2_firmware` をビルドしてください。ファームウェアはプロジェクトディレクトリの下の `Release` ディレクトリにビルドされます。
ファイル名は `a20_midi.elf` と `a20_midi.hex`です。

### ファームウェアのプロセッサへの書き込み

#### 書き込みに必要なもの

ファームウェアの書き込みには、以下のものが必要です。

- **ファームウェアイメージ** 
  上述のとおり

- **AVRプログラマ (ハードウェア)**
  「AVRライタ」とも呼ばれます。AVRプロセッサに書き込みを行うためのハードウェアデバイスです。
  Atmel / Microchip の安価な純正品だった AVRISP mkII は残念ながら販売終了してしまいました。ほかに入手可能な純正品には、以下のようなものがあります。

  - AVR Dragon
  - ATAtmel-ICE

  純正品でない安価なプログラマも入手可能です。

  以下は、書き込みができることを確認したプログラマです。

  - Atmel / Microchip
    - AVRISP mkII http://www.atmel.com/tools/avrispmkii.aspx (Windows)
  - サードパーティー
    - Aitendo USB-ASP2 http://www.aitendo.com/product/10259 (MacOS)<br />
      元となったプログラマ http://www.fischl.de/usbasp/

- **AVRプログラマ (ソフトウェア)**
  実際に書き込みを行うソフトウェアです。Atmel Studio に付属しているので、Atmel Studio が認識できるプログラマなら別にソフトウェアを用意する必要はありません。そうでない場合、[AVRDUDE](http://www.nongnu.org/avrdude/) がよく使われるソフトウェアです。このページの書き込み例でも AVRDUDE を使います。

- AVRプログラマとMiniboard II プロセッサをつなぐケーブル

#### AVRDUDE のインストール

##### MacOS

[Homebrew](https://brew.sh/) というソフトウェアを使っていれば以下のように簡単にインストールできます。

```
$ brew install avrdude
```

参考： http://macappstore.org/avrdude/

##### Windows

TBD

##### Linux

TBD

#### AVRプログラマを MiniBoard に接続

Miniboard 2 の基板には、6本ピンのプログラミング用端子がついています。これは Atmel AVRISP のピン配置に準拠しているので、AVRISP II からプログラミングする場合は、このプログラマについている6ピンソケットのケーブルをそのままさしてください。このピン配置に準拠していないプログラマを使う場合には、そのプログラマのピンと Miniboard 2 のプログラミング端子のピン名が一致するように接続してください。

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

#### 書き込み

ここでは、Aitendo USB-ASP2 を MacOS 上で AVRDUDE を使って書き込む例を書きます。この場合には、特別にドライバソフトウェアなどをインストール必要がないので、比較的簡単に書き込みができます。

まずは、ファームウェアイメージのあるディレクトリに移動して、イメージファイルがあることを確認します。

```
$ ls a20_midi.hex
a20_midi.hex
```

次に、AVRDUDE を使ってファームウェアを書き込みます。hex ファイルには、プログラム領域のデータしか入っていないので fuse バイトも明示して書き込みます。

```
$ avrdude -c usbasp -p t2313 -U lfuse:w:0xcf:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m -U flash:w:a20_midi.hex
```

この操作で書き込みの確認も行われるので、実行が成功すればこれで書き込みは完了です。

次に動作確認です。Miniboard のファンクションスイッチを一秒程度押し続けてください。書き込みが成功していれば 440Hz の基準音がスピーカから鳴ります。さらに一秒ほど押し続けると音は止まります。
