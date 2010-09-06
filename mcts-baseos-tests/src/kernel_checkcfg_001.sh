#!/bin/sh

cd `dirname $0`
basecfg="netbook_config"
kernelcfg="/boot/config-`uname -r`"
pass="yes"
while read line; do
	comments=`echo $line | sed -ne "/^#/p"`
	[ -n "$comments" ] && continue
		grep -q "$line" $kernelcfg
		if [ $? -eq 0 ]; then
			echo "Find: $line"
		else
			echo "Not find: $line"	
			pass="fail"
		fi
                scen_count=$(($scen_count+1))
done < $basecfg 
if [ "$pass" = "fail" ]; then
	echo "there are cfg options not set as expected"
	exit 1
fi
exit 0

