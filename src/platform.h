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

#ifndef _PLATFORM_H_INCLUDED_
#define _PLATFORM_H_INCLUDED_

#if defined(WIN32) || defined(_WIN32) || defined(__CYGWIN__)
#include <windows.h>
#endif

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#if !defined(WIN32) || defined(__CYGWIN__)
#include <stdint.h>
#include <unistd.h>
#include <sys/io.h>
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
#pragma warning( disable: 4786 )
#pragma warning( disable: 4097 )
#pragma warning( disable: 4710 )
#pragma warning( push, 1 )
#pragma warning( disable: 4702 )
#endif
#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
#if defined(WIN32) && !defined(__CYGWIN__)
#pragma warning( pop )
#endif

using namespace std;

typedef char                    s8;
typedef unsigned char           u8;
typedef short                   s16;
typedef unsigned short          u16;
typedef int                     s32;
typedef unsigned int            u32;
#if defined(WIN32) && !defined(__CYGWIN__)
typedef __int64                 s64;
typedef unsigned __int64        u64;
#else
typedef long long               s64;
typedef unsigned long long      u64;
#endif

#ifdef WIN32
#define vsnprintf				_vsnprintf
#endif

#if defined(__GNUC__)
#define stricmp					strcasecmp
#define strnicmp				strncasecmp
#endif

#endif
