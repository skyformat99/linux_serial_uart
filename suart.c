#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <string.h>

static volatile int keeprunning = 1;
#define SRLCLIENT "/dev/ttyUSB1"

int c_fd;
struct termios c_io_old;
void intHandler(int dummy) {
        keeprunning = 0;
	tcsetattr(c_fd, TCSADRAIN, &c_io_old);
	close(c_fd);
	exit(EXIT_SUCCESS);
	
}

void set_serial() {
	struct stat sb;
	struct termios c_io;	
	if (stat(SRLCLIENT, &sb) == -1) {
               perror("stat");
               exit(EXIT_FAILURE);
        }
	if (!S_ISCHR(sb.st_mode)) {
		printf("not a char file\n");
		exit(EXIT_FAILURE);
	}
	printf("opening %s\n", SRLCLIENT);
	c_fd = open(SRLCLIENT, O_RDWR | O_NONBLOCK | O_NOCTTY);
	if (c_fd < 0) {
		perror("open tty");
		exit(EXIT_FAILURE);
	}
	fcntl(c_fd, F_SETFL, 0);
	
	tcgetattr(c_fd, &c_io_old);
	c_io.c_cflag &= ~CSIZE;
	c_io.c_cflag = CS8 | CREAD | CLOCAL ;
	c_io.c_cflag &= ~PARENB;
	c_io.c_cflag &= ~CSTOPB;
	c_io.c_lflag = 0;
		
	cfsetospeed(&c_io,B115200);            // 115200 baud
        cfsetispeed(&c_io,B115200);            // 115200 baud
	tcsetattr(c_fd, TCSANOW, &c_io);
}
/* int process_serial(int fd) {
	char tmp_buffer[256];
	int bytes = 0;
	//if (ioctl(fd, FIONREAD, &bytes) == -1) {
	//	perror("ioctl read");
	//}
	//printf("ioctl read, bytes=%d \n",bytes);
	//if(bytes == 0){
		return 0;
	//}
	//int size = (bytes < 256)?bytes:sizeof(tmp_buffer);
	//printf("size=%d\n",size);
	//while(size > 0) {}
	if(fd > 0) {
		bytes = read(c_fd, tmp_buffer, 256);
		printf("client: %s",tmp_buffer);
	} else {
		bytes = read(STDIN_FILENO, tmp_buffer, 256);
		write(c_fd,tmp_buffer,bytes);
	}
	//size = size-bytes;
	return 0;
}
*/

int main() {
	set_serial();
	char o_b[256];
	fd_set rfds;
	struct timeval tv;
        int retval;

        FD_ZERO(&rfds);
	fcntl(0, F_SETFL, O_NONBLOCK);       // make the reads non-blocking
        /* Wait up to five seconds. */
        tv.tv_sec = 5;
        tv.tv_usec = 0;
	signal(SIGINT, intHandler);
	unsigned char c = 'a';
	while(keeprunning) {
	FD_SET(0, &rfds);
        FD_SET(c_fd, &rfds);
        	retval = select(c_fd+1, &rfds, NULL, NULL, &tv);
		//printf("%d : started\n",retval);
		//if(retval >= 0) {
			if(FD_ISSET(0,&rfds)) {
				printf("data to client: \n");
				int size = read(0,o_b,sizeof(o_b));
				printf("out: %s\n",o_b);
				o_b[size]='\n';
				o_b[size+1]=0;
				write(c_fd,o_b,size+1);
				memset(o_b,0,size);
				//write(c_fd,&c,1);
				//process_serial(0);
			}
			if(FD_ISSET(c_fd,&rfds)) {
				printf("data from client:\n");
				int size = read(c_fd,o_b,sizeof(o_b));
				printf("out:%s\n",o_b);
				memset(o_b,0,size);	
				//write(STDOUT_FILENO,&c,1);
				//process_serial(1);
			}
	//}
	}
	tcsetattr(c_fd, TCSADRAIN, &c_io_old);
	close(c_fd);
	printf("exiting application\n");
	exit(EXIT_SUCCESS);
}










