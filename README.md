id3ed-fixes v1.10.5 Copyright 2015 Sahan Fernando <sahan.h.fernando@gmail.com>
Forked from id3ed v1.10.4, Copyright 1998-2001,2003 Matthew Mueller <donut@azstarnet.com>
#id3ed
**id3ed** is a program for the editing of id3-tags. **id3ed-fixes** is a set of **patches** to tidy up and advance id3ed, which is no longer in development.

##New Features & Additions:

####The -m/-M flags

These flags were in the 1.10.4 release of id3ed, but were undocumented and 
unfinished. These options were originally meant to **rename** the target files in 
terms of the id3 tag values, ie rename the file to 
**'[TRACK NUMBER]-[SONG NAME].mp3'**, 
but have been modified to rename the file based on a provided format string. 
**-M** will actually **rename** the file, but **-m** will only perform a **dry run.**

#####Example

    $ id3ed -M "%s - %n" foo.mp3
    foo.mp3->Let It Die - The Foo Fighters.mp3

####Use of -[SNAYCKG] and -q with the -i flag

The **-i** flag is used to print the id3 info of the given file. This flag can be 
used with **-[SNAYCKG]** to **choose which fields to print**, and with **-q** to to omit the **field name**, and the **tag version**.

#####Example

    $ id3ed -iSN foo.mp3
    foo.mp3: (tag v1.1?)
    songname: Let It Die
    artist: The Foo Fighters 
    $ id3ed -iSNq foo.mp3
    Let It Die
    The Foo Fighters 

##Install:
    1 ./configure
    2 make
    3 make install

For more install info: **./configure --help**

##Copying:
This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version. 
See the file 'COPYING' for more information.

You can get the latest version at 
https://github.com/ccapitalK/id3ed-fixes

You can get the original id3ed tarball at
http://code.fluffytapeworm.com/projects/id3ed/
