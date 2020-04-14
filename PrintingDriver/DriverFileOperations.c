#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <asm/atomic.h>
#include <linux/rwsem.h>
#include <asm/uaccess.h>
#include <asm/desc.h>
#include <asm/page.h>
#include <asm/mtrr.h>

#include "DriverFileOperations.h"
#include "DriverMain.h"
#include "ToolFunctions.h"
#include "IoCtlSupport.h"

int DriverOpen(struct inode *pslINode, struct file *pslFileStruct)
{
	DEBUG_PRINT(DEVICE_NAME ": open invoked, do nothing\n");
	return 0;
}

int DriverClose(struct inode *pslINode, struct file *pslFileStruct)
{
	DEBUG_PRINT(DEVICE_NAME ": close invoked, do nothing\n");
	return 0;
}

ssize_t DriverRead(struct file *pslFileStruct, char __user *pBuffer, size_t nCount, loff_t *pOffset)
{
	DEBUG_PRINT(DEVICE_NAME ": read invoked, do nothing\n");
	return 0;
}

/*--------------------用于寻找物理地址---------------------*/
int SetPageReadAndWriteAttribute(unsigned long ulAddress)
{
	unsigned long ulCR3;
	unsigned long ulPML4TPhysAddr;
	unsigned long ulPDPTPhysAddr;
	unsigned long ulPDTPhysAddr;
	unsigned long ulPTPhysAddr;

	unsigned long ulPPhyAddr; //对应的物理地址

	unsigned long ulPML4TIndex;
	unsigned long ulPDPTIndex;
	unsigned long ulPDTIndex;
	unsigned long ulPTIndex;

	unsigned long offset; //ulAddress线性地址最后12位
	unsigned long offset2; //ulAddress线性地址最后21位

	unsigned long ulPML4TE;
	unsigned long ulPDPTE;
	unsigned long ulPDTE;
	unsigned long ulPTE;
	unsigned long ulPTEAddr; //有点多余？？

	unsigned long ulFinalPhy; //ulAddress最终的物理地址
	unsigned long flag; //判断页面的大小

	ulPML4TIndex = (ulAddress & 0xff8000000000UL) >> 39;
	ulPDPTIndex = (ulAddress & 0x7fc0000000UL) >> 30;
	ulPDTIndex = (ulAddress & 0x3fe00000UL) >> 21;
	ulPTIndex = (ulAddress & 0x1ff000UL) >> 12;

	offset = (ulAddress & 0xfffUL);
	offset2 = (ulAddress & 0x1fffff);

	asm volatile("mov %%cr3, %0\n\t" : "=r" (ulCR3), "=m" (__force_order));

	ulPML4TPhysAddr = ulCR3 & 0xfffffffffffff000UL;
	ulPML4TE = *((unsigned long *)(__va(ulPML4TPhysAddr + ulPML4TIndex * 8)));

	ulPDPTPhysAddr = ulPML4TE & 0x7ffffffffffff000UL;//the right most bit is XD(eXecution Disable)
	ulPDPTE = *((unsigned long *)(__va(ulPDPTPhysAddr + ulPDPTIndex * 8)));

	ulPDTPhysAddr = ulPDPTE & 0x7ffffffffffff000UL;
	ulPDTE = *((unsigned long *)(__va(ulPDTPhysAddr + ulPDTIndex * 8))); //判断是页表的大小
    
	flag = ulPDTE & 0x40;

	if (flag) { //如果4k页面
		DEBUG_PRINT(DEVICE_NAME "Page is 4k\n");
		ulPTPhysAddr = ulPDTE & 0x7ffffffffffff000UL;
		ulPTEAddr = (unsigned long)(__va(ulPTPhysAddr + ulPTIndex * 8));
		ulPTE = *((unsigned long *)(ulPTEAddr));

		ulPPhyAddr = ulPTE & 0x7ffffffffffff000UL;
		ulFinalPhy = ulPPhyAddr + offset;
	}
	else { //2M页面
		DEBUG_PRINT(DEVICE_NAME "Page is 2M\n");
		ulPPhyAddr = ulPDTE & 0x7fffffffffe00000UL;
		ulFinalPhy = ulPPhyAddr + offset2;
	}

	DEBUG_PRINT(DEVICE_NAME "Finall phy addr is: %lx\n", ulFinalPhy);

	return 0;
}
/*-----------------------------------------*/

ssize_t DriverWrite(struct file *pslFileStruct, const char __user *pBuffer, size_t nCount, loff_t *pOffset)
{	
	DEBUG_PRINT(DEVICE_NAME ": write invoked, do nothing\n");

	char gdtr[10] = {0};
	asm volatile ("sgdt %0\n\t" : "=m"(gdtr) : :);

	DEBUG_PRINT("gdtr is: \n");
	MEM_PRINT((unsigned long)gdtr, 10);

	unsigned int gs_base_high, gs_base_low;
	asm volatile ("mov $0xc0000101, %%ecx \n\t"
		"rdmsr \n\t" : "=d"(gs_base_high), "=a"(gs_base_low) : : "%ecx", "memory");

	unsigned long gs_base = ((unsigned long)gs_base_high << 32) + gs_base_low;
	DEBUG_PRINT(DEVICE_NAME " gs_base = 0x%lx\n", gs_base);

	unsigned long gdt_offset = (unsigned long)&gdt_page;
	DEBUG_PRINT(DEVICE_NAME " gdt_offset = 0x%lx\n", gdt_offset);
	
	unsigned long gdt_addr = gs_base + gdt_offset;
	DEBUG_PRINT(DEVICE_NAME " gdt_addr = 0x%lx\n", gdt_addr);
	
	return 0;
}

long DriverIOControl(struct file *pslFileStruct, unsigned int uiCmd, unsigned long ulArg)
{
	DEBUG_PRINT(DEVICE_NAME ": ioctl invoked, do nothing\n");
	return 0;
}

int DriverMMap(struct file *pslFileStruct, struct vm_area_struct *pslVirtualMemoryArea)
{
	DEBUG_PRINT(DEVICE_NAME ": mmap invoked, do nothing\n");
	return 0;
}
