#!/bin/bash
DIR_SCRIPT=$(cd $(dirname $0);pwd)
echo $DIR_SCRIPT
if [ "$(ps -a|grep "kbdmouse")" = "" ]; then
	zenity --question --text="キーボードマウスを有効化しますか？\n（テンキーが必要です）" --ok-label="はい" --cancel-label="いいえ" --width 250
	if [ $? != 0 ]; then
		exit 1
	else
		#有効化処理
		mkfifo $DIR_SCRIPT/kbdmouse_err_msg
		pkexec $DIR_SCRIPT/kbdmouse $($DIR_SCRIPT/search_device.sh) 2>$DIR_SCRIPT/kbdmouse_err_msg&
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
