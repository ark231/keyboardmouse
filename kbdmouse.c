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
	//MOUSE,
	MOUSE_MOV,//no pause between events
	MOUSE_BTN,//some pause between events
	NONE
} device_type;

typedef enum{
	LEFT = BTN_LEFT,
	MIDDLE =  BTN_MIDDLE,
	RIGHT = BTN_RIGHT
} mouse_btn_type;

struct event_result{
	struct input_event events[4];//max when double click
	device_type distination;
	int num_events;
};

typedef struct {
	bool Shift_is_pressed;
	bool Alt_is_pressed;
	bool NumLock_is_pressed;
} exit_shortcut;

void prep_for_quit(int[]);
void process_input_event(struct input_event,struct event_result*);
void clear_event_result(struct event_result*);
void clear_shortcut_flags(exit_shortcut*);
void set_input_event(struct input_event*,int,int,int);
//void signal_handler(void);

bool requested_exit = false;//when true, mainloop will end

int main(int argc,char *argv[]){
	/*
	int kbd_eventfile = open(KEYBOARD,O_RDONLY|O_NONBLOCK);
	int kbd_eventfile_test = open("/dev/input/event3",O_RDONLY|O_NONBLOCK);
	ioctl(kbd_eventfile,EVIOCGRAB,1);
	*/

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
	while(requested_exit == false){
		for(int j=0;j<num_usable_kbd_evfl;j++){
			if(read(kbd_eventfiles[j],&events[j],sizeof(struct input_event))<sizeof(struct input_event)){
				prep_for_quit(kbd_eventfiles);
				printf("error: cannot read from device\n");
				exit(EXIT_FAILURE);
			}
			process_input_event(events[j],&results[j]);
			/*if(results[j].distination==MOUSE_MOV){
				requested_exit=true;
			}*/
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
	static mouse_btn_type selected_btn = LEFT;
	static exit_shortcut flags_pressed = {.Shift_is_pressed = false,.Alt_is_pressed = false,.NumLock_is_pressed = false};//in this method, user can quit by pressing three keys on different keyboards, but I dont care because it wont be so much probrem
	clear_event_result(result);
	if(event_to_process.type==EV_KEY){
		switch(event_to_process.code){
			case KEY_KP0:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value == 1){
					printf(" KEY_KP0\n");
					result->distination = MOUSE_BTN;
					result->num_events = 1;
					set_input_event(&(result->events[0]),EV_KEY,BTN_LEFT,1);
				}
				break;
			case KEY_KP1:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					printf(" KEY_KP1\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,-VALUE_MOUSE_MOVE_X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,-VALUE_MOUSE_MOVE_Y);
				}
				break;
			case KEY_KP2:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					printf(" KEY_KP2\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,0);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,-VALUE_MOUSE_MOVE_Y);
				}
				break;
			case KEY_KP3:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					printf(" KEY_KP3\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,VALUE_MOUSE_MOVE_X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,-VALUE_MOUSE_MOVE_Y);
				}
				break;
			case KEY_KP4:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					printf(" KEY_KP4\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,-VALUE_MOUSE_MOVE_X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,0);
				}
				break;
			case KEY_KP5:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value == 1){
					printf(" KEY_KP5\n");
					result->distination = MOUSE_BTN;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_KEY,selected_btn,1);
					set_input_event(&(result->events[1]),EV_KEY,selected_btn,0);
				}
				break;
			case KEY_KP6:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					printf(" KEY_KP6\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,VALUE_MOUSE_MOVE_X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,0);
				}
				break;
			case KEY_KP7:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					printf(" KEY_KP7\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,-VALUE_MOUSE_MOVE_X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,VALUE_MOUSE_MOVE_Y);
				}
				break;
			case KEY_KP8:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					printf(" KEY_KP8\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,0);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,VALUE_MOUSE_MOVE_Y);
				}
				break;
			case KEY_KP9:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					printf(" KEY_KP9\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,VALUE_MOUSE_MOVE_X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,VALUE_MOUSE_MOVE_Y);
				}
				break;
			case KEY_KPDOT:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value == 1){
					printf(" KEY_KPDOT\n");
					result->distination = MOUSE_BTN;
					result->num_events = 1;
					set_input_event(&(result->events[0]),EV_KEY,BTN_LEFT,0);
				}
				break;
			case KEY_KPSLASH:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value == 1){
					printf(" KEY_KPSLASH\n");
					result->num_events = 0;
					selected_btn = LEFT;
				}
				break;
			case KEY_KPASTERISK:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value == 1){
					printf(" KEY_KPASTERISK\n");
					result->num_events = 0;
					selected_btn = MIDDLE;
				}
				break;
			case KEY_KPMINUS:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value == 1){
					printf(" KEY_KPMINUS\n");
					result->num_events = 0;
					selected_btn = RIGHT;
				}
				break;
			case KEY_KPPLUS:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value == 1){
					printf(" KEY_KPPLUS\n");
					result->distination = MOUSE_BTN;
					result->num_events = 4;
					set_input_event(&(result->events[0]),EV_KEY,selected_btn,1);
					set_input_event(&(result->events[1]),EV_KEY,selected_btn,0);
					set_input_event(&(result->events[2]),EV_KEY,selected_btn,1);
					set_input_event(&(result->events[3]),EV_KEY,selected_btn,0);
				}
				break;
			case KEY_LEFTSHIFT:
				if(event_to_process.value != 0){
					if(flags_pressed.Alt_is_pressed == true && flags_pressed.NumLock_is_pressed == true){
						requested_exit = true;
					}else{
						flags_pressed.Shift_is_pressed = true;
						result->distination = KEYBOARD;
						result->num_events = 1;
						result->events[0] = event_to_process;
					}
				}else{
					clear_shortcut_flags(&flags_pressed);
					result->distination = KEYBOARD;
					result->num_events = 1;
					result->events[0] = event_to_process;
				}
				break;
			case KEY_LEFTALT:
				if(event_to_process.value != 0){
					if(flags_pressed.Shift_is_pressed == true && flags_pressed.NumLock_is_pressed == true){
						requested_exit = true;
					}else{
						flags_pressed.Alt_is_pressed= true;
						result->distination = KEYBOARD;
						result->num_events = 1;
						result->events[0] = event_to_process;
					}
				}else{
					clear_shortcut_flags(&flags_pressed);
					result->distination = KEYBOARD;
					result->num_events = 1;
					result->events[0] = event_to_process;
				}
				break;
			case KEY_NUMLOCK:
				if(event_to_process.value != 0){
					if(flags_pressed.Shift_is_pressed == true && flags_pressed.Alt_is_pressed== true){
						requested_exit = true;
					}else{
						flags_pressed.NumLock_is_pressed= true;
						result->distination = KEYBOARD;
						result->num_events = 1;
						result->events[0] = event_to_process;
					}
				}else{
					clear_shortcut_flags(&flags_pressed);
					result->distination = KEYBOARD;
					result->num_events = 1;
					result->events[0] = event_to_process;
				}
				break;
			default:
				clear_shortcut_flags(&flags_pressed);
				printf(" else\n");
				result->distination = KEYBOARD;
				result->num_events = 1;
				result->events[0] = event_to_process;
				break;
		}
	}
}

void set_input_event(struct input_event *to_set,int type,int code,int value){
	to_set->type=type;
	to_set->code=code;
	to_set->value=value;
}

const struct input_event input_event_NULL={.type=0,.code=0,.value=0};
inline void clear_event_result(struct event_result *to_clear){
	to_clear->distination = NONE;
	to_clear->num_events = 0;
	for(int i=0;i<4;i++){
		to_clear->events[i] = input_event_NULL;
	}
}

inline void clear_shortcut_flags(exit_shortcut *to_clear){
	to_clear->Shift_is_pressed = false;
	to_clear->Alt_is_pressed = false;
	to_clear->NumLock_is_pressed = false;
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
