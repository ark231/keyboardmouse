#!/bin/bash
CONFIG_FILE="$HOME/.config/kbdmouse/config"
get_val_from_configfl(){
	eval $1=$(cat $CONFIG_FILE|grep $2|sed -e "s/^[[:space:]]*$2[[:space:]]*=[[:space:]]*\|[[:space:]]*$//g")
}

notify_msg(){
	$DIR_SCRIPT/notification.sh $1 $2
}

DIR_SCRIPT=$(cd $(dirname $0);pwd)
if [ "$(ps -a|grep "kbdmouse")" = "" ]; then
	get_val_from_configfl "confirm_before_start" "CONFIRM"
	if [ "a$confirm_before_start" = "a" ] || [ "a$confirm_before_start" = "atrue" ];then
		zenity --question --text="キーボードマウスを有効化しますか？\n（テンキーが必要です）" --ok-label="はい" --cancel-label="いいえ" --width 250
		if [ $? != 0 ]; then
			exit 1
		fi
	fi
	#有効化処理
	get_val_from_configfl "X_mov_from_conf" "X_MOV"
	get_val_from_configfl "Y_mov_from_conf" "Y_MOV"
	get_val_from_configfl "notification" "NOTIFICATION"
	if [ "a$X_mov_from_conf" != "a" ];then
		X_mov_arg="--X_mov $X_mov_from_conf"
	else
		X_mov_arg=""
	fi
	if [ "a$Y_mov_from_conf" != "a" ];then
		Y_mov_arg="--Y_mov $Y_mov_from_conf"
	else
		Y_mov_arg=""
	fi
	ERR_MSG_FIFO_NAME=$(mktemp -u kbdmouse_err_msg_XXX)
	NORMAL_MSG_FIFO_NAME=$(mktemp -u kbdmouse_normal_msg_XXX)
	mkfifo $DIR_SCRIPT/$ERR_MSG_FIFO_NAME
	mkfifo $DIR_SCRIPT/$NORMAL_MSG_FIFO_NAME
	if [ "a$notification" = "a" ] || [ "a$notification" = "afull" ];then
		notify_msg "normal" "$DIR_SCRIPT/$NORMAL_MSG_FIFO_NAME"&
	else
		cat "$DIR_SCRIPT/$NORMAL_MSG_FIFO_NAME">/dev/null &
	fi
	if [ "a$notification" = "a" ] || [ "a$notification" = "aerror" ] || [ "a$notification" = "afull" ];then
		notify_msg "error" "$DIR_SCRIPT/$ERR_MSG_FIFO_NAME"&
	else
		cat "$DIR_SCRIPT/$ERR_MSG_FIFO_NAME">/dev/null &
	fi
	pkexec bash -c "$DIR_SCRIPT/kbdmouse $($DIR_SCRIPT/search_device.sh) $X_mov_arg $Y_mov_arg 1>$DIR_SCRIPT/$NORMAL_MSG_FIFO_NAME 2>$DIR_SCRIPT/$ERR_MSG_FIFO_NAME"
	rm $DIR_SCRIPT/$ERR_MSG_FIFO_NAME
	rm $DIR_SCRIPT/$NORMAL_MSG_FIFO_NAME
else
	zenity --error --text="すでにキーボードマウスが有効になっています。" --width 300
	exit 2
fi
