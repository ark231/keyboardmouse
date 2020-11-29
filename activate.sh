#!/bin/bash
DIR_SCRIPT=$(cd $(dirname $0);pwd)
echo $DIR_SCRIPT
if [ "$(ps -a|grep "kbdmouse")" = "" ]; then
	zenity --question --text="キーボードマウスを有効化しますか？\n（テンキーが必要です）" --ok-label="はい" --cancel-label="いいえ" --width 250
	if [ $? != 0 ]; then
		exit 1
	else
		#有効化処理
		pkexec $DIR_SCRIPT/kbdmouse $($DIR_SCRIPT/search_device.sh)
		:
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
