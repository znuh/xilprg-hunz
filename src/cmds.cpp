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
#include "cmdline.h"
#include "cmds.h"
#include "parport.h"
#include "prgfile.h"
#include "utils.h"
#include "chip.h"
#include "server.h"

#include <assert.h>

const char* current_command_line;

int cmd_detect(int, const char**)
{
    cable* cbl;
    
    cbl = open_cable(1);
    if (cbl == NULL)
        return 0;

    close_cable(cbl);

    return 0;
}

int cmd_read(int argc, const char** argv)
{
	u8* data = NULL;
	cable* cbl;
	const char* param;
	chip* dev;
	int index, length, spi=0;
	program_file* f = NULL;

	// Position
	param = cmdline_get_non_opt(argc, argv, 0);
	if (param == NULL || str2num(param, &index) || index <= 0)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }

	// File name
	param = cmdline_get_non_opt(argc, argv, 1);
	if (param == NULL)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }

    cbl = open_cable(1);
    if (cbl == NULL)
        goto cleanup;
	
    dev = select_device_in_chain(index - 1);
    if (dev == NULL)
        goto cleanup;

	// spi flash read?
     if (cmdline_has_opt(argc, argv, "spi"))
	 spi=1;    
    
    msgf(STR_READING, index, dev->name.c_str());
	length = dev->readback(cbl, &data, spi);
	if (length <= 0 || data == NULL)
		goto cleanup;

    f = create_program_file(dev, argc, argv);
	if (f == NULL)
		goto cleanup;

	if (f->save(param, data, length))
		goto cleanup;

	msgf(STR_SUCCESS_COMPLETED);
    
cleanup:

	close_cable(cbl);

	if (data)
        free(data);

    if (f)
        delete f;
    
    return 0;
}

int cmd_program(int argc, const char** argv)
{
	cable* cbl = NULL;
	int index, low_index, hi_index, spi=0;
	program_file* f = NULL;
	const char* param;
	chip* dev;

	// Position
	param = cmdline_get_non_opt(argc, argv, 0);
	if(param == NULL) {
		msgf(STR_INVALID_PARAMETERS);
		return 0;
	}
	else if(strchr(param,'*')) {
		low_index = 1;
		hi_index = 0xffff;
	}
	else {
		if(str2num(param, &index) || index <= 0) {
			msgf(STR_INVALID_PARAMETERS);
			return 0;
		}
		low_index = index;
		hi_index = index;
	}

	// File name
	param = cmdline_get_non_opt(argc, argv, 1);
	if (param == NULL)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
	}

	// Open programmer cable with auto-detect
	cbl = open_cable(1);
	if (cbl == NULL)
		goto cleanup;

	f = create_program_file(dev, argc, argv);
	if (f == NULL)
		goto cleanup;
    
	// Load file
	if (f->load(param))
		goto cleanup;

    
	for(index=low_index; index <= hi_index; index++) {
		// Select device
		dev = select_device_in_chain(index - 1);
		if (dev == NULL)
			goto cleanup;
	
		// Erase first
		if (cmdline_has_opt(argc, argv, "e"))
		{
			msgf(STR_ERASING, index, dev->name.c_str());
			if (dev->erase(cbl))
				goto cleanup;
		}
    	
		// spi flash write?
		if (cmdline_has_opt(argc, argv, "spi"))
			spi=1;
	
		msgf(STR_PROGRAMMING, index, dev->name.c_str());
	
		if (dev->program(cbl, f,spi) == 0)
			msgf(STR_SUCCESS_COMPLETED);
		
	}
	
cleanup:

	close_cable(cbl);

	if (f)
	{
		f->clear();
		delete f;
	}

    return 0;
}

int cmd_erase(int argc, const char** argv)
{
	cable* cbl = NULL;
	int index;
	chip* dev;
	const char* param;

	// Position
	param = cmdline_get_non_opt(argc, argv, 0);
	if (param == NULL || str2num(param, &index) || index <= 0)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }
    
    cbl = open_cable(1);
    if (cbl == NULL)
        return 0;
	
    dev = select_device_in_chain(index - 1);
    if (dev == NULL)
        return -1;

	msgf(STR_ERASING, index, dev->name.c_str());
	
	if (dev->erase(cbl) == 0)
		msgf(STR_SUCCESS_COMPLETED);

	close_cable(cbl);

    return 0;
}

int cmd_verify(int argc, const char** argv)
{
	cable* cbl = NULL;
	int index;
    program_file* f = NULL;
	const char* param;
	chip* dev;
	int length, spi=0;
	u8* data = NULL;

	// Position
	param = cmdline_get_non_opt(argc, argv, 0);
	if (param == NULL || str2num(param, &index) || index <= 0)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }

	// File name
	param = cmdline_get_non_opt(argc, argv, 1);
	if (param == NULL)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }

	// Open programmer cable with auto-detect
    cbl = open_cable(1);
    if (cbl == NULL)
        goto cleanup;

	// Select device
	dev = select_device_in_chain(index - 1);
    if (dev == NULL)
        goto cleanup;
	    
	f = create_program_file(dev, argc, argv);
	if (f == NULL)
		goto cleanup;
    
	// Load file
	if (f->load(param))
		goto cleanup;

	// spi flash?
	if (cmdline_has_opt(argc, argv, "spi"))
		spi=1;	
	
	msgf(STR_VERIFYING, index, dev->name.c_str());
	length = dev->readback(cbl, &data, spi);
	if (length <= 0 || data == NULL)
		goto cleanup;

	// Verify
	length = length < f->get_length() ? length : f->get_length();
	if (memcmp(data, f->get_stream(), length) == 0)
		msgf(STR_SUCCESS_COMPLETED);
	else
		msgf(STR_FAILED);
	
cleanup:

	close_cable(cbl);

	if (f)
	{
		f->clear();
		delete f;
	}

	if (data)
		free(data);

    return 0;
}

int cmd_idcode(int argc, const char** argv)
{
	cable* cbl;
    chip* dev;
	int index;
	const char* param;

	// Position
	param = cmdline_get_non_opt(argc, argv, 0);
	if (param == NULL || str2num(param, &index) || index <= 0)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }

	cbl = open_cable(1);
    if (cbl == NULL)
        return 0;

    dev = select_device_in_chain(index - 1);
    if (dev)
        msgf(STR_DEVICE_IDCODE, index, dev->id);
	    
	close_cable(cbl);
	
    return 0;
}

int cmd_server(int argc, const char **argv) {
	const char* param;
	int index;

	// Position
	param = cmdline_get_non_opt(argc, argv, 0);
	if (param == NULL || str2num(param, &index) || index <= 0)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
	}
	
	return start_server(index);
}

/* this will be gone again soon I hope! */
int cmd_la(int argc, const char **argv) {
	cable* cbl;
	chip* dev;
	int index, user,cnt,val;
	uint8_t ival[32],oval[32];
	const char* param;
	FILE *la_out;

	// Position
	param = cmdline_get_non_opt(argc, argv, 0);
	if (param == NULL || str2num(param, &index) || index <= 0)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }

	cbl = open_cable(1);
    if (cbl == NULL)
        return 0;

    dev = select_device_in_chain(index - 1);
    if (!dev)
	goto cleanup;
        
    // USER1..4
    param = cmdline_get_non_opt(argc, argv, 1);
    if(param == NULL) {
	    msgf(STR_INVALID_PARAMETERS);
	    goto cleanup;
    }
	if (param == NULL || str2num(param, &user) || (user <= 0) || (user > 4))
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }

    // file
    param = cmdline_get_non_opt(argc, argv, 2);
    if(param == NULL) {
	    msgf(STR_INVALID_PARAMETERS);
	    goto cleanup;
    }
    
    assert((la_out = fopen(param,"w")));
    
    cnt=0;
    
    ival[0]=0;
    ival[1]=0;
    ival[2]=0;
    ival[3]=0;
    ival[4]=reverse8(0x7f);
    
    while(1) {
	    int i, j;
	    
	    assert(!(dev->user(cbl, user, ival, oval, 40)));
	    
	    for(i=0;i<5;i++)
		oval[i]=reverse8(oval[i]);
	    	    
	    // no new data left
	    if(!(oval[0]&0x80))
		    break;
	    
	    fprintf(la_out,"%6d",cnt++);
	    
	    for(i=1;i<5;i++) {
		for(j=7;j>=0;j--)
			fprintf(la_out,",%d",(oval[i]>>j)&1);
	    }
	    
	    fprintf(la_out,"\n");
    }
    
    fclose(la_out);
    
    printf("%d samples read\n",cnt);

cleanup:    
	close_cable(cbl);
		
   return 0;
}
	
int cmd_user(int argc, const char **argv){
	cable* cbl;
    chip* dev;
	int index, user,cnt,val;
	uint8_t ival[32],oval[32];
	const char* param;

	// Position
	param = cmdline_get_non_opt(argc, argv, 0);
	if (param == NULL || str2num(param, &index) || index <= 0)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }

	cbl = open_cable(1);
    if (cbl == NULL)
        return 0;

    dev = select_device_in_chain(index - 1);
    if (!dev)
	goto cleanup;
        
    // USER1..4
    param = cmdline_get_non_opt(argc, argv, 1);
    if(param == NULL) {
	    msgf(STR_INVALID_PARAMETERS);
	    goto cleanup;
    }
	if (param == NULL || str2num(param, &user) || (user <= 0) || (user > 4))
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }
    
    // DR Value - 8 Bit only so far - TODO
    param = cmdline_get_non_opt(argc, argv, 2);
    if(param == NULL) {
	    msgf(STR_INVALID_PARAMETERS);
	    goto cleanup;
    }
    
    printf("USER%d: ",user);
    
    cnt=0;
    
    while((param[0]) && (sscanf(param,"%02x",&val))) {
	    param+=2;
	    ival[cnt++]=reverse8(val);
	    printf("%02x",val);
	}
	printf(" -> ");

    //ival=(strtoul(param,NULL,0))&0xff;

    if(!( dev->user(cbl, user,ival,oval,cnt*8))) {
	for(val=0;val<cnt;val++)
		 printf("%02x",reverse8(oval[val]));
    }
    printf("\n");
	//printf("USER%d: %x %x\n",user,oval[0],oval[1]);
    
cleanup:    
	close_cable(cbl);
		
   return 0;
}

unsigned long getbytes(char *buf, int num) {
    char bbuf[10];
    int count;
    
    for(count=0;count<(num<<1);count++)
	bbuf[count]=buf[count];
    bbuf[count]=0;
    
    return strtoul(bbuf, NULL, 16);
}

int cmd_write(int argc, const char **argv){
	cable* cbl;
    chip* dev;
	int index, user,cnt;
	uint8_t ival[32],oval[32];
	uint8_t len, type;
	uint16_t addr, data, laddr=0;
	const char* param;
	char buf[128], *ptr;
	FILE *fl;

	// Position
	param = cmdline_get_non_opt(argc, argv, 0);
	if (param == NULL || str2num(param, &index) || index <= 0)
	{
        msgf(STR_INVALID_PARAMETERS);
		return 0;
    }

	cbl = open_cable(1);
    if (cbl == NULL)
        return 0;

    dev = select_device_in_chain(index - 1);
    if (!dev)
	goto cleanup;
  
    // open file
    param = cmdline_get_non_opt(argc, argv, 1);
    if(param == NULL) {
	    msgf(STR_INVALID_PARAMETERS);
	    goto cleanup;
    }
    
    fl=fopen(param,"r");
    if(fl == NULL) {
	    msgf(STR_INVALID_PARAMETERS);
	    goto cleanup;
    }
    
    while(fgets(buf,128,fl)) {
	    buf[127]=0;
	    ptr=buf+1;

	if(buf[0] != ':') {
		printf("not a hex file?!? no '.' at pos 0\n");
		goto cleanup;
	}
	
	len=getbytes(ptr, 1);
	ptr+=2;
    
	addr=getbytes(ptr, 2);
	ptr+=4;
	
	type=getbytes(ptr, 1);
	ptr+=2;
	 
	if(type)
		continue;
	
	printf("addr %4x len %2d\n",addr,len);
	
	//printf("#");
	while(len) {
		data=getbytes(ptr,2);
		ptr+=4;
		
		ival[0]=0x01;
		ival[1]=(addr/2)>>8;
		ival[2]=(addr/2)&0xff;
		ival[4]=data>>8;
		ival[3]=data&0xff;
		
		for(cnt=0;cnt<5;cnt++)
			ival[cnt]=reverse8(ival[cnt]);
		
		dev->user(cbl, 1,ival,oval,5*8);
		
		//for(cnt=0;cnt<5;cnt++)
		//	printf("%02x ",reverse8(oval[cnt]));
		//printf("\n");
		laddr = addr;
		//printf("laddr %x\n",laddr);
		addr+=2;
		len-=2;
	}

    }
    fclose(fl);
    
    ival[0]=reverse8(0x02);
    printf("last addr written - byte: 0x%x (%d) - word: 0x%x (%d)\n",laddr,laddr,laddr/2,laddr/2);
    printf("reset: %d\n",dev->user(cbl, 1,ival,oval,5*8));
    
cleanup:    
	close_cable(cbl);
		
   return 0;
}

int cmd_chips(int argc, const char** argv)
{
    chip_database::iterator iter;
    chip_family* family;
    chip_family::iterator iter2;
    string desc;
    int all;
    char support[16];

    all = cmdline_has_opt(argc, argv, "all");

    printf("%s:\n\n", res_str(STR_SUPPORTED_FAMILIES));
        
    for (iter = g.chips.begin(); iter != g.chips.end(); iter++)
    {
        family = *iter;

        // Family description
        family->vars.get(strDESC, desc);

        // Supported operations
        strcpy(support, "[D   ]");
        if (family->vars.exists(strERASE) == 0)
            support[2] = 'E';
        if (family->vars.exists(strPROGRAM) == 0)
            support[3] = 'P';
        if (family->vars.exists(strREADBACK) == 0)
            support[4] = 'R';
                    
        printf("%-40s %s\n", desc.c_str(), support);

        // Show devices in family
        if (all)
            for (iter2 = family->begin(); iter2 != family->end(); iter2++)
                printf("    %s\n", (*iter2).name.c_str());
    }
    
    return 0;
}

int cmd_cable(int argc, const char** argv)
{
    cable* cbl;
	int i;
    string desc, cable_def;
    
    if (argc > 1)
	{
		for (i = 1; i < argc; i++)
        {
            cable_def += argv[i];
            if (i + 1 < argc)
                cable_def += ' ';
        }

		g.vars[strCABLE] = cable_def;
	}
	else
	{
		if (g.vars.get(strCABLE, cable_def))
		{
            msgf(STR_INVALID_CABLE_DEF);
			return -1;
		}
	}

    cbl = cable::factory(cable_def.c_str());
    if (cbl == NULL)
        return 0;

    cbl->get_description(desc);
    msgf(STR_SELECTED_CABLE, desc.c_str());

    return 0;
}

int cmd_exit(int, const char**)
{
    return CMDLINE_EXIT_PROGRAM;
}

int cmd_cd(int argc, const char** argv)
{
    char dir[256] = "";

    if (argc == 2)
    {
        if (set_current_directory(argv[1]))
            msgf(STR_CHDIR_ERROR);
    }
    else
    {
        if (get_current_directory(dir, sizeof(dir)) == 0)
            printf("%s\n", dir);
    }
    
    return 0;
}

int cmd_version(int, const char**)
{
    msgf(STR_LOGO);
    return 0;
}

int help(const char* cmd)
{
    int i;
    const char* str_cmd = NULL;
    const char* str_help = NULL;
    const char* str_usage = NULL;

    if (cmd == NULL)
    {
        printf("%s:\n\n", res_str(STR_HELP));
        for (i = 0; commands[i].fn != NULL; i++)
        {
            str_cmd = res_str(commands[i].res_id);
            str_help = str_cmd + strlen(str_cmd) + 1;

            printf("%-20s %s\n", str_cmd, str_help);
        }
        printf("\n");
    }
    else
    {
        for (i = 0; commands[i].fn != NULL; i++)
        {
            str_cmd = res_str(commands[i].res_id);
            if (strcmp(str_cmd, cmd) == 0)
            {
                str_help = str_cmd + strlen(str_cmd) + 1;
                str_usage = str_help + strlen(str_help) + 1;
                break;
            }
        }

        if (commands[i].fn != NULL)
			printf("%s\n\n%s\n", str_help, str_usage);
        else
            msgf(STR_UNKNOWN_COMMAND);
    }
    return 0;
}

int cmd_help(int argc, const char** argv)
{
    help(argc <= 1 ? NULL : argv[1]);
    return 0;
}

int process_command_line(const char* line)
{
    int argc = 0;
    char** argv = NULL;
    int i, rc = 0;
    const char* str_cmd;

    if (line == NULL)
        return 0;

    if (line[0] == '.')
    {
        system(line + 1);
        return 0;
    }

	current_command_line = line;

    if (parse_cmdline_args(line, &argc, &argv) < 0)
	{
		rc = -1;
		goto cleanup;
	}

    if (argc == 0)
        goto cleanup;

    for (i = 0; commands[i].fn != NULL; i++)
    {
        str_cmd = res_str(commands[i].res_id);
        if (strcmp(str_cmd, argv[0]) == 0)
        {
            rc = commands[i].fn(argc, (const char**)argv);
            break;
        }
    }
    
    if (commands[i].fn == NULL)
    {
        msgf(STR_UNKNOWN_COMMAND);
        rc = 0;
    }
    else
    {
        switch (rc)
        {
        case CMDLINE_INVALID_PARAMETER:
            msgf(STR_INVALID_PARAMETERS);
			help(argv[0]);
            break;
        }
    }

cleanup:

    if (argv != NULL)
        free(argv);

	current_command_line = NULL;

    return rc;
}

command_t commands[] = 
{
    {STR_CMD_DETECT, cmd_detect},
    {STR_CMD_CABLE, cmd_cable},
    {STR_CMD_CHIPS, cmd_chips},
    {STR_CMD_PROGRAM, cmd_program},
    {STR_CMD_ERASE, cmd_erase},
    {STR_CMD_READ, cmd_read},
	{STR_CMD_VERIFY, cmd_verify},
    {STR_CMD_IDCODE, cmd_idcode},
    {STR_CMD_CD, cmd_cd},
    {STR_CMD_VERSION, cmd_version},
    {STR_CMD_EXIT, cmd_exit},
    {STR_CMD_HELP, cmd_help},
    {STR_CMD_USER, cmd_user},
    {STR_CMD_SERVER, cmd_server},
    {STR_CMD_WRITE, cmd_write},
    {STR_CMD_LA, cmd_la},
    {(unsigned int)-1, NULL}
};


