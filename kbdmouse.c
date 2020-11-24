#include<stdio.h>
#include<stdlib.h>
#include<linux/input.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdbool.h>
//#define KEYBOARD "/dev/input/keyboard_with_tenkey"
typedef enum{
	KEYBOARD,
	MOUSE_MOV,
	MOUSE_BTN,
	NONE
} device_type;

struct event_result{
	struct input_event event_1;
	struct input_event event_2;//for mouse(2nd axis) if dist is KEYBOARD or MOUSE_BTN, do NOT access because this'll not be initialized
	device_type distination;
};

void prep_for_quit(int[]);
void process_input_event(struct input_event,struct event_result*);
 void reset_event_result(struct event_result*);
//void signal_handler(void);

int main(int argc,char *argv[]){
	/*
	int kbd_eventfile = open(KEYBOARD,O_RDONLY|O_NONBLOCK);
	int kbd_eventfile_test = open("/dev/input/event3",O_RDONLY|O_NONBLOCK);
	ioctl(kbd_eventfile,EVIOCGRAB,1);
	*/
	bool wanted_key_pressed = false;

	/*open and check device files*/
	int *kbd_eventfiles;
	int num_usable_kbd_evfl = argc-1;
	if(argc == 1){
		fprintf(stderr,"error: no device file specified\n");
		exit(EXIT_FAILURE);
	}else{
		if ((kbd_eventfiles = (int*)malloc(sizeof(int)*(argc-1))) == NULL){
			fprintf(stderr,"error: cannot malloc kbd_eventfiles\n");
			exit(EXIT_FAILURE);
		}
		int counta_usable_fl=0;
		int fd_tmp=-1;
		for(int i=1;i<=(argc-1);i++){
			fd_tmp=open(argv[i],O_RDONLY|O_NONBLOCK);
			if(fd_tmp==-1){
				num_usable_kbd_evfl-1;
				continue;
			}else{
				kbd_eventfiles[counta_usable_fl]=fd_tmp;
				counta_usable_fl++;
			}
		}
		printf("%d\n",num_usable_kbd_evfl);
		if(num_usable_kbd_evfl<=0){
			fprintf(stderr,"error: no usable device file\n");
			exit(EXIT_FAILURE);
		}
	}
	struct input_event *events;
	if((events=(struct input_event*)malloc(sizeof(struct input_event)*num_usable_kbd_evfl))==NULL){
		fprintf(stderr,"error: cannot malloc events\n");
		exit(EXIT_FAILURE);
	}
	struct event_result *results;
	if((results=(struct event_result*)malloc(sizeof(struct event_result)*num_usable_kbd_evfl))==NULL){
		fprintf(stderr,"error: cannot malloc results\n");
		exit(EXIT_FAILURE);
	}
	/*open and check device files end*/
	/*mainloop*/
	while(wanted_key_pressed == false){
		for(int j=0;j<num_usable_kbd_evfl;j++){
			if(read(kbd_eventfiles[j],&events[j],sizeof(struct input_event))<sizeof(struct input_event)){
				prep_for_quit(kbd_eventfiles);
				printf("error: cannot read from device\n");
				exit(EXIT_FAILURE);
			}
			process_input_event(events[j],&results[j]);
			if(results[j].distination==MOUSE_MOV){
				wanted_key_pressed=true;
			}
		}
	}
	/*meinloop end*/

	//ioctl(kbd_eventfile,EVIOCGRAB,0);
	printf("exit\n");
	return 0;
}
#define VALUE_MOUSE_MOVE_X 1
#define VALUE_MOUSE_MOVE_Y 1

void process_input_event(struct input_event event_to_process,struct event_result *result){
	reset_event_result(result);
	if(event_to_process.type==EV_KEY){
		switch(event_to_process.code){
			case KEY_KP0:
				printf(" KEY_KP0\n");
				result->distination = MOUSE_BTN;
				result->event_1.type = EV_KEY;
				result->event_1.code = BTN_LEFT;
				result->event_1.value = 1;
				break;
			case KEY_KP1:
				printf(" KEY_KP1\n");
				result->distination = MOUSE_MOV;
				result->event_1.type = EV_REL;
				result->event_1.code = REL_X;
				result->event_1.value = -VALUE_MOUSE_MOVE_X;
				result->event_2.type = EV_REL;
				result->event_2.code = REL_Y;
				result->event_2.value = -VALUE_MOUSE_MOVE_Y;
				break;
			case KEY_KP2:
				printf(" KEY_KP2\n");
				result->distination = MOUSE_MOV;
				result->event_1.type = EV_REL;
				result->event_1.code = REL_X;
				result->event_1.value = 0;
				result->event_2.type = EV_REL;
				result->event_2.code = REL_Y;
				result->event_2.value = -VALUE_MOUSE_MOVE_Y;
				break;
			case KEY_KP3:
				printf(" KEY_KP3\n");
				result->distination = MOUSE_MOV;
				result->event_1.type = EV_REL;
				result->event_1.code = REL_X;
				result->event_1.value = VALUE_MOUSE_MOVE_X;
				result->event_2.type = EV_REL;
				result->event_2.code = REL_Y;
				result->event_2.value = -VALUE_MOUSE_MOVE_Y;
				break;
			case KEY_KP4:
				printf(" KEY_KP4\n");
				result->distination = MOUSE_MOV;
				result->event_1.type = EV_REL;
				result->event_1.code = REL_X;
				result->event_1.value = -VALUE_MOUSE_MOVE_X;
				result->event_2.type = EV_REL;
				result->event_2.code = REL_Y;
				result->event_2.value = 0;
				break;
			default:
				printf(" else\n");
				result->distination = KEYBOARD;
				result->event_1 = event_to_process;
				break;
		}
	}
}

const struct input_event input_event_NULL={.type=0,.code=0,.value=0};
 void reset_event_result(struct event_result *to_reset){
	to_reset->distination = NONE;
	to_reset->event_1 = input_event_NULL;
	to_reset->event_2 = input_event_NULL;
}

void prep_for_quit(int fds_to_close[]){
	//destroy uinput
	//close uinput
	//unlock input device files
	//close input device files
	//
	;
}

/*
void signal_handler(void){#set flag
	;
}
 */
