Xilinx JTAG Programmer for Win32/Linux
======================================

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

THIS VERSION IS A BETA VERSION!

ONLY SPARTAN3 AND XCF04S WAS TESTED ON REAL HARDWARE!

PLEASE SEND FEEDBACKS TO zoltan_csizmadia at yahoo dot com!

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

Supported operations:
---------------------

Detect device chain:

    detect

Erase device:

    erase pos

    E.g.: erase 1

Program device:

    program [-bit|-bin|-mcs] pos file

    E.g.: program 1 c:\untitled.mcs
          program 2 download.bit

Read device:
 
    read [-bin|-mcs] pos file

    E.g.: read 1 c:\untitled.mcs

Select programmer cable:
    
    Xilinx Parallel III
    cable xil3 [ioaddr]

    Digilent USB
    cable dusb

Supported cables:
-----------------

Xilinx Parallel III
    Win32: giveio.sys must be installed to enable user-mode I/O access!

Digilent USB
    Win32: Digilent driver must be installed!
    Linux: libusb must be installed!


Supported devices [Detect/Erase/Program/Read]:
----------------------------------------------

Xilinx Spartan-2                         [D P ]
    xc2s15
    xc2s30
    xc2s50
    xc2s100
    xc2s150
    xc2s200
Xilinx Spartan-2E                        [D P ]
    xc2s50e
    xc2s100e
    xc2s150e
    xc2s200e
    xc2s300e
    xc2s400e
    xc2s600e
Xilinx Spartan-3                         [D P ]
    xc3s50
    xc3s200
    xc3s400
    xc3s1000
    xc3s1500
    xc3s2000
    xc3s4000
    xc3s5000
Xilinx Spartan-3E                        [D P ]
    xc3s100e
    xc3s250e
    xc3s500e
    xc3s1200e
    xc3s1600e
Xilinx Virtex                            [D P ]
    xcv50
    xcv100
    xcv150
    xcv200
    xcv300
    xcv400
    xcv600
    xcv800
    xcv1000
Xilinx VirtexE                           [D P ]
    xcv50e
    xcv100e
    xcv200e
    xcv300e
    xcv400e
    xcv405e
    xcv600e
    xcv812e
    xcv1000e
    xcv1600e
    xcv2000e
    xcv2600e
    xcv3200e
Xilinx Virtex-2                          [D P ]
    xc2v40
    xc2v80
    xc2v250
    xc2v500
    xc2v1000
    xc2v1500
    xc2v2000
    xc2v3000
    xc2v4000
    xc2v6000
    xc2v8000
Xilinx Virtex-2 Pro                      [D P ]
    xc2vp2
    xc2vp4
    xc2vp7
    xc2vp20
    xc2vp30
    xc2vp40
Xilinx Virtex-4                          [D P ]
    xc4v15lx
    xc4v25lx
    xc4v40lx
    xc4v60lx
    xc4v80lx
    xc4v100lx
    xc4v160lx
    xc4v25sx
    xc4v35sx
    xc4v55sx
    xc4v12fx
    xc4v20fx
    xc4v40fx
    xc4v60fx
    xc4v100fx
    xc4v140fx
Xilinx Virtex-5                          [D   ]
    xc5v30lx
    xc5v50lx
    xc5v85lx
    xc5v110lx
    xc5v220lx
    xc5v330lx
Xilinx XCF00S Platform Flash             [DEPR]
    xcf01s
    xcf02s
    xcf04s
Xilinx XC18V00 Platform Flash            [DEPR]
    xc18v256
    xc18v512
    xc18v01
    xc18v02
    xc18v04
Xilinx XCF00P Platform Flash             [D   ]
    xcf08p
    xcf16p
    xcf32p
Xilinx XCR3000XL CPLD                    [D   ]
    xcr3032xl
    xcr3064xl
    xcr3128xl
    xcr3256xl
    xcr3384xl
    xcr3512xl
Xilinx XC9500 CPLD                       [D   ]
    xc9536
    xc9572
    xc95108
    xc95144
    xc95216
    xc95288
Xilinx XC9500XL CPLD                     [D   ]
    xc9536xl
    xc9572xl
    xc95108xl
    xc95144xl
    xc95216xl
    xc95288xl
Xilinx XC9500XV CPLD                     [D   ]
    xc9536xv
    xc9572xv
    xc95144xv
    xc95288xv

License:
--------

xilprg is covered by the LGPL:

Copyright (c) 2006 Zoltan Csizmadia <zoltan_csizmadia@yahoo.com>
Copyright (c) 2008 Benedikt Heinz <Zn000h@googlemail.com>
All rights reserved.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the
Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.

Copyright (c) 2006 Zoltan Csizmadia <zoltan_csizmadia@yahoo.com>
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
