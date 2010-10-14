#!/usr/bin/python
#
# Copyright (C) 2010, Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU General Public License,
# version 2, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place - Suite 330, Boston, MA 02111-1307 USA.
#
# Authors:
#              Tang, Shao-Feng <shaofeng.tang@intel.com>
#
# Description:
#    represent the item list which is shown on myZone


import urllib2, urllib, time
from lxml import etree
from item import ItemTestModel
import item, sys
reload(sys)
sys.setdefaultencoding("utf-8")

class MyZoneModel (object):
    """
    represent the item list which is shown on myZone
    """
    
    def __init__(self, user, apiKey):
        """
        Constructor for the item list which is shown on myZone with 2 params,
        user, the name of the user who is using Meego 
        apiKey, the key for invoking last.fm APIs
        """
        self.user = user
        self.apiKey = apiKey
        self.items = {}
        self.item_array = []

    def get_items(self):
        return self.__items
    
    def items_size(self):
        return len(self.__items.keys())

    def set_items(self, value):
        self.__items = value


    def del_items(self):
        del self.__items


    def get_user(self):
        return self.__user

    def get_api_key(self):
        return self.__apiKey

    def set_user(self, value):
        self.__user = value

    def set_api_key(self, value): 
        self.__apiKey = value

    user = property(get_user, set_user, None, "user, the name of the user who is using Meego")
    apiKey = property(get_api_key, set_api_key, None, "apiKey, the key for invoking last.fm APIs")
    items = property(get_items, set_items, del_items, "Items List")
    
    def invokeRestApi(self):
        url = "http://ws.audioscrobbler.com/2.0/?method=user.getFriends&user=%s&api_key=%s&timestamp=%s" % (urllib.quote(self.get_user().encode('utf-8')), self.get_api_key(), time.time())
        website = item.sendHttpRequest(url)
        friends_xml = website.read()
        
        return friends_xml
    
    def parsResponseXml(self, friends_xml):
        if None != friends_xml:
            root = self.__parse(friends_xml)
            friends = root.xpath("//user")
            for friend in friends:
                if(0 < len(friend.xpath("./name/text()"))):
                    item = ItemTestModel(friend.xpath("./name/text()")[0], "last.fm")
                    self.item_array.append(item)
                    item.set_user_image_url(friend.xpath("./image[@size='medium']/text()"))
                    if(0 < len(friend.xpath("./realname/text()"))):
                        item.set_real_name(friend.xpath("./realname/text()")[0])
                    else:
                        item.set_real_name(item.get_user())
                #print friend


    def __parse(self, xmlStr):
        try:
            root = etree.fromstring(xmlStr)
            return root
        except Exception, e:
            print e
            
    def updateFriendsRecentTrack(self):
        for item in self.item_array:
            new_item_array = item.updateRecentTrack(self.get_api_key())
            if(len(new_item_array)):
                for new_item in new_item_array:
                    self.items[new_item.get_uuid()] = new_item
            
