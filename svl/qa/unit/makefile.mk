# -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Novell, Inc.
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Contributor(s):
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#

PRJ=..$/..
PRJNAME=svl
TARGET=qa_unit

ENABLE_EXCEPTIONS=TRUE

.INCLUDE : settings.mk

CFLAGSCXX += $(CPPUNIT_CFLAGS) -I../../inc

SHL1TARGET = $(TARGET)
SHL1OBJS = $(SLO)$/svl.obj
SHL1STDLIBS= \
    $(UNOTOOLSLIB)    \
    $(SVLLIB)         \
    $(TOOLSLIB)       \
    $(I18NISOLANGLIB) \
    $(UCBHELPERLIB)   \
    $(COMPHELPERLIB)  \
    $(CPPUHELPERLIB)  \
    $(CPPULIB)        \
    $(SOTLIB)         \
    $(SALLIB)         \
    $(CPPUNITLIB)

SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)
SHL1VERSIONMAP=version.map

.INCLUDE: target.mk

.IF "$(OS)" == "WNT"
my_file = file:///
.ELSE
my_file = file://
.END

ALLTAR: test

$(MISC)$/$(TARGET)$/types.rdb .ERRREMOVE : $(SOLARBINDIR)$/types.rdb
    $(MKDIRHIER) $(@:d)
    $(GNUCOPY) $? $@

$(MISC)/$(TARGET)/udkapi.rdb .ERRREMOVE : $(SOLARBINDIR)$/udkapi.rdb
    $(MKDIRHIER) $(@:d)
    $(GNUCOPY) $? $@

#Make a services.rdb with the services we know we need to get up and running
$(MISC)/$(TARGET)/services.rdb .ERRREMOVE : $(MISC)/$(TARGET)/udkapi.rdb makefile.mk
    $(MKDIRHIER) $(@:d)
    $(REGCOMP) -register -br $(MISC)/$(TARGET)/udkapi.rdb -r $@ -wop \
        -c i18npool.uno$(DLLPOST)

#Tweak things so that we use the .res files in the solver
STAR_RESOURCEPATH:=$(PWD)/$(BIN)$(PATH_SEPERATOR)$(SOLARBINDIR)
.EXPORT : STAR_RESOURCEPATH

test .PHONY: $(SHL1TARGETN) $(MISC)/$(TARGET)/services.rdb $(MISC)$/$(TARGET)$/types.rdb $(MISC)/$(TARGET)/udkapi.rdb
    @echo ----------------------------------------------------------
    @echo - start unit test \#1 on library $(SHL1TARGETN)
    @echo In the environment: STAR_RESOURCEPATH=$$STAR_RESOURCEPATH
    @echo ----------------------------------------------------------
    $(CPPUNITTESTER) $(SHL1TARGETN) \
        -env:UNO_SERVICES=$(my_file)$(PWD)/$(MISC)/$(TARGET)/services.rdb \
        -env:UNO_TYPES="$(my_file)$(PWD)/$(MISC)/$(TARGET)/types.rdb $(my_file)$(PWD)/$(MISC)/$(TARGET)/udkapi.rdb" \
        -env:BRAND_BASE_DIR="$(my_file)$(PWD)/$(MISC)/$(TARGET)" \
        -env:UNO_USER_PACKAGES_CACHE="$(my_file)$(PWD)/$(MISC)/$(TARGET)"
