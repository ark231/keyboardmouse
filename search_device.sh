#!/bin/bash
tmp_info_devices=()
keyboards_names=()
keyboards_handlers=()
counta=0
while read line
do
	if [ "$line" = "" ];then
		counta=0;
	else
		if [ $counta == 2 ] && [ "$(echo $line|grep --only-matching -e '[0-9]*')" = "120013" ];then
			keyboards_names+=("$(echo ${tmp_info_devices[0]}|sed -e's/^N: Name=\|"//g')")
			keyboards_handlers+=("$(echo ${tmp_info_devices[1]}|grep --only-matching -e'event[0-9]*')")
		else
			tmp_info_devices[$counta]=$line
		fi
		counta=$((counta+1))
	fi
done< <(cat /proc/bus/input/devices |grep -e'N:\|H:\|B: EV\|^$')

for((i=0;i<${#keyboards_names[@]};i++))
do
	echo ${keyboards_names[$i]}
done
for((j=0;j<${#keyboards_handlers[@]};j++))
do
	echo ${keyboards_handlers[$j]}
done
