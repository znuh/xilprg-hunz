#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <endian.h>
#include <string.h>
#include <stdio.h>
#include "xilprg.h"
#include "cable.h"
#include "chip.h"
#include "prgfile.h"
#include <assert.h>

#define SERV_PORT		18034

#define MAX_DATASIZE 	(1<<16)

#define CMD_USER	0x100
#define ERROR	0x80000000

struct xilusr00_cmd_hdr {
	uint32_t cmd;
//	uint32_t bitstream_id;
//	uint32_t reply_len;
//	uint32_t reply_skip;
	uint32_t data_len;
} __attribute__((packed));

struct xilusr00_reply_hdr {
	uint32_t result;
//	uint32_t bitstream_id;
	uint32_t reply_len;
} __attribute__((packed));

int start_server(int index) {
	struct sockaddr_in local_sin;
	uint8_t *ibuf=(uint8_t *)malloc(MAX_DATASIZE), *obuf=(uint8_t *)malloc(MAX_DATASIZE);
	int bind_sock, rc=0;
	
	// one nice day here will be a pthread!
	
	assert(ibuf);
	assert(obuf);
	
	bind_sock = socket(AF_INET, SOCK_STREAM, 0);
	assert(bind_sock >= 0);
	
	bzero(&local_sin, sizeof(local_sin));
	local_sin.sin_port = htons(SERV_PORT);
	
	printf("bind\n");
	assert(bind(bind_sock, (struct sockaddr *)&local_sin, sizeof(local_sin)) >= 0);
	
	printf("listen\n");
	assert(!(listen(bind_sock,0)));
	
	while(1) {
		struct sockaddr_in remote_sin;
		int remote_sock, remote_sin_len=sizeof(remote_sin);
		struct xilusr00_cmd_hdr xilusr00_cmd_hdr;
		struct xilusr00_reply_hdr xilusr00_reply_hdr;
		char version_string[8];
		cable *cbl;
		chip *dev;
		
		bzero(&xilusr00_cmd_hdr, sizeof(xilusr00_cmd_hdr));
		bzero(&xilusr00_reply_hdr, sizeof(xilusr00_reply_hdr));
		
		printf("accept\n");
		remote_sock = accept(bind_sock, (struct sockaddr *)&remote_sin, (socklen_t *)&remote_sin_len);
		assert(remote_sock >= 0);
		
		// handle client
		
		// version check
		printf("version\n");
		assert(read(remote_sock,version_string,8) == 8);
		assert(!memcmp(version_string,"XILUSR00",8));
		
		// open cable
		cbl = open_cable(1);
		assert(cbl);
		
		// select device
		dev = select_device_in_chain(index - 1);
		assert(dev);
		
		printf("cmd mode\n");
		while(read(remote_sock,&xilusr00_cmd_hdr, sizeof(xilusr00_cmd_hdr)) == sizeof(xilusr00_cmd_hdr)) {
			int res, user, len;
			
			// screw network order, we use le for the transfers
			xilusr00_cmd_hdr.cmd = le32toh(xilusr00_cmd_hdr.cmd);
			len = xilusr00_cmd_hdr.data_len = le32toh(xilusr00_cmd_hdr.data_len);
			
			// read command
			assert(xilusr00_cmd_hdr.cmd & CMD_USER);
			
			// user 1..4
			user = xilusr00_cmd_hdr.cmd & 0xff;
			assert(user>=1);
			assert(user<=4);
			
			// check len
			assert(len <= MAX_DATASIZE);
			
			// read data
			if (read(remote_sock,ibuf,len) != len)
				break;
			
			// invoke user
			res = xc_user(dev, cbl, user, ibuf, obuf, len*8); //TODO: len in bits!?
			
			// prepare result
			xilusr00_reply_hdr.result = xilusr00_cmd_hdr.cmd;
			if(res<0)
				xilusr00_reply_hdr.result |= ERROR;
			
			// to le
			xilusr00_reply_hdr.result = htole32(xilusr00_reply_hdr.result);
			xilusr00_reply_hdr.reply_len = htole32(len);
			
			// send result
			if(write(remote_sock,&xilusr00_reply_hdr,sizeof(xilusr00_reply_hdr)) != sizeof(xilusr00_reply_hdr))
				break;
			
			// send data
			if(write(remote_sock,obuf,len) != len)
				break;
		}
		
		printf("close\n");
		close(remote_sock);
		
		close_cable(cbl);
	}
	
	free(ibuf);
	free(obuf);
	
	return rc;
}