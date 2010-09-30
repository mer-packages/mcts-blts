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

import flickrapi, sys
from flickr_item import FlickrItemModel, creatFlickrItem

reload(sys)
sys.setdefaultencoding("utf-8")

FLICKR_API_KEY = "bad5a5800603ede4c188a720b9e96659"
FLICKR_API_SECRET = "702c4adab14ca13c"

class FlickrZoneModel(object):
    '''
    represent the 'flickr' item list which is shown on myZone
    '''

    def __init__(self, apiKey=FLICKR_API_KEY, apiSecret=FLICKR_API_SECRET):
        """
        Constructor for the item list which is shown on myZone with 3 params,
        user, the name of the user who is using Meego 
        apiKey, the key for invoking flickr APIs
        """
        #self.user = user
        self.items = {}
        
        api_key = apiKey
        api_secret = apiSecret
        
        self.flickr = flickrapi.FlickrAPI(api_key, api_secret) #, username=self.user
        
        (token, frob) = self.flickr.get_token_part_one(perms='write')
        if not token: 
            raw_input("Press ENTER after you authorized this program")
        self.flickr.get_token_part_two((token, frob))

        
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

    def set_user(self, value):
        self.__user = value


    #user = property(get_user, set_user, None, "user, the name of the user who is using Meego")
    items = property(get_items, set_items, del_items, "Items List")
    
    def query(self, query, text=None, tags=None, licenses=None):
        """
        query, which type of ItemView query. For now, the possible value is 'own', 'feed', "friends-only", and "x-flickr-search"
        """
        print FlickrZoneModel.query.__doc__
        if(FlickrZoneModel.QUERY_TYPE_METHOD.__contains__(query)):
            return FlickrZoneModel.QUERY_TYPE_METHOD[query](self)
        elif ('x-flickr-search' == query):
            self.queryXSearch(text, tags, licenses)
        else:
            return False
    
    def parseResponse(self, response):
        if(None != response):
            print "Fetching the response from Flickr"
            for photos in response:
                print "%d photos are fetched" % len(photos)
                for photo in photos:
                    item = creatFlickrItem(photo)
                    if(None != item and None != item.id):
                        self.items[item.id] = item
                        print "Create a new item (ID: %s" % item.id
                print "%d photos are fetched" % len(self.items)
            return True
        else:
            print "Fail to get the photos from Flickr"
            return False
    
    def queryOwn(self):
        print "Preparing to query Flickr API 'flickr.people.getPhotos' with user_id='me'"
        print "For more details, http://www.flickr.com/services/api/flickr.people.getPhotos.html"
        response = self.flickr.people_getPhotos(count='50', user_id='me',
                extras='date_upload,icon_server,geo,url_m,url_l,url_o')
        return self.parseResponse(response)
    
    def photos_getContactsPhotos(self, include_self):
        print "For more details, http://www.flickr.com/services/api/flickr.photos.getContactsPhotos.html"
        response = self.flickr.photos_getContactsPhotos(count='50', 
                extras='date_upload,icon_server,geo,url_m,url_l,url_o',
                include_self = include_self)
        
        return self.parseResponse(response)
    
    def queryFeed(self):
        print "Preparing to query Flickr API 'flickr.photos.getContactsPhotos' with include_self=1"
        return self.photos_getContactsPhotos(include_self = '1')
        
    def queryOnlyFriends(self):
        print "Preparing to query Flickr API 'flickr.photos.getContactsPhotos' with include_self=0"
        return self.photos_getContactsPhotos(include_self = '0')
    
    def queryXSearch(self, text=None, tags=None, licenses=None):
        print "Preparing to query Flickr API 'flickr.photos.search' with the filter params \
            (text = %s,  tags=%s, licenses=%s)" % (text, tags, licenses)
        print "For more details, http://www.flickr.com/services/api/flickr.photos.search.html"
        
        response = self.flickr.photos_search(count='50', 
                extras='date_upload,icon_server,geo,url_m,url_l,url_o',
                text = text, tags=tags, licenses=licenses)
        return self.parseResponse(response)
        
    
    QUERY_TYPE_METHOD = {'own':queryOwn, 'feed': queryFeed, 
                         'friends-only': queryOnlyFriends}
        