#!/bin/bash
DIR_SCRIPT=$(cd $(dirname $0);pwd)
case $1 in
	"error")
		cat $2|while read line
		do
			notify-send -u critical -a kbdmouse -c ERROR "error message" "$line"
		done
		;;
	"normal")
		IS_IN_STATUS=false
		MOUSE_PIC=""
		cat $2|while read line
		do
			if [[ "$line" =~ '<status>' ]];then
				IS_IN_STATUS=true
				MSG_BUF=""
			elif [[ "$line" =~ '</status>' ]];then
				IS_IN_STATUS=false
				notify-send -u normal -a kbdmouse -c MESSAGE -t 5000 "message" "$MSG_BUF"
			elif $IS_IN_STATUS;then
				line=$(echo $line|sed -e "s/[[:digit:]]\+/<u><b>&<\/b><\/u>/g")
				if [[ "$line" =~ 'mouse' ]];then
					line="<img src='$DIR_SCRIPT/pictures/mouse_"$(echo $line|sed -e "s/mouse:[[:space:]]//")".png' alt='LEFT' />"
				fi
				MSG_BUF=${MSG_BUF}${line}"<br>"
			else
				line=$(echo $line|sed -e "s/[[:digit:]]\+/<u><b>&<\/b><\/u>/g")
				if [[ "$line" =~ 'mouse' ]];then
					line="<img src='$DIR_SCRIPT/pictures/mouse_"$(echo $line|sed -e "s/mouse:[[:space:]]//")".png' alt='LEFT' />"
				fi
				notify-send -u normal -a kbdmouse -c MESSAGE -t 250 "message" "$line"
			fi
		done
		;;
esac
