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

PRJ=.

PRJNAME=external
TARGET=AppleRemote

ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUIBASE)"!="aqua"

dummy:
    @echo "Nothing to build for GUIBASE $(GUIBASE)"

.ELSE		# "$(GUIBASE)"!="aqua"

SHL1STDLIBS+= \
    -framework Cocoa -framework Carbon -framework IOKit

LIB1FILES+= \
        $(SLB)$/AppleRemote.lib

SLOFILES=	\
        $(SLO)$/KeyspanFrontRowControl.obj 	\
        $(SLO)$/AppleRemote.obj 		\
        $(SLO)$/RemoteControl.obj		\
        $(SLO)$/RemoteControlContainer.obj	\
        $(SLO)$/GlobalKeyboardDevice.obj	\
        $(SLO)$/HIDRemoteControlDevice.obj	\
        $(SLO)$/MultiClickRemoteBehavior.obj	\
        $(SLO)$/RemoteMainController.obj

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1OBJS= $(SLOFILES)

OUT2INC = \
        $(BUILDDIR)$/KeyspaFrontRowControl.h	\
        $(BUILDDIR)$/AppleRemote.h		\
        $(BUILDDIR)$/RemoteControl.h		\
        $(BUILDDIR)$/RemoteControlContainer.h	\
        $(BUILDDIR)$/GlobalKeyboardDevice.h	\
        $(BUILDDIR)$/HIDRemoteControlDevice.h	\
        $(BUILDDIR)$/MultiClickRemoteBehavior.h	\
        $(BUILDDIR)$/RemoteMainController.h


.ENDIF		# "$(GUIBASE)"!="aqua"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
