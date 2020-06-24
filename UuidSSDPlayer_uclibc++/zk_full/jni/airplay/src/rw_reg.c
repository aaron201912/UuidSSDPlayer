#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include "airplayer.h"


#define DBG_INFO printf
#define DBG_ERR printf


#define BIT8        0x100

#define BANK_TO_ADDR32(b) (b<<9)
#define REG_ADDR(riu_base,bank,reg_offset) ((riu_base)+BANK_TO_ADDR32(bank)+(reg_offset*4))


#define  riu_w_colse 		0xC409
#define  riu_w_open			0xC408

typedef struct
{
	unsigned char *virt_addr;
	unsigned char *mmap_base;
	unsigned int mmap_length;
}MmapHandle;

static unsigned int const page_size_mask = 0xFFF;

MmapHandle* devMemMMap(unsigned int phys_addr, unsigned int length)
{
	int fd;
	unsigned int phys_offset;

	fd = open("/dev/mem", O_RDWR|O_SYNC);
	if (fd == -1)
	{
		DBG_ERR("open /dev/mem fail\n");
		return NULL;
	}

	MmapHandle *handle = (MmapHandle*)malloc(sizeof(MmapHandle));
	phys_offset =(phys_addr & (page_size_mask));
	phys_addr &= ~(page_size_mask);
	handle->mmap_length = length + phys_offset;
	handle->mmap_base = (unsigned char*)mmap(NULL, handle->mmap_length , PROT_READ|PROT_WRITE, MAP_SHARED, fd, phys_addr);
	handle->virt_addr = handle->mmap_base + phys_offset;
	DBG_INFO("phys_addr: %#x\n", phys_addr);
	DBG_INFO("virt_addr: %p\n", handle->virt_addr);
	DBG_INFO("phys_offset: %#x\n", phys_offset);

	if (handle->mmap_base == MAP_FAILED)
	{
		DBG_ERR("mmap fail\n");
		close(fd);
		free(handle);
		return NULL;
	}

	close(fd);
	return handle;
}

static int devMemUmap(MmapHandle* handle)
{
	int ret = 0;

	ret = munmap(handle->mmap_base, handle->mmap_length);
	if(ret != 0)
	{
		printf("munmap fail\n");
		return ret;
	}
	free(handle);
	return ret;
}

static int riu_base(int base)
{
	unsigned long long uuid;
    /* RIU mapping*/
    MmapHandle *riu_base = devMemMMap(0x1F000000, 0x2B0000);

    /*Configure PAD and Clock here*/

    //chg default dev2,3 pclk from d4 to gpio2
    *(unsigned short*)REG_ADDR(riu_base->virt_addr, 0x1234, 0x0) = base;

    devMemUmap(riu_base);
	printf("uuid: %llx\n",uuid);

	return 0;
}

void Ss_UI_Close(void)
{
	riu_base(riu_w_colse);
}

void Ss_UI_Open(void)
{
	riu_base(riu_w_open);
}

