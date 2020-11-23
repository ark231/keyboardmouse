#include<stdio.h>
#include<stdlib.h>
#include<linux/input.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdbool.h>
#define KEYBOARD "/dev/input/keyboard_with_tenkey"

void prep_for_quit(void);
//void signal_handler(void);

int main(void){
	int kbd_eventfile = open(KEYBOARD,O_RDONLY);
	struct input_event event;
	bool wanted_key_pressed = false;
	while(wanted_key_pressed == false){
		if(read(kbd_eventfile,&event,sizeof(struct input_event))<sizeof(struct input_event)){
			prep_for_quit();
			printf("error\n");
			exit(EXIT_FAILURE);
		}
		if(event.type==EV_KEY){
			switch(event.code){
				case KEY_KP0:
					printf("KEY_KP0\n");
					wanted_key_pressed=true;
					break;
				case KEY_KP1:
					printf("KEY_KP1\n");
					wanted_key_pressed=true;
					break;
				case KEY_KP2:
					printf("KEY_KP2\n");
					wanted_key_pressed=true;
					break;
				case KEY_KP3:
					printf("KEY_KP3\n");
					wanted_key_pressed=true;
					break;
				case KEY_KP4:
					printf("KEY_KP4\n");
					wanted_key_pressed=true;
					break;
				default:
					printf("else\n");
					break;
			}
		}
	}
	printf("exit\n");
	return 0;
}

void prep_for_quit(void){
	;
}

/*
void signal_handler(void){#set flag
	;
}
 */
