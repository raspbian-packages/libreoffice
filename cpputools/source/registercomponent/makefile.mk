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
PRJ=..$/..

PRJNAME=cpputools
TARGET=regcomp
LIBTARGET=NO

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------
.INCLUDE :  settings.mk

.IF "$(OS)" == "IOS"
ALL:
    @echo Nothing done for $(OS)
.ENDIF

UNOUCRDEP=$(SOLARBINDIR)$/udkapi.rdb 
UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb

NO_OFFUH=TRUE
CPPUMAKERFLAGS += -C

UNOTYPES=\
             com.sun.star.uno.TypeClass \
             com.sun.star.lang.XMultiServiceFactory \
             com.sun.star.lang.XSingleServiceFactory \
             com.sun.star.lang.XMultiComponentFactory \
             com.sun.star.lang.XSingleComponentFactory \
             com.sun.star.lang.XComponent \
             com.sun.star.container.XContentEnumerationAccess \
             com.sun.star.container.XSet \
             com.sun.star.loader.CannotActivateFactoryException \
            com.sun.star.registry.XImplementationRegistration2

# --- Files --------------------------------------------------------

DEPOBJFILES=   $(OBJ)$/registercomponent.obj 

APP1TARGET= $(TARGET)
APP1OBJS=$(DEPOBJFILES)  
APP1RPATH=UREBIN

APP1STDLIBS=\
            $(SALLIB) \
            $(CPPULIB)	\
            $(CPPUHELPERLIB)

.IF "$(GUI)"=="WNT"
APP1STDLIBS+= \
            $(LIBCMT)
.ENDIF


.INCLUDE :  target.mk
