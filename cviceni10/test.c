#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

int main() {
	char* buf;
	int n;
        buf = (char*)malloc(20);
	strcpy(buf, "ahoj tu pisu");
	int fd = open("/dev/mpp", O_RDWR);
	//printf("%d\n", fd);
	write(fd, buf,strlen(buf)+1);
	read(fd,buf,strlen(buf)+1);
	printf("%s\n", buf);
	ioctl(fd, buf, 100, 2);
	printf("%s\n", buf);
	close(fd);
	return 0;
}
