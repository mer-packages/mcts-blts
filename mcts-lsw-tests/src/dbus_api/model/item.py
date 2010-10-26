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
#    represent the "last.fm" item which is shown on myZone
import time
import urllib2, urllib
from lxml import etree
import os, sys
reload(sys)
sys.setdefaultencoding("utf-8")


def now():
    return time.strftime("%T", time.localtime())

def extractElementText(tagName, pro, root):
    path = ""
    if (None == pro ): 
        path = "./%s/text()" % tagName
    else:
        path = "./%s[@%s]/text()" % (tagName, pro)
    texts = root.xpath(path)
    if(0 < len(texts)):
        return texts[0]
    else:
        return None
    
def setProxy():
    envProxy = os.getenv("http_proxy", None)
    if(None == envProxy or 0 == len(envProxy.strip())):
        opener = urllib2.build_opener(urllib2.BaseHandler)
    else:
        if("/" == envProxy[-1]):
            envProxy = envProxy[: -1]
        proxy = envProxy[len("http://"):]
        print "Get the env 'http_proxy': %s" % proxy
        opener = urllib2.build_opener( urllib2.ProxyHandler({'http':proxy}) )
    urllib2.install_opener( opener )
        
#proxy = 'proxy.pd.intel.com:911'
#opener = urllib2.build_opener( urllib2.ProxyHandler({'http':proxy}) )
# #opener = urllib2.build_opener( urllib2.BaseHandler ) 
#urllib2.install_opener( opener )

def sendHttpRequest(url):
    setProxy()
    print "URL for last.fm: %s" % (url)
    website = urllib2.urlopen(url)
    
    return website



class ItemTestModel (object):
    """
    represent the item which is shown on myZone
    """
    
    def __init__(self, userName, serviceName):
        """
        Constructor for the ItemTestModel with 2 params,
        userName, the name of friends 
        trackUrl, the whole url of the recent track of the user
        """
        self.__artist = ""
        self.__artist_image_url = ""
        self.__date = now()
        self.__realName = ""
        self.__serviceName = ""
        self.__track = ""
        self.__track_album = ""
        self.__track_title = ""
        self.__trackImageUrl = ""
        self.set_user(userName)
        self.set_service_name (serviceName)
        self.set_date(time.time())
        
    def get_uuid(self):
        return (self.get_track() + " " + self.get_user()).encode('utf-8')
        
        
    def toString(self):
        uuid = self.track + " " + self.user
        str = "[%s] %s: Item %s from %s" % (now(), self.get_user(), uuid, self.serviceName)
        str += "\n Timestamp: %s" % self.date
        str += "\n album: %s" % self.track_album
        str += "\n title: %s" % self.track_title
        str += "\n cached: 1"
        str += "\n author: %s" % self.artist
        str += "\n authoricon: %s" % self.artist_image_url
        str += "\n url: %s" % self.track
        str += "\n authorid: %s" % self.realName
        str += "\n date: %s" % self.date
        str += "\n uuid: %s" % uuid
        str.encode('utf-8')
        return str
    
    
    def __clone(self):
        new_item = ItemTestModel(self.get_user(), self.get_service_name())
        new_item.set_user_image_url(self.get_user_image_url())
        new_item.set_real_name(self.get_real_name())
        
        return new_item
        
    
    def updateRecentTrack(self, apiKey):
        url = "http://ws.audioscrobbler.com/2.0/?method=user.getrecenttracks&limit=1&user=%s&api_key=%s&timestamp=%s" % (urllib.quote(self.get_user().encode('utf-8')), apiKey, time.time())
        website = sendHttpRequest(url)
        track_xml = website.read() 
        root = self.__parse(track_xml)
        
        tmp_items = []
        
        if("status" in root.keys() and "ok" == root.get("status")):
            recent_tracks = root[0]
            for track in recent_tracks:
                if(None != extractElementText("name", None, track)):
                    new_item = self.__clone()
                    new_item.set_artist(extractElementText("artist",None, track))
                    new_item.set_date(extractElementText("date", None, track))
                    new_item.set_track(extractElementText("url", None, track))
                    new_item.set_track_album(extractElementText("album", None, track))
                    new_item.set_track_image_url(extractElementText("image", "size='large'", track))
                    new_item.set_track_title(extractElementText("name", None, track))
                    
                    tmp_items.append(new_item)
                    print new_item.toString()
        
        return tmp_items
    
    def __updateArtistImageUrl(self, apiKey):
        url = "http://ws.audioscrobbler.com/2.0/?method=artist.getimages&artist=" + urllib.quote(self.get_artist().encode('utf-8')) + "&api_key=" + apiKey
        website = sendHttpRequest(url)
        image_xml = website.read()
        
        root = self.__parse(image_xml)
        texts = extractElementText("size", "name='large'", root)

        if( None != texts ):
            self.set_artist_image_url(texts)
        
        
    def __parse(self, xmlStr):
        try:
            root = etree.fromstring(xmlStr)
            return root
        except Exception, e:
            print e

    def get_service_name(self):
        return self.__serviceName


    def set_service_name(self, value):
        self.__serviceName = value


    def del_service_name(self):
        del self.__serviceName
        

    def get_real_name(self):
        return self.__realName


    def set_real_name(self, value):
        self.__realName = value


    def del_real_name(self):
        del self.__realName


    def get_artist_image_url(self):
        return self.__artist_image_url


    def set_artist_image_url(self, value):
        self.__artist_image_url = value


    def del_artist_image_url(self):
        del self.__artist_image_url


    def get_artist(self):
        return self.__artist


    def set_artist(self, value):
        self.__artist = value


    def del_artist(self):
        del self.__artist


    def get_user(self):
        return self.__user


    def get_track(self):
        return self.__track


    def get_track_image_url(self):
        return self.__trackImageUrl


    def get_user_image_url(self):
        return self.__userImageUrl


    def get_date(self):
        return self.__date


    def get_track_album(self):
        return self.__track_album


    def get_track_title(self):
        return self.__track_title


    def set_user(self, value):
        self.__user = value


    def set_track(self, value):
        self.__track = value


    def set_track_image_url(self, value):
        self.__trackImageUrl = value


    def set_user_image_url(self, value):
        self.__userImageUrl = value


    def set_date(self, value):
        self.__date = value


    def set_track_album(self, value):
        self.__track_album = value


    def set_track_title(self, value):
        self.__track_title = value


    def del_user(self):
        del self.__user


    def del_track(self):
        del self.__track


    def del_track_image_url(self):
        del self.__trackImageUrl


    def del_user_image_url(self):
        del self.__userImageUrl


    def del_date(self):
        del self.__date


    def del_track_album(self):
        del self.__track_album


    def del_track_title(self):
        del self.__track_title

    user = property(get_user, set_user, del_user, "userName, the name of friends")
    
    track = property(get_track, set_track, del_track, "trackUrl, the whole url of the recent track of the user")
    
    trackImageUrl = property(get_track_image_url, set_track_image_url, del_track_image_url, "The url of track's large image")
    
    userImageUrl = property(get_user_image_url, set_user_image_url, del_user_image_url, "the url of user's medium image")
    
    date = property(get_date, set_date, del_date, "time stamp")
    
    track_album = property(get_track_album, set_track_album, del_track_album, "The album which contain this song")
    
    track_title = property(get_track_title, set_track_title, del_track_title, "track's title")
    
    artist = property(get_artist, set_artist, del_artist, "artist's name")
    
    artist_image_url = property(get_artist_image_url, set_artist_image_url, del_artist_image_url, "the url of the artist 's large image")
    
    realName = property(get_real_name, set_real_name, del_real_name, "The real name of the user in last.fm")
    
    serviceName = property(get_service_name, set_service_name, del_service_name, "The corresponding service name")
        
    