#include <stdio.h>

#ifndef LEN
#define LEN 125
#endif

const int key[] = { 77, 4, 73, 95, 71, 76, 11, 3 };

int main(void) {
	int a[LEN];
	FILE *fd;
	int i;
	
	if (!(fd = fopen("im_crackme_1.exe", "rb"))) {
		puts("error opening file");
		return 1;
	}
	
	fseek(fd, 0x44f, SEEK_SET);
	
	for (i = 0; i < 20; ++i) {
		fread(a + i, 4, 1, fd);
		fseek(fd, 4, SEEK_CUR);
	}
	
	fseek(fd, 3, SEEK_CUR);
	
	for (i = 20; i < LEN; ++i) {
		fread(a + i, 4, 1, fd);
		fseek(fd, 7, SEEK_CUR);
	}
	
	fclose(fd);
	
	i = 0;
	
	while (i < LEN) {
		printf("0x%0x:", i);
		switch (a[i]) {
			case 1:
				printf("\tMOV r%d,0x%0x\n", a[i+1], key[a[i+2]]);
				i += 3;
				break;
			case 2:
				printf("\tMOV r%d,password[%d]\n", a[i+1], a[i+2]);
				i += 3;
				break;
			case 3:
				printf("\tXOR r%d,r%d\n", a[i+1], a[i+2]);
				i += 3;
				break;
			case 4:
				printf("\tCMP r%d,r%d\n", a[i+1], a[i+2]);
				i += 3;
				break;
			case 5:
				printf("\tJZ 0x%0x\n", i + 2 + a[i+1]);
				i += 2;
				break;
			case 6:
				printf("\tJNZ 0x%0x\n", i + 2 + a[i+1]);
				i += 2;
				break;
			case 7:
				puts("\tcongrat!");
				++i;
				break;
			case 8:
				puts("\ttry again..");
				++i;
				break;
			case 9:
				printf("\tCMP password.length(), 0x%0x\n", a[i+1]);
				i += 2;
				break;
		}
	}
	
	putchar(0xa);
	
	return 0;
}
