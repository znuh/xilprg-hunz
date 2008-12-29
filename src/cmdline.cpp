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

#include "xilprg.h"
#include "utils.h"
#include "cmdline.h"

#define NULCHAR    '\0'
#define SPACECHAR  ' '
#define TABCHAR    '\t'
#define DQUOTECHAR '\"'
#define SLASHCHAR  '\\'

void parse_cmdline(
    char* cmdstart,
    char** argv,
    char* args,
    int* numargs,
    int* numchars
    )
{
    char *p;
    unsigned char c;
    int inquote;                    /* 1 = inside quotes */
    int copychar;                   /* 1 = copy char to *args */
    unsigned int numslash;              /* num of backslashes seen */

    *numchars = 0;
    *numargs = 1;                   /* the program name at least */

    /* first scan the program name, copy it, and count the bytes */
    p = cmdstart;
    if (argv)
        *argv++ = args;

    if ( *p == DQUOTECHAR ) 
    {
        while ( (*(++p) != DQUOTECHAR) && (*p != NULCHAR) ) 
        {
            ++*numchars;
            if ( args )
                *args++ = *p;
        }
        /* append the terminating null */
        ++*numchars;
        if ( args )
            *args++ = NULCHAR;

        /* if we stopped on a double-quote (usual case), skip over it */
        if ( *p == DQUOTECHAR )
            p++;
    }
    else {
        /* Not a quoted program name */
        do {
            ++*numchars;
            if (args)
                *args++ = *p;

            c = (unsigned char) *p++;
        } while ( c != SPACECHAR && c != NULCHAR && c != TABCHAR );

        if ( c == NULCHAR ) {
            p--;
        } else {
            if (args)
                *(args-1) = NULCHAR;
        }
    }

    inquote = 0;

    /* loop on each argument */
    for(;;) {

        if ( *p ) {
            while (*p == SPACECHAR || *p == TABCHAR)
                ++p;
        }

        if (*p == NULCHAR)
            break;              /* end of args */

        /* scan an argument */
        if (argv)
            *argv++ = args;     /* store ptr to arg */
        ++*numargs;

    /* loop through scanning one argument */
    for (;;) {
        copychar = 1;
        /* Rules: 2N backslashes + " ==> N backslashes and begin/end quote
           2N+1 backslashes + " ==> N backslashes + literal "
           N backslashes ==> N backslashes */
        numslash = 0;
        while (*p == SLASHCHAR) {
            /* count number of backslashes for use below */
            ++p;
            ++numslash;
        }
        if (*p == DQUOTECHAR) {
            /* if 2N backslashes before, start/end quote, otherwise
                copy literally */
            if (numslash % 2 == 0) {
                if (inquote) {
                    if (p[1] == DQUOTECHAR)
                        p++;    /* Double quote inside quoted string */
                    else        /* skip first quote char and copy second */
                        copychar = 0;
                } else
                    copychar = 0;       /* don't copy quote */

                inquote = !inquote;
            }
            numslash /= 2;          /* divide numslash by two */
        }

        /* copy slashes */
        while (numslash--) {
            if (args)
                *args++ = SLASHCHAR;
            ++*numchars;
        }

        /* if at end of arg, break loop */
        if (*p == NULCHAR || (!inquote && (*p == SPACECHAR || *p == TABCHAR)))
            break;

        /* copy character into argument */
        if (copychar) {
            if (args)
                *args++ = *p;
            ++*numchars;
        }
        ++p;
        }

        /* null-terminate the argument */

        if (args)
            *args++ = NULCHAR;          /* terminate string */
        ++*numchars;
    }

    /* We put one last argument in -- a null ptr */
    if (argv)
        *argv++ = NULL;
    ++*numargs;
}

int parse_cmdline_args(
    const char* line,
    int* argc,
    char*** argv
    )
{
    int numargs, numchars;
    char* p;
    
    if (line == NULL)
        return -1;

    while (*line != 0 && (*line == ' ' || *line == '\t'))
        line++;
    if (*line == 0)
        return -1;

    parse_cmdline((char*)line, NULL, NULL, &numargs, &numchars);

    p = (char*)malloc(numargs * sizeof(char*) + numchars * sizeof(char));
    if (p == NULL)
        return -1;

    parse_cmdline((char*)line, (char**)p, p + numargs * sizeof(char*), &numargs, &numchars);

    *argc = numargs - 1;
    *argv = (char**)p;

    return numargs - 1;
}

int cmdline_has_opt(int argc, const char** argv, const char* opt)
{
    int i;
    for (i = 1; i < argc; i++)
#ifdef WIN32
        if (argv[i][0] == '-' || argv[i][0] == '/')
#else
		if (argv[i][0] == '-')
#endif
            if (stricmp(argv[i] + 1, opt) == 0)
                return i;
    return 0;
}

const char* cmdline_get_non_opt(int argc, const char** argv, int index)
{
    int i;
    for (i = 1; i < argc; i++)
#ifdef WIN32
        if (argv[i][0] != '-' && argv[i][0] != '/')
#else
		if (argv[i][0] != '-')
#endif
        {
            if (index == 0)
                return argv[i];
            index--;
        }
    return NULL;
}
