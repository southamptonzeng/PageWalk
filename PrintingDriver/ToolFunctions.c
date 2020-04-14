#include <asm/page.h>
#include <asm/special_insns.h>
#include "ToolFunctions.h"

#define LINE_BUFFER_LENGTH 64

void MEM_PRINT(unsigned long addr, unsigned long size)
{
	unsigned long i, j, k;
	char buf[LINE_BUFFER_LENGTH];
	char *q;
	unsigned char *p = (unsigned char *)addr;

	for (i = 0; i < size;)
	{
	    	q = buf;
		k = snprintf(q, LINE_BUFFER_LENGTH, "%lx:\t", (unsigned long)p);
		q = q + k;

		for (j = 0; j < 8; j++)
		{
			if (i + j >= size)
				break;

			if (*(p + j) <= 0xf)
			    	k = snprintf(q, LINE_BUFFER_LENGTH - (q - buf), "0%X ", *(p + j));
			else
				k = snprintf(q, LINE_BUFFER_LENGTH - (q - buf), "%X ", *(p + j));

			q = q + k;
		}

		printk("%s", buf);

		i = i + 8;
		p = p + 8;
	}
}


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
		//DEBUG_PRINT(DEVICE_NAME "Page is 4k\n");
		ulPTPhysAddr = ulPDTE & 0x7ffffffffffff000UL;
		ulPTEAddr = (unsigned long)(__va(ulPTPhysAddr + ulPTIndex * 8));
		ulPTE = *((unsigned long *)(ulPTEAddr));

		ulPPhyAddr = ulPTE & 0x7ffffffffffff000UL;
		ulFinalPhy = ulPPhyAddr + offset;
	}
	else { //2M页面
		//DEBUG_PRINT(DEVICE_NAME "Page is 2M\n");
		ulPPhyAddr = ulPDTE & 0x7fffffffffe00000UL;
		ulFinalPhy = ulPPhyAddr + offset2;
	}

	//DEBUG_PRINT(DEVICE_NAME "Finall phy addr is: %lx\n", ulFinalPhy);

	return 0;
}