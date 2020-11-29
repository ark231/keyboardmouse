#!/bin/bash
CONFIG_FILE="$HOME/.config/kbdmouse/config"
get_val_from_configfl(){
	eval $1=$(cat $CONFIG_FILE|grep $2|sed -e "s/^[[:space:]]*$2[[:space:]]*=[[:space:]]*\|[[:space:]]*$//g")
}
DIR_SCRIPT=$(cd $(dirname $0);pwd)
if [ "$(ps -a|grep "kbdmouse")" = "" ]; then
	zenity --question --text="キーボードマウスを有効化しますか？\n（テンキーが必要です）" --ok-label="はい" --cancel-label="いいえ" --width 250
	if [ $? != 0 ]; then
		exit 1
	else
		#有効化処理
		get_val_from_configfl "X_mov_from_conf" "X_MOV"
		get_val_from_configfl "Y_mov_from_conf" "Y_MOV"
		if [ "$X_mov_from_conf" != "" ];then
			X_mov_arg="--X_mov $X_mov_from_conf"
		else
			X_mov_arg=""
		fi
		if [ "$Y_mov_from_conf" != "" ];then
			Y_mov_arg="--Y_mov $Y_mov_from_conf"
		else
			Y_mov_arg=""
		fi
		mkfifo $DIR_SCRIPT/kbdmouse_err_msg
		pkexec $DIR_SCRIPT/kbdmouse $($DIR_SCRIPT/search_device.sh) 2>$DIR_SCRIPT/kbdmouse_err_msg $X_mov_arg $Y_mov_arg &
		#$DIR_SCRIPT/kbdmouse $($DIR_SCRIPT/search_device.sh) 2>$DIR_SCRIPT/kbdmouse_err_msg& # this must fail
		KBD_MOUSE_ERR_MSG=$(env DIR_SCRIPT=$DIR_SCRIPT cat $DIR_SCRIPT/kbdmouse_err_msg)
		if [ "$KBD_MOUSE_ERR_MSG" != "" ];then
			notify-send -u critical -a kbdmouse -c ERROR "error message" "$KBD_MOUSE_ERR_MSG"
		fi
		rm $DIR_SCRIPT/kbdmouse_err_msg
	fi
else
	zenity --question --text="キーボードマウスを無効化しますか？" --ok-label="はい" --cancel-label="いいえ" --width 250
	if [ $? != 0 ]; then
		exit 2
	else
		#無効化処理
		:
	fi
fi
