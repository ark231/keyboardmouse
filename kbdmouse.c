#include<stdio.h>
#include<stdlib.h>
#include<linux/input.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdbool.h>
#include<libevdev/libevdev.h>
#include<libevdev/libevdev-uinput.h>
#include<unistd.h>
#include<memory.h>
#include<getopt.h>
#define sec 1000000
#define milli_sec 1000
#define micro_sec 1


#ifdef __GNUC__
#define PRINTF(format,...) {\
	printf(format __VA_OPT__(,) __VA_ARGS__);\
	fflush(stdout);\
}

#define FPRINTF(stream,format,...) {\
	fprintf(stream,format __VA_OPT__(,) __VA_ARGS__);\
	fflush(stream);\
}
#else
#error "sorry, but I'm not clever enough to make this work without gnu extention "
#endif


typedef enum{
	KEYBOARD,
	//MOUSE,
	MOUSE_MOV,//no pause between events
	MOUSE_BTN,//some pause between events
	NOWHERE
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

typedef struct{
	int X;
	int Y;
}X_Y_vector;

//void //prep_for_quit(int ,struct libevdev_uinput[],struct libevdev_uinput[]);
void process_input_event(struct input_event,struct event_result*,X_Y_vector*);
void clear_event_result(struct event_result*);
void clear_shortcut_flags(exit_shortcut*);
void set_input_event(struct input_event*,int,int,int);
int find_max_fd(int[],int);
//void signal_handler(void);

bool requested_exit = false;//when true, mainloop will end

int main(int argc,char *argv[]){
	/*process args*/
	struct option longopts[]={
		{"X_mov",required_argument,0,'X'},
		{"Y_mov",required_argument,0,'Y'},
		{      0,                0,0, 0 },
	};
	int idx_lngopt=0;
	int val_opt=0;
	X_Y_vector value_mouse_move={.X = 5,.Y = 5};
	while((val_opt=getopt_long(argc,argv,"X:Y:",longopts,&idx_lngopt))!=-1){
		char *err=NULL;
		switch(val_opt){
			case 'X':
				{
				//PRINTF("opt_val%s\n",optarg);
				int X_mov_tmp=(int)strtol(optarg,&err,0);
				if(*err != '\n' && *err != '\0'){
					PRINTF("error: invalid value %s\n",optarg);
				}else{
					value_mouse_move.X=X_mov_tmp;
				}
				break;
				 }
			case 'Y':
				{
				//PRINTF("opt_val%s\n",optarg);
				int Y_mov_tmp=(int)strtol(optarg,&err,0);
				if(*err != '\n' && *err != '\0'){
					PRINTF("error: invalid value %s\n",optarg);
				}else{
					value_mouse_move.Y=Y_mov_tmp;
				}
				break;
				}
			case '?':
				//PRINTF("error\n");
				break;
		}
	}
	/*open and check device files*/
	int *kbd_eventfiles=NULL;
	int *kbd_uinputfiles=NULL;
	fd_set readfds ,kbd_allfds;
	int num_usable_kbd_evfl=0;
	FD_ZERO(&kbd_allfds);
	if(optind == argc){
		FPRINTF(stderr,"error: no device file specified\n");
		exit(EXIT_FAILURE);
	}else{
		if ((kbd_eventfiles = (int*)malloc(sizeof(int)*(argc-optind))) == NULL){
			FPRINTF(stderr,"error: cannot malloc kbd_eventfiles\n");
			exit(EXIT_FAILURE);
		}
		int fd_tmp=-1;
		while (optind < argc){
			fd_tmp=open(argv[optind],O_RDONLY);//block in order to reduce cpu usage
			if(fd_tmp==-1){
				continue;
			}else{
				kbd_eventfiles[num_usable_kbd_evfl]=fd_tmp;
				ioctl(fd_tmp,EVIOCGRAB,1);//exclucive block the file
				FD_SET(fd_tmp, &kbd_allfds);
				num_usable_kbd_evfl++;
			}
			optind++;
		}
		if(num_usable_kbd_evfl<=0){
			FPRINTF(stderr,"error: no usable device file\n");
			exit(EXIT_FAILURE);
		}
	}
	if((kbd_uinputfiles=(int*)malloc(sizeof(int)*num_usable_kbd_evfl))==NULL){
		FPRINTF(stderr,"error: cannot malloc kbd_uinputfiles\n");
		exit(EXIT_FAILURE);
	}
	struct libevdev **kbd_evdevs=NULL;
	if((kbd_evdevs=(struct libevdev**)malloc(sizeof(struct libevdev*)*num_usable_kbd_evfl))==NULL){
		FPRINTF(stderr,"error: cannot malloc kbd_evdevs\n");
		exit(EXIT_FAILURE);
	}
	struct libevdev_uinput **kbd_uidevs=NULL;
	if((kbd_uidevs=(struct libevdev_uinput**)malloc(sizeof(struct libevdev_uinput*)*num_usable_kbd_evfl))==NULL){
		FPRINTF(stderr,"error: cannot malloc evdev\n");

		exit(EXIT_FAILURE);
	}
	for(int k=0;k<num_usable_kbd_evfl;k++){
		int uinput_fd_tmp=-1;
		if((uinput_fd_tmp=open("/dev/uinput",O_WRONLY))==-1){
			if((uinput_fd_tmp=open("/dev/input/uinput",O_WRONLY))==-1){//I didnt choose && because when first open was successful and second doesnt, second one will destroy the result of first one's
				FPRINTF(stderr,"error: couldn't open either /dev/uinput nor /dev/input/uinput\n");
				exit(EXIT_FAILURE);
			}else{
				kbd_uinputfiles[k]=uinput_fd_tmp;
				if(libevdev_new_from_fd(kbd_eventfiles[k],&kbd_evdevs[k])!=0){
					FPRINTF(stderr,"error: couldn't new kbd_evdevs\n");
					exit(EXIT_FAILURE);
				}
				if(libevdev_uinput_create_from_device(kbd_evdevs[k],kbd_uinputfiles[k],&kbd_uidevs[k])!=0){
					FPRINTF(stderr,"error: couldn't create kbd_uidevs\n");
					exit(EXIT_FAILURE);
				}
			}
		}else{
			kbd_uinputfiles[k]=uinput_fd_tmp;
			if(libevdev_new_from_fd(kbd_eventfiles[k],&kbd_evdevs[k])!=0){
				FPRINTF(stderr,"error: couldn't new kbd_evdevs\n");
				exit(EXIT_FAILURE);
			}
			if(libevdev_uinput_create_from_device(kbd_evdevs[k],kbd_uinputfiles[k],&kbd_uidevs[k])!=0){
				FPRINTF(stderr,"error: couldn't create kbd_uidevs\n");

				exit(EXIT_FAILURE);
			}
		}
	}
	
	struct libevdev *mouse_evdev=libevdev_new();
	struct libevdev_uinput *mouse_uidev=NULL;
	libevdev_set_name(mouse_evdev,"keyboard mouse");
	libevdev_enable_event_type(mouse_evdev,EV_REL);
	libevdev_enable_event_code(mouse_evdev,EV_REL,REL_X,NULL);
	libevdev_enable_event_code(mouse_evdev,EV_REL,REL_Y,NULL);
	libevdev_enable_event_code(mouse_evdev,EV_REL,REL_WHEEL,NULL);
	libevdev_enable_event_code(mouse_evdev,EV_REL,REL_WHEEL_HI_RES,NULL);
	libevdev_enable_event_code(mouse_evdev,EV_REL,REL_HWHEEL,NULL);
	libevdev_enable_event_code(mouse_evdev,EV_REL,REL_HWHEEL_HI_RES,NULL);
	libevdev_enable_event_type(mouse_evdev,EV_KEY);
	libevdev_enable_event_code(mouse_evdev,EV_KEY,BTN_LEFT,NULL);
	libevdev_enable_event_code(mouse_evdev,EV_KEY,BTN_MIDDLE,NULL);
	libevdev_enable_event_code(mouse_evdev,EV_KEY,BTN_RIGHT,NULL);
	libevdev_uinput_create_from_device(mouse_evdev,LIBEVDEV_UINPUT_OPEN_MANAGED,&mouse_uidev);
	
	struct input_event *events;
	if((events=(struct input_event*)malloc(sizeof(struct input_event)*num_usable_kbd_evfl))==NULL){
		FPRINTF(stderr,"error: cannot malloc events\n");
		//prep_for_quit(num_usable_kbd_evfl,kbd_uidevs,mouse_uidev);
		exit(EXIT_FAILURE);
	}
	struct event_result *results;
	if((results=(struct event_result*)malloc(sizeof(struct event_result)*num_usable_kbd_evfl))==NULL){
		FPRINTF(stderr,"error: cannot malloc results\n");

		//prep_for_quit(num_usable_kbd_evfl,kbd_uidevs,mouse_uidev);
		exit(EXIT_FAILURE);
	}
	/*open and check device files end*/
	/*mainloop*/
	int fd_max = find_max_fd(kbd_eventfiles,num_usable_kbd_evfl);
	while(requested_exit == false){
		memcpy(&readfds,&kbd_allfds,sizeof(fd_set));
		select(fd_max+1,&readfds,NULL,NULL,NULL);
		for(int j=0;j<num_usable_kbd_evfl;j++){
			if(FD_ISSET(kbd_eventfiles[j],&readfds)){
				if(read(kbd_eventfiles[j],&events[j],sizeof(struct input_event))<sizeof(struct input_event)){
					PRINTF("error: cannot read from device\n");
					//prep_for_quit(num_usable_kbd_evfl,kbd_uidevs,mouse_uidev);
					exit(EXIT_FAILURE);
				}
				process_input_event(events[j],&results[j],&value_mouse_move);
				switch(results[j].distination){
					case KEYBOARD:
						for(int l=0;l<results[j].num_events;l++){
							if(libevdev_uinput_write_event(kbd_uidevs[j],results[j].events[l].type,results[j].events[l].code,results[j].events[l].value)!=0){
								FPRINTF(stderr,"error: failed to write event\n");
								exit(EXIT_FAILURE);
							}
							if(libevdev_uinput_write_event(kbd_uidevs[j],EV_SYN,SYN_REPORT,0)!=0){
								FPRINTF(stderr,"error: failed to write event\n");
								exit(EXIT_FAILURE);
							}
							usleep(20*micro_sec);
						}
						break;
					case MOUSE_BTN:
						for(int l=0;l<results[j].num_events;l++){
							if(libevdev_uinput_write_event(mouse_uidev,results[j].events[l].type,results[j].events[l].code,results[j].events[l].value)!=0){
								FPRINTF(stderr,"error: failed to write event\n");
								exit(EXIT_FAILURE);
							}
							if(libevdev_uinput_write_event(mouse_uidev,EV_SYN,SYN_REPORT,0)!=0){
								FPRINTF(stderr,"error: failed to write event\n");
								exit(EXIT_FAILURE);
							}
							if(results[j].num_events==4 && l%2==1){
								usleep(50*milli_sec);
							}else{
								usleep(20*micro_sec);
							}
						}
						break;
					case MOUSE_MOV:
#ifdef DEBUG
						PRINTF("send mouse move for %d times\n",results[j].num_events);
#endif
						for(int l=0;l<results[j].num_events;l++){
#ifdef DEBUG
							PRINTF("%dst send out of %d\n",l,results[j].num_events);
#endif
							if(libevdev_uinput_write_event(mouse_uidev,results[j].events[l].type,results[j].events[l].code,results[j].events[l].value)!=0){
								FPRINTF(stderr,"error: failed to write event\n");
								exit(EXIT_FAILURE);
							}
#ifdef DEBUG
							PRINTF("fin send data\n");
#endif 
							if(l%2 == 1){
								if(libevdev_uinput_write_event(mouse_uidev,EV_SYN,SYN_REPORT,0)!=0){
									FPRINTF(stderr,"error: failed to write event\n");
									exit(EXIT_FAILURE);
								}
#ifdef DEBUG
								PRINTF("fin send sync\n");
#endif
								usleep(20*micro_sec);
							}
						}
						if(results[j].num_events%2 == 1){//奇数回だと、最後のSYNCが送られないので、ここで判別
							if(libevdev_uinput_write_event(mouse_uidev,EV_SYN,SYN_REPORT,0)!=0){
								FPRINTF(stderr,"error: failed to write event\n");
								exit(EXIT_FAILURE);
							}
#ifdef DEBUG
							PRINTF("fin send sync\n");
#endif
							usleep(20*micro_sec);
						}
						break;
				}
			}
		}
	}
	/*meinloop end*/

	//ioctl(kbd_eventfile,EVIOCGRAB,0);
	PRINTF("exit\n");
	//prep_for_quit(num_usable_kbd_evfl,kbd_uidevs,mouse_uidev);
	return 0;
}
//#define VALUE_MOUSE_MOVE_X 5
//#define VALUE_MOUSE_MOVE_Y 5

void process_input_event(struct input_event event_to_process,struct event_result *result,X_Y_vector *val_mouse_mov){
	static mouse_btn_type selected_btn = LEFT;
	static exit_shortcut flags_pressed = {.Shift_is_pressed = false,.Alt_is_pressed = false,.NumLock_is_pressed = false};//in this method, user can quit by pressing three keys on different keyboards, but I dont care because it wont be so much probrem
	static bool NumLock_escape_activated = false;
	static bool Enter_escape_activated = false;
#ifdef OLD_ESCAPE
#define IS_ESCAPED (NumLock_escape_activated)
#elif defined MIX_ESCAPE
#define IS_ESCAPED (NumLock_escape_activated || Enter_escape_activated)
#else
#define IS_ESCAPED (Enter_escape_activated)
#endif
	clear_event_result(result);
	if(event_to_process.type==EV_KEY){
		switch(event_to_process.code){
			case KEY_KP0:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value == 1){
					//PRINTF(" KEY_KP0\n");
					result->distination = MOUSE_BTN;
					result->num_events = 1;
					set_input_event(&(result->events[0]),EV_KEY,BTN_LEFT,1);
				}
				break;
			case KEY_KP1:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					//PRINTF(" KEY_KP1\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,-val_mouse_mov->X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,val_mouse_mov->Y);
				}
				break;
			case KEY_KP2:
				clear_shortcut_flags(&flags_pressed);
				if(IS_ESCAPED && event_to_process.value != 0){
					result->distination = MOUSE_MOV;
					result->num_events = 1;
					set_input_event(&(result->events[0]),EV_REL,REL_WHEEL,-1);
					//set_input_event(&(result->events[0]),EV_REL,REL_WHEEL_HI_RES,-120);
				}else{
					if(event_to_process.value != 0){
						//PRINTF(" KEY_KP2\n");
						result->distination = MOUSE_MOV;
						result->num_events = 2;
						set_input_event(&(result->events[0]),EV_REL,REL_X,0);
						set_input_event(&(result->events[1]),EV_REL,REL_Y,val_mouse_mov->Y);
					}
				}
				break;
			case KEY_KP3:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					//PRINTF(" KEY_KP3\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,val_mouse_mov->X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,val_mouse_mov->Y);
				}
				break;
			case KEY_KP4:
				clear_shortcut_flags(&flags_pressed);
				if(IS_ESCAPED && event_to_process.value != 0){
					result->distination = MOUSE_MOV;
					result->num_events = 1;
					set_input_event(&(result->events[0]),EV_REL,REL_HWHEEL,-1);
				}else{
					if(event_to_process.value != 0){
						//PRINTF(" KEY_KP4\n");
						result->distination = MOUSE_MOV;
						result->num_events = 2;
						set_input_event(&(result->events[0]),EV_REL,REL_X,-val_mouse_mov->X);
						set_input_event(&(result->events[1]),EV_REL,REL_Y,0);
					}
				}
				break;
			case KEY_KP5:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value == 1){
					//PRINTF(" KEY_KP5\n");
					result->distination = MOUSE_BTN;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_KEY,selected_btn,1);
					set_input_event(&(result->events[1]),EV_KEY,selected_btn,0);
				}
				break;
			case KEY_KP6:
				clear_shortcut_flags(&flags_pressed);
				if(IS_ESCAPED && event_to_process.value != 0){
					result->distination = MOUSE_MOV;
					result->num_events = 1;
					set_input_event(&(result->events[0]),EV_REL,REL_HWHEEL,1);
				}else{
					if(event_to_process.value != 0){
						//PRINTF(" KEY_KP6\n");
						result->distination = MOUSE_MOV;
						result->num_events = 2;
						set_input_event(&(result->events[0]),EV_REL,REL_X,val_mouse_mov->X);
						set_input_event(&(result->events[1]),EV_REL,REL_Y,0);
					}
				}
				break;
			case KEY_KP7:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					//PRINTF(" KEY_KP7\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,-val_mouse_mov->X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,-val_mouse_mov->Y);
				}
				break;
			case KEY_KP8:
				clear_shortcut_flags(&flags_pressed);
				//PRINTF("KP8\n");
				if(IS_ESCAPED && event_to_process.value != 0){
					result->distination = MOUSE_MOV;
					result->num_events = 1;
					set_input_event(&(result->events[0]),EV_REL,REL_WHEEL,1);
					//set_input_event(&(result->events[1]),EV_REL,REL_WHEEL_HI_RES,120);
				}else{
					if(event_to_process.value != 0){
						result->distination = MOUSE_MOV;
						result->num_events = 2;
						set_input_event(&(result->events[0]),EV_REL,REL_X,0);
						set_input_event(&(result->events[1]),EV_REL,REL_Y,-val_mouse_mov->Y);
					}
				}
				break;
			case KEY_KP9:
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					//PRINTF(" KEY_KP9\n");
					result->distination = MOUSE_MOV;
					result->num_events = 2;
					set_input_event(&(result->events[0]),EV_REL,REL_X,val_mouse_mov->X);
					set_input_event(&(result->events[1]),EV_REL,REL_Y,-val_mouse_mov->Y);
				}
				break;
			case KEY_KPDOT:
				clear_shortcut_flags(&flags_pressed);
				if(NumLock_escape_activated && event_to_process.value == 1){
					PRINTF("<status>\n");
					PRINTF("X move speed: %d\n",val_mouse_mov->X);
					PRINTF("Y move speed: %d\n",val_mouse_mov->Y);
					switch(selected_btn){
						case BTN_LEFT:
							PRINTF("mouse: LEFT\n");
							break;
						case BTN_MIDDLE:
							PRINTF("mouse: MIDDLE\n");
							break;
						case BTN_RIGHT:
							PRINTF("mouse: RIGHT\n");
							break;
						default:
							break;
					}
					PRINTF("</status>\n");
				}else{
					if(event_to_process.value == 1){
						//PRINTF(" KEY_KPDOT\n");
						result->distination = MOUSE_BTN;
						result->num_events = 1;
						set_input_event(&(result->events[0]),EV_KEY,BTN_LEFT,0);
					}
				}
				break;
			case KEY_KPSLASH:
				clear_shortcut_flags(&flags_pressed);
				if(NumLock_escape_activated && event_to_process.value !=0){
					val_mouse_mov->X--;
					PRINTF("X move speed: %d\n",val_mouse_mov->X);

				}else{
					if(event_to_process.value == 1){
						//PRINTF(" KEY_KPSLASH\n");
						//result->num_events = 0;
						selected_btn = LEFT;
						PRINTF("mouse: LEFT\n");
					}
				}
				break;
			case KEY_KPASTERISK:
				clear_shortcut_flags(&flags_pressed);
				if(NumLock_escape_activated && event_to_process.value !=0){
					val_mouse_mov->X++;
					PRINTF("X move speed: %d\n",val_mouse_mov->X);
				}else{
					if(event_to_process.value == 1){
						//PRINTF(" KEY_KPASTERISK\n");
						//result->num_events = 0;
						selected_btn = MIDDLE;
						PRINTF("mouse: MIDDLE\n");
					}
				}
				break;
			case KEY_KPMINUS:
				clear_shortcut_flags(&flags_pressed);
				if(NumLock_escape_activated && event_to_process.value !=0){
					val_mouse_mov->Y--;
					PRINTF("Y move speed: %d\n",val_mouse_mov->Y);
				}else{
					if(event_to_process.value == 1){
						//PRINTF(" KEY_KPMINUS\n");
						//result->num_events = 0;
						selected_btn = RIGHT;
						PRINTF("mouse: RIGHT\n");
					}
				}
				break;
			case KEY_KPPLUS:
				clear_shortcut_flags(&flags_pressed);
				if(NumLock_escape_activated && event_to_process.value !=0){
					val_mouse_mov->Y++;
					PRINTF("Y move speed: %d\n",val_mouse_mov->Y);
				}else{
					if(event_to_process.value == 1){
						//PRINTF(" KEY_KPPLUS\n");
						result->distination = MOUSE_BTN;
						result->num_events = 4;
						set_input_event(&(result->events[0]),EV_KEY,selected_btn,1);
						set_input_event(&(result->events[1]),EV_KEY,selected_btn,0);
						set_input_event(&(result->events[2]),EV_KEY,selected_btn,1);
						set_input_event(&(result->events[3]),EV_KEY,selected_btn,0);
					}
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
						NumLock_escape_activated = true;
						/* NumLock key is not neccesary
						result->distination = KEYBOARD;
						result->num_events = 1;
						result->events[0] = event_to_process;
						*/
					}
				}else{
					clear_shortcut_flags(&flags_pressed);
					NumLock_escape_activated = false;
					/*
					result->distination = KEYBOARD;
					result->num_events = 1;
					result->events[0] = event_to_process;
					*/
				}
				break;
			case KEY_KPENTER:
				//lets disable whole tenkey!
				clear_shortcut_flags(&flags_pressed);
				if(event_to_process.value != 0){
					Enter_escape_activated = true;
				}else{
					Enter_escape_activated = false;
				}
				break;
			default:
				clear_shortcut_flags(&flags_pressed);
				//PRINTF(" else\n");
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
	to_clear->distination = NOWHERE;
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

int find_max_fd(int fds[],int num_fds){
	int max_tmp=-1;
	for(int i=0;i<num_fds;i++){
		if(max_tmp<fds[i]){
			max_tmp=fds[i];
		}
	}
	return max_tmp;
}

/*
inline void prep_for_quit(int num_uidev_to_destroy,struct libevdev_uinput kbd_uidev_to_destroy[],struct libevdev_uinput mouse_uidev_to_destroy[]){
	//destroy uinput
	for(int i=0;i<num_uidev_to_destroy;i++){
		libevdev_uinput_destroy(kbd_uidev_to_destroy[i]);
	}
	libevdev_uinput_destroy(mouse_uidev_to_destroy);
}
*/

/*
void signal_handler(void){#set flag
	requested_exit = true;
}
 */
