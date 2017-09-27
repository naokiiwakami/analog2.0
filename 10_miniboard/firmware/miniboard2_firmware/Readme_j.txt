-----
ファイル
-----
- a20_midi.elf : Miniboard ファームウェア 実行ファイル
- a20_midi.hex : Miniboard ファームウェア フラッシュメモリデータ
- a20_midi.c   : Miniboard ファームウェア ソースコード
- LICENSE      : ライセンスファイル

---------
Fuse バイト
---------
low      : 0xcf
high     : 0xdf (default)
extended : 0xff (default)

-----------------------------------------------
avrdude で Miniboard ファームウェアを書き込むときのコマンド例
-----------------------------------------------
-c オプションで指定するプログラマを使っているものに変えてください

Version 6 以降：
$ avrdude -c usbasp -p t2313 -U lfuse:w:a20_midi.elf -U hfuse:w:a20_midi.elf -U efuse:w:a20_midi.elf -U flash:w:a20_midi.elf

すべてのバージョン：
$ avrdude -c usbasp -p t2313 -U lfuse:w:0xcf:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m -U flash:w:a20_midi.hex