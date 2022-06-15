Simple DOS audio player
=======================

This is a very simple PCM audio player for DOS. The main reason it exists, is as
scaffolding for easier hacking of sound card device drivers, for my various
operating system kernel projects. It's generally easier to rapidly iterate on
driver code under DOS, directly compiling and running, without having to boot a
kernel image every time.

Currently supported audio devices:
  - Creative SoundBlaster 16, AWE32 & AWE64.

License
-------
Copyright (C) 2019-2022  John Tsiombikas <nuclear@member.fsf.org>

This program is free software. Feel free to use, modify, and/or redistribute it
under the terms of the GNU General Public License v3, or at your option any
later version published by the Free Software Foundation. See COPYING for
details.

Build instructions
------------------
Build with the Watcom C compiler. Tested with Watcom 11.0c and OpenWatcom 1.9.

Just type `wmake` to build.
