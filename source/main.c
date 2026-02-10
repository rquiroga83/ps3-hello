#include <stdio.h>
#include <sys/process.h>

SYS_PROCESS_PARAM(1001, 0x100000);

int main()
{
	printf("hello, ps3\n");
	return 0;
}
