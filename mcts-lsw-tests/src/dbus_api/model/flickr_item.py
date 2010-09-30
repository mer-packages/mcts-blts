#!/usr/bin/python

# Copyright (C) 2010 Intel Corporation.
#
# This program is free software; you can redistribute it and/or modify it
# under the terms and conditions of the GNU Lesser General Public License,
# version 2.1, as published by the Free Software Foundation.
#
# This program is distributed in the hope it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
# more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
# Authors:
#              Tang, Shao-Feng <shaofeng.tang@intel.com>
#

import string, sys
reload(sys)
sys.setdefaultencoding("utf-8")

DEFAULT_BUDDY_ICON = "http://www.flickr.com/images/buddyicon.jpg"

def creatFlickrItem(photo):
    if('url_m' in photo.keys()):
        item = FlickrItemModel()
        if('owner' in photo.keys() and 'id' in photo.keys()):
            item.id = ("http://www.flickr.com/photos/%s/%s/" % (photo.attrib['owner'], photo.attrib['id']))
            item.url = item.id
        if('title' in photo.keys() ):
            item.title = photo.attrib['title']
        if('owner' in photo.keys() ):
            item.authorid = photo.attrib['owner']
        if('username' in photo.keys() ):
            item.author=photo.attrib['username']
        if('dateupload' in photo.keys() ):
            item.date = photo.attrib['dateupload']
        if('url_m' in photo.keys() ):
            item.thumbnail = photo.attrib['url_m']
        if(('owner' in photo.keys()) and ('iconfarm' in photo.keys()) 
           and ('iconserver' in photo.keys())
           and (0 != string.atoi (photo.attrib['iconserver']) )):    
            item.authoricon = "http://farm%s.static.flickr.com/%s/buddyicons/%s.jpg" % \
                (photo.attrib['iconfarm'], photo.attrib['iconserver'], photo.attrib['owner'])
        
        if('url_o' in photo.keys()):
            item.x_flickr_photo_url = photo.attrib['url_o']
        elif ('url_l' in photo.keys()):
            item.x_flickr_photo_url = photo.attrib['url_l']
        elif ('url_m' in photo.keys()):
            item.x_flickr_photo_url = photo.attrib['url_m']
        
        print "Create a new item (ID: %s" % item.id
        
        return item
    else:
        return None

class FlickrItemModel(object):
    '''
    represent the flickr item which is shown on myZone.
    
    '''
    
    def __init__(self):
        '''
        Constructor
        '''
        self.id = "";
        self.x_flickr_photo_url = ""
        self.title = ""
        self.url = ""
        self.authoricon = DEFAULT_BUDDY_ICON
        self.authorid = ""
        self.author=""
        self.date = None
        self.thumbnail = ""

    def get_thumbnail(self):
        return self.__thumbnail


    def set_thumbnail(self, value):
        self.__thumbnail = value


    def del_thumbnail(self):
        del self.__thumbnail


    def get_id(self):
        return self.__id


    def get_x_flickr_photo_url(self):
        return self.__x_flickr_photo_url


    def get_title(self):
        return self.__title


    def get_url(self):
        return self.__url


    def get_authoricon(self):
        return self.__authoricon


    def get_authorid(self):
        return self.__authorid


    def get_author(self):
        return self.__author


    def get_date(self):
        return self.__date


    def set_id(self, value):
        self.__id = value


    def set_x_flickr_photo_url(self, value):
        self.__x_flickr_photo_url = value


    def set_title(self, value):
        self.__title = value


    def set_url(self, value):
        self.__url = value


    def set_authoricon(self, value):
        self.__authoricon = value


    def set_authorid(self, value):
        self.__authorid = value


    def set_author(self, value):
        self.__author = value


    def set_date(self, value):
        self.__date = value


    def del_id(self):
        del self.__id


    def del_x_flickr_photo_url(self):
        del self.__x_flickr_photo_url


    def del_title(self):
        del self.__title


    def del_url(self):
        del self.__url


    def del_authoricon(self):
        del self.__authoricon


    def del_authorid(self):
        del self.__authorid


    def del_author(self):
        del self.__author


    def del_date(self):
        del self.__date

    id = property(get_id, set_id, del_id, "id's docstring")
    x_flickr_photo_url = property(get_x_flickr_photo_url, set_x_flickr_photo_url, del_x_flickr_photo_url, "x_flickr_photo_url's docstring")
    title = property(get_title, set_title, del_title, "title's docstring")
    url = property(get_url, set_url, del_url, "url's docstring")
    authoricon = property(get_authoricon, set_authoricon, del_authoricon, "authoricon's docstring")
    authorid = property(get_authorid, set_authorid, del_authorid, "authorid's docstring")
    author = property(get_author, set_author, del_author, "author's docstring")
    date = property(get_date, set_date, del_date, "date's docstring")
    thumbnail = property(get_thumbnail, set_thumbnail, del_thumbnail, "thumbnail's docstring")
        
    