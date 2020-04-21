#include "pmII.h"
#define DIAG_DISPLAY(d) STATUS_REG &= ~0xf;\
			STATUS_REG |= d;
main() {

	/* bang at the page and protectection maps */

	int i,c;
	long  addr,addrsave;
	char buf[80];
	unsigned short value,value1,savevalue;
	short protect;
	int readtoo;
	int allmap;
	while (1) {
	
		while (c != 'x' && c != 'p' && c != 'r') {
			printf("\nPage or pRotection maps, or eXit:");
			c = tolower(getchar());
		}
		
		if (c == 'x') return(0);
		protect = (c == 'r');

		printf("\nentire map?(y/n):(y)");
		c = tolower(getchar());
		allmap = (c != 'n');

		if (!allmap) {
			while (1) {
			printf("\nenter address:");
			getline(buf);
			addr = atox(buf);
			addr |= 0xfc0000;
			if ((int)addr < 0) printf("\nillegal hex value!");
			else if ((protect && ((int)addr > (int)PROTEND)) ||
					 (!protect && (((int)addr < (int)PAGEMAP) || 
								((int)addr >= (int)PROTMAP))))  {
				printf("\nillegal start address.");
				printf("\nmust be between 0x%x and 0x%x",
						protect?PROTMAP:PAGEMAP, protect?PROTEND:PROTMAP);
				}
			else break;
			}

			if ((addr & 0x1fff) < 4) {
				addr = (long)(protect?&PROTMAP[4]:&PAGEMAP[4]);
			}
			addr &= ~2;
			printf("\nstarting operation at address 0x%x\n",addr);
		}


		printf("\nverify by reading?(y/n):(y)");
		c = tolower(getchar());
		readtoo = (c != 'n');

		printf("\nenter the short value to write:");
		getline(buf);
		value = atox(buf);
		if (protect && (value > 0x3fff)) {
			value &= 0x3fff;
			printf("value used will be %4.4x\n",value );
		}
		else if (!protect && (value > 0xfff)) {
			value &= 0xfff;
			printf("value used will be %4.4x\n",value);
		}

		printf("\n\ntype any character to halt:");

		savevalue = value;
		flushinput();
		addrsave = 0;
		while ((c = nwgetchar()) < 0) {

			DIAG_DISPLAY(0);

			if (allmap) {

				if (protect) {
					value = savevalue;
					for (addr = (int)&PROTMAP[4]; 
						addr < (int)PROTEND; 
						addr += 2, value += 1) {

						if (value == 0x4000) value = 0;

						*(short *)addr = value;
					}
					value = savevalue;
					if (readtoo) {
						for (addr = (int)&PROTMAP[4]; 
							addr < (int)PROTEND; addr += 2, value += 1) 
							{
							if (value == 0x4000) value = 0;
							value1 = *(short *)addr;
							if (value != value1) {
								putchar('!');
								addrsave=addr;
								DIAG_DISPLAY(0xf);
							}
						}
					}

				}
				else {
					value = savevalue;
					for (addr = (int)&PAGEMAP[4]; 
						addr < (int)&PROTMAP[0]; 
						addr += 2, value += 1) {

						if (value == 0x1000) value = 0;
						*(short *)addr = value;
					}
					value = savevalue;
					if (readtoo) {
						for (addr = (int)&PAGEMAP[4]; 
							addr < (int)&PROTMAP[0]; addr += 2, value += 1) 
							{
							if (value == 0x1000) value = 0;
							value1 = *(short *)addr;
							if (value != value1) {
								putchar('!');
								addrsave=addr;
								DIAG_DISPLAY(0xf);
							}
						}
					}

				}
			}
			else {

				for (i=0; i< 10000; i++ ) {

					if (protect) {
						if (value == 0x4000) value = 0;
					} else if (value == 0x1000) value = 0;
					*(short *)addr = value;
					if (readtoo) {
						value1 = *(short *)addr;
						if (value != value1) {
							putchar('!');
							addrsave=addr;
							DIAG_DISPLAY(0xf);
						}
					}
				}
			}

		}

		if (addrsave) printf("\nlast error was at %x\n",addrsave);


	}

}
