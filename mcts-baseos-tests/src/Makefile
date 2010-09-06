#
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
#       Wang,Jing  <jing.j.wang@intel.com>
#
#
# Date Created: 02-12--2010
#
# Modifications:
#          | Modificator | Date | Content of Modification |
#          | Modificator | Date | Content of Modification |
#
# Example Makefile for MID project.
# It does most of the work for you, but may require further modificationi.
#
#<variables settings>
#
# Common Variables.
RELA_DIR=$(subst $(STF_ROOT),,$(shell pwd)) 
TEST_DIR=$(INSTALL_DIR)$(RELA_DIR)
METAFILES=Makefile testinfo.xml 

# Src code, scripts anything needed to compile the test.
FILES=$(METAFILES) *.sh helper startup 

# FIXME: you can add anything needed to compile the test.
# CFLAG = 
# LDFLAGS = 
BUILT_FILES=

# mandatory targets
all: build

build: $(BUILT_FILES)

install: $(FILES)
	mkdir -p $(TEST_DIR)
	cp -a $(FILES) $(TEST_DIR)

uninstall:
	rm -rf $(TEST_DIR)

clean:
	rm -f *~ $(BUILT_FILES)
