#include <linux/input.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#define EVENT_MOUSE "/dev/input/event5"
#define EVENT_CLICK "/dev/input/event4"

int main() {
    struct input_event ev;
	fd_set read_fds;
	int fd_mouse, fd_click, max_fd;
	
	fd_mouse, fd_click, max_fd;

	fd_mouse = open(EVENT_MOUSE, O_RDONLY);
	fd_click = open(EVENT_CLICK, O_RDONLY);

	if(fd_mouse < 0 || fd_click < 0){
		perror("Error opeing device files");
		return 1;
	}

	max_fd = (fd_mouse > fd_click) ? fd_mouse : fd_click;

	printf("Listening for mouse events...\n");


while (1) {
	FD_ZERO(&read_fds);
	FD_SET(fd_mouse, &read_fds);
	FD_SET(fd_click, &read_fds);

	if(select(max_fd + 1, &read_fds, NULL, NULL, NULL) < 0){
		perror("select() failed");
		break;
	}

	if(FD_ISSET(fd_mouse, &read_fds)){
		read(fd_mouse, &ev, sizeof(struct input_event));
		if(ev.type == EV_REL){
			if(ev.code == REL_X){
				printf("-> Mouse moved X: %d\n", ev.value);
			}else if(ev.code == REL_Y){
				printf("-> Mouse moved Y: %d\n", ev.value);
			}
		}
	}

	if(FD_ISSET(fd_click, &read_fds)){
		read(fd_click, &ev, sizeof(struct input_event));
		if(ev.type == EV_KEY){
			if(ev.code == BTN_LEFT){
				printf("-> LeftClick %s\n", ev.value ? "Pressed" : "Released");
			}else if(ev.code == BTN_RIGHT){
				printf("-> Right Click %s\n", ev.value ? "Pressed" : "Released");
			}else if(ev.code == BTN_MIDDLE){
				printf("-> Middle Click %s\n", ev.value ? "Pressed" : "Released");
			}
		}
	}
}

	close(fd_mouse);
	close(fd_click);
	return 0;
}

