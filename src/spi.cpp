#include "xilprg.h"
#include "cable.h"
#include "chip.h"
#include "prgfile.h"
#include <assert.h>

void hexdump(uint8_t *d, int len) {
	while(len--) {
		printf("%02x ",*d);
		d++;
	}
	printf("\n");
}

int spi_xfer_user1(chip *dev, cable *cbl, uint8_t *last_miso, int miso_len, int miso_skip, uint8_t *mosi, int mosi_len) {
	uint8_t *mosi_buf=NULL;
	uint8_t *miso_buf=NULL;
	int cnt, rc, maxlen = miso_len+miso_skip;
	
	//TODO: assert maxlens
	
	if((mosi_len) && (mosi_len + 4 + 2 > maxlen))
		maxlen = mosi_len + 4 + 2;
		
	if(last_miso) {
		miso_buf=(uint8_t*)malloc(maxlen);
		assert(miso_buf);
	}

	if(mosi) {
		mosi_buf=(uint8_t*)malloc(maxlen);
		assert(mosi_buf);
	
		// SPI magic
		mosi_buf[0]=0x59;
		mosi_buf[1]=0xa6;
		mosi_buf[2]=0x59;
		mosi_buf[3]=0xa6;
	
		// SPI len (bits)
		mosi_buf[4]=(mosi_len*8)>>8;
		mosi_buf[5]=(mosi_len*8)&0xff;
	
		// bit-reverse header
		for(cnt=0;cnt<6;cnt++)
			mosi_buf[cnt]=reverse8(mosi_buf[cnt]);
	
		// bit-reverse payload
		for(cnt=0;cnt<mosi_len;cnt++)
			mosi_buf[cnt+4+2]=reverse8(mosi[cnt]);
	}
	
	
	rc=xc_user(dev,cbl,1,mosi_buf,miso_buf,maxlen*8);
	
	if(miso_buf) {
		//printf("miso "); hexdump(miso_buf,maxlen);	
		
		for(cnt=miso_skip; cnt<miso_len+miso_skip; cnt++)
			last_miso[cnt-miso_skip]=reverse8(miso_buf[cnt]);
		
		free(miso_buf);
	}
	
	if(mosi_buf) {
		//printf("mosi "); hexdump(mosi_buf,maxlen);
		free(mosi_buf);
	}
	//printf("-\n");
	return rc;
}

int spi_cfg[] = {
	// sreg[5..2], pagesize, pages
	3, 264, 512, // XC3S50AN
	7, 264, 2048, // XC3S200AN / XC3S400AN
	9, 264, 4096, // XC3S700AN
	11, 528, 4096, // XC3S1400AN
	-1, 0, 0
};

int spi_flashinfo(chip *dev, cable *prg, int *size, int *pages) {
	uint8_t buf[8];
	int idx;
	
	// send JEDEC info
	buf[0]=0x9f;
	spi_xfer_user1(dev,prg,NULL,0,0,buf,3);
	
	// get JEDEC, send status
	buf[4]=0xd7;
	spi_xfer_user1(dev,prg,buf,2,1,buf+4,2);
	
	printf("JEDEC: %02x %02x\n",buf[0],buf[1]);
	
	// tiny sanity check
	if(buf[0] != 0x1f) {
		printf("unknown JEDEC manufacturer: %02x\n",buf[0]);
		return -1;
	}
	
	// get status
	spi_xfer_user1(dev, prg, buf,1,1, NULL, 0);
	printf("status: %02x\n",buf[0]);
	
	for(idx=0;spi_cfg[idx] != -1;idx+=3) {
		if(spi_cfg[idx] == ((buf[0]>>2)&0x0f))
			break;
	}
	
	if(spi_cfg[idx] == -1) {
		printf("don't know that flash or status b0rken!\n");
		return -1;
	}
	
	printf("%d bytes/page, %d pages = %d bytes total \n",spi_cfg[idx+1],spi_cfg[idx+2],spi_cfg[idx+1]*spi_cfg[idx+2]);
	
	*size=spi_cfg[idx+1];
	*pages=spi_cfg[idx+2];
	
	return 0;
}

int spi_readback(chip *dev, cable *prg, u8 **data) {
	uint8_t *buf;
	int pgsize,pages,page,res,rc=0;
	
	res=spi_flashinfo(dev,prg,&pgsize,&pages);
	if(res) {
		*data=NULL;
		return -1;
	}
	
	*data=(u8*)malloc(pgsize*pages);
	
	buf=(uint8_t*)malloc(pgsize+16);
	buf[0]=0x03;
	buf[3]=buf[2]=buf[1]=0;
	
	// send: read 1st page
	res=spi_xfer_user1(dev,prg,NULL,0,0,buf,pgsize+4);
	//TODO: check res
	
	for(page=1;page<pages;page++) {
		uint16_t paddr=page<<1;
		int res;
		
		if(!(page&0x0f)) {
			printf("\rpage %d",page);
			fflush(stdout);
		}
		
		// see UG333 page 19
		if(pgsize>512)
			paddr<<=1;
		
		buf[1]=paddr>>8;
		buf[2]=paddr&0xff;
		
		// get: page n-1, send: read page n		
		res=spi_xfer_user1(dev,prg,(*data)+((page-1)*pgsize),pgsize,4,buf,pgsize+4);
		//TODO: check res
		
		rc+=pgsize;
	}
	
	// get last page
	res=spi_xfer_user1(dev,prg,(*data)+((page-1)*pgsize),pgsize,4,NULL,0);
	
//cleanup:
	free(buf);

	if (rc < 0)
		if (*data)
		{
			free(*data);
			*data = NULL;
		}
	
	printf("\r");
		
	return rc;
}

int spi_program(chip* dev, cable* prg, program_file* file)
{
    int rc,i,len = file->get_bit_length()/8;
    int pages, pgsize, page=0;
    uint8_t *buf,*data=file->get_stream();
    
    if (len == 0)
         return -1;

    if(spi_flashinfo(dev,prg,&pgsize,&pages))
	    return -1;
	
    // check of teh users sanity
    if(len>(pgsize*pages)) {
	    printf("dude, that file is larger than the flash!\n");
	    return -1;
    }
    
    buf=(uint8_t *)malloc(pgsize+8);
    assert(buf);
    
    buf[0]=0x82; // page program with builtin erase
    buf[3]=0;
    
    for(i = 0; i < len; i += pgsize)
    {
	uint16_t paddr = page<<1;
	int res;
	
	if(!(page&0x0f)) {
		printf("\rpage %d/%d",page,len/pgsize);
		fflush(stdout);
	}
	
	// see UG333 page 19
	if(pgsize>512)
		paddr<<=1;
		
	buf[1]=paddr>>8;
	buf[2]=paddr&0xff;
	
	memcpy(buf+4,data+i,((len-i)>pgsize) ? pgsize : (len-i));
	
	res=spi_xfer_user1(dev,prg,NULL,0,0,buf,pgsize+4);
	//TODO: check res
	
	usleep(6000); //t_p <= 6ms (UG333 page 44)	
	page++;
    } 

     rc = 0;

//cleanup:
	
     free(buf);

    printf("\r");
    
    return rc;
}
