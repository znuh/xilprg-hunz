/*
xilprg is covered by the LGPL:

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
*/

#ifndef _UTILS_H_INCLUDED_
#define _UTILS_H_INCLUDED_

#define NTOH8(x)	(x)
#define NTOH16(x)	((u16) \
	((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8)))
#define NTOH32(x)	((u32) \
	((((x) & 0x000000ff) << 24) | \
	(((x) & 0x0000ff00) <<  8) | \
	(((x) & 0x00ff0000) >>  8) | \
	(((x) & 0xff000000) >> 24) ))

#define HTON8(x)    NTOH8(x)
#define HTON16(x)   NTOH16(x)
#define HTON32(x)   NTOH32(x)

#ifndef WIN32
#define UNUSED(x)
#endif

int enable_user_mode_io();

//
u8 reverse8(u8);
u32 reverse32(u32);

void readline_load_history(void);
void readline_save_history(void);

#if defined(WIN32) && !defined(__CYGWIN__)
u8 inb(unsigned int addr);
void outb(u8 data, unsigned int addr);
#endif

int set_current_directory(const char*);
int get_current_directory(char*, int);

int prompt_read_line(const char* prompt, char* line, int maxlen);

int is_whitespace(char c);
char* strip_whitespaces(char* s);

int hexdigit(char c);
int hex2int(const char* sz, int n, int* sum);
int str2num(const char* str, int* v);

program_file* create_program_file(chip* dev, int argc, const char** argv);

#endif
