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

user=`ps -C Xorg -o user=`
banned_file=/home/$user/.cache/libsocialweb/lastfm-banned.txt
if [ -e $banned_file ]; then
    rm $banned_file
fi
export DISPLAY=:0.0
#export http_proxy=http://proxy.pd.intel.com:911

case_dir=`dirname $0`
dbus_api_dir=$case_dir/../dbus_api

usage()
{
    printf "\nUSAGE: $0 -h -l [-t ItemView|CacheData|HideItem|ItemHidden|UserChanged|CapabilitiesChanged]\n" 
    printf "\t -h: \t\thelp\n" $0
    printf "\t -l: \t\tlist test cases\n"
    printf "\t -t: \t\tselect test api\n"
    exit 2
}

list_test_api()
{
    echo "Test DBus API List:"
    echo "  ItemView"
    echo "  HideItem"
    echo "  ItemHidden"
    echo "  UserChanged"
    echo "  CapabilitiesChanged"
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
	test_type=$OPTARG
	;;
    l)
	list_test_api;;
    h)
	usage
	exit 2
	;;
  esac
done
  cd $dbus_api_dir
  case "$test_type" in
  	ItemView)
	$dbus_api_dir/lastfm_main.py -w1
                if [ $? -eq 0 ]; then
                        exit_pass
                else
                        exit_fail
		fi
                ;;
	CacheData)
		$dbus_api_dir/lastfm_main.py -w1	
		grep "\[http" /tmp/lastfm.items.cache |sort -n >/tmp/cache.new
		grep "\[http" /home/$user/.cache/libsocialweb/cache/lastfm-feed* |sort -n >/tmp/cache.old
		diff /tmp/cache.new /tmp/cache.old
		if [ $? -eq 0 ]; then
			rm -f /tmp/cache.*
			exit_pass
		else
			rm -f /tmp/cache.*
			exit_fail
		fi
		;;	
        HideItem)
                $dbus_api_dir/lastfm_banishable_main.py -i  "http://www.last.fm/music/Robert+Pattinson/_/Never+Think tinaknows"
                if [ $? -eq 0 ]; then
			exit_pass
                else
                        exit_fail
		fi
                ;;
        ItemHidden)
		$dbus_api_dir/lastfm_banishable_main.py -i  "http://www.last.fm/music/Robert+Pattinson/_/Never+Think tinaknows" -s
                if [ $? -eq 0 ]; then
                        exit_pass
                else
                        exit_fail
		fi
                ;;
        UserChanged)
                $dbus_api_dir/service_user_changed_main.py -s lastfm 
                if [ $? -eq 0 ]; then
                        exit_pass
                else
                        exit_fail
                fi
                ;;
        CapabilitiesChanged)
		$dbus_api_dir/service_user_changed_main.py -s lastfm -c
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
