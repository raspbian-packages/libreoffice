#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2010 Oracle and/or its affiliates.
#
# OpenOffice.org - a multi-platform office productivity suite
#
# This file is part of OpenOffice.org.
#
# OpenOffice.org is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License version 3
# only, as published by the Free Software Foundation.
#
# OpenOffice.org is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.  If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

PRJ := ..
PRJNAME := cppu
TARGET := qa

ENABLE_EXCEPTIONS := TRUE
VISIBILITY_HIDDEN=TRUE

.INCLUDE: settings.mk

CFLAGSCXX+=$(CPPUNIT_CFLAGS)

.IF "$(OS)" == "IOS"
CFLAGSCXX += $(OBJCXXFLAGS)
.ENDIF

DLLPRE=# no leading "lib" on .so files

INCPRE+=$(MISC)$/$(TARGET)$/inc

SHL1TARGET = $(TARGET)_any
SHL1OBJS = $(SLO)$/test_any.obj
SHL1STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(SALLIB)
SHL1USE_EXPORTS = name
SHL1IMPLIB = i$(SHL1TARGET)
DEF1NAME = $(SHL1TARGET)

SHL2TARGET = $(TARGET)_unotype
SHL2OBJS = $(SLO)$/test_unotype.obj
SHL2STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(SALLIB)
SHL2USE_EXPORTS = name
SHL2IMPLIB = i$(SHL2TARGET)
DEF2NAME = $(SHL2TARGET)

SHL3TARGET = $(TARGET)_reference
SHL3OBJS = $(SLO)$/test_reference.obj
SHL3STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(SALLIB)
SHL3USE_EXPORTS = name
SHL3IMPLIB = i$(SHL3TARGET)
DEF3NAME = $(SHL3TARGET)

SHL4TARGET = $(TARGET)_recursion
SHL4OBJS = $(SLO)$/test_recursion.obj
SHL4STDLIBS = $(CPPULIB) $(CPPUNITLIB) $(SALLIB)
SHL4USE_EXPORTS = name
SHL4IMPLIB = i$(SHL4TARGET)
DEF4NAME = $(SHL4TARGET)

SLOFILES = $(SHL1OBJS) $(SHL2OBJS) $(SHL3OBJS) $(SHL4OBJS)

.IF "$(OS)" == "IOS no thanks for now" 
# Nah, we can't build this here for iOS after all, thanks to having to
# use static linking, we would need to link with -lgcc3_uno from
# bridges which has not been built yet (and which in fact depends on
# cppu...) Seems that any meaningful unit test for iOS actually needs
# to be built in the "subsequent" stage.
APP5OBJS = $(OBJ)/cppu_cppunittester_all.obj $(SHL1OBJS) $(SHL2OBJS) $(SHL3OBJS) $(SHL4OBJS)
APP5RPATH = NONE
APP5STDLIBS = $(CPPUNITLIB) $(CPPULIB) $(SALLIB)
APP5TARGET = cppu_cppunittester_all
.ENDIF

.INCLUDE: target.mk

$(SHL1OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(SHL2OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(SHL3OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(SHL4OBJS): $(MISC)$/$(TARGET).cppumaker.flag

$(MISC)$/$(TARGET).cppumaker.flag: $(MISC)$/$(TARGET).rdb
    - $(MKDIRHIER) $(MISC)$/$(TARGET)$/inc
    $(CPPUMAKER) -O$(MISC)$/$(TARGET)$/inc -BUCR -C $< \
        $(SOLARBINDIR)$/udkapi.rdb
    $(TOUCH) $@

$(MISC)$/$(TARGET).rdb: $(MISC)$/$(TARGET)$/types.urd
    - $(RM) $@
    $(REGMERGE) $@ /UCR $<

$(MISC)$/$(TARGET)$/types.urd: types.idl
    - $(MKDIR) $(MISC)$/$(TARGET)
    $(IDLC) -O$(MISC)$/$(TARGET) -I$(SOLARIDLDIR) -cid -we $<

.INCLUDE : _cppunit.mk
