#!/bin/sh
#DESCR: Check libsocialweb core dbus interface
# Copyright (C) 2010 Intel Corporation
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
# Authors:
#       Jenny Cao  <jenny.q.cao@intel.com>
# Date Created: 2010/09/07
#
# Modifications:
#          Modificator  Date
#          Content of Modification

# Enter the case folder

# use regular user to execute this cases

export DISPLAY=:0.0
export http_proxy=http://proxy.pd.intel.com:911

case_dir=`dirname $0`
dbus_api_dir=$case_dir/../dbus_api

cd $case_dir
usage()
{
    printf "\nUSAGE: $0 -h -l [-t getServices_01|getServices_02|IsOnline|OnlineChanged]\n" 
    printf "\t -h: \t\thelp\n" $0
    printf "\t -l: \t\tlist test api\n"
    printf "\t -t: \t\tselect test api\n"
    exit 2
}

list_test_api()
{
    echo "Test DBus API List:"
    echo "  getServices_01"
    echo "  getServices_02"
    echo "  IsOnline"
    echo "  OnlineChanged"
    exit 2
}

exit_pass()
{
        echo "PASS"
        exit 0
}

exit_fail()
{
        echo "FAIL"
        exit 1
}



while getopts t:hl name
do
  case "$name" in
    t)
	echo $OPTARG
	if [ "$OPTARG" = "getServices_01" ] || [ "$OPTARG" = "getServices_02" ] || [ "$OPTARG" = "IsOnline" ] \
	|| [ "$OPTARG" = "OnlineChanged" ]; then
		test_type=$OPTARG
	else
	        echo "Wrong parameter for -t!"
                usage
                exit 2 
	fi
	;;
    l)
	list_test_api;;
    h)
	usage
	exit 2
	;;
  esac
done
  case "$test_type" in
	getServices_01)
	dbus-send --session --type=method_call --print-reply --dest=com.meego.libsocialweb /com/meego/libsocialweb/Service org.freedesktop.DBus.Introspectable.Introspect >/tmp/dbus_result 
	dbus_result=`cat /tmp/dbus_result|grep 'node name'|sed 's/.*<node name="\([a-zA-Z]*\)"\/>.*/\1/g'`
	echo "dbus_result="$dbus_result
	dbus_array=($dbus_result)
	rm /tmp/dbus_result

	for service in $dbus_result
	do 
		$dbus_api_dir/core_getservices_method_main.py -e $service
		if [ $? -ne 0 ]; then
			exit_fail
                fi
	done
	exit_pass
	;;
  	getServices_02)
	$dbus_api_dir/core_getservices_method_main.py -k
                if [ $? -eq 0 ]; then
                        exit_pass
                else
                        exit_fail
		fi
                ;;
        IsOnline)
                sudo ifconfig eth0 down
                $dbus_api_dir/core_online_method_main.py -s off
                if [ $? -eq 0 ]; then
                        sudo ifconfig eth0 up
			$dbus_api_dir/core_online_method_main.py -s on
                        if [ $? -eq 0 ]; then
                                exit_pass
                        else
                                exit_fail
			fi
                else
			sudo ifconfig eth0 up
                        exit_fail
		fi
                ;;
        OnlineChanged)
		cd $dbus_api_dir/
		sudo ./core_online_signal_main.py
                if [ $? -eq 0 ]; then
                        exit_pass
                else
                        exit_fail
		fi
                ;;

        ?) 
                echo "Wrong parameter for -t!"
                usage
                exit 2 
	esac
