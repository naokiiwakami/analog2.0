-----
Files
-----
- a20_midi.elf : Miniboard firmware executable file
- a20_midi.hex : Miniboard firmware flash memory data
- a20_midi.c   : Miniboard firmware source code
- LICENSE      : Source code license

----------
Fuse bytes
----------
low      : 0xcf
high     : 0xdf (default)
extended : 0xff (default)

------------------------------------------------
Examples of avrdude command to program Miniboard
------------------------------------------------
Change the programmer type specified by -c option to yours.

Version 6 or later:
$ avrdude -c usbasp -p t2313 -U lfuse:w:a20_midi.elf -U hfuse:w:a20_midi.elf -U efuse:w:a20_midi.elf -U flash:w:a20_midi.elf

All versions:
$ avrdude -c usbasp -p t2313 -U lfuse:w:0xcf:m -U hfuse:w:0xdf:m -U efuse:w:0xff:m -U flash:w:a20_midi.hex