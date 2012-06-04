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
PRJ=..

PRJNAME=cppu
TARGET=cppu
USE_DEFFILE=TRUE
ENABLE_EXCEPTIONS=TRUE
NO_BSYMBOLIC=TRUE

.IF "$(COM)" != "MSC"
UNIXVERSIONNAMES=UDK
.ENDIF


# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE : ../source/helper/purpenv/export.mk

# --- Files --------------------------------------------------------

SHL1LIBS= \
    $(SLB)$/cppu_typelib.lib	\
    $(SLB)$/cppu_uno.lib		\
    $(SLB)$/cppu_threadpool.lib	\
    $(SLB)$/cppu_cppu.lib

.IF "$(COM)" == "MSC"
SHL1TARGET=$(TARGET)$(UDK_MAJOR)
.ELSE
SHL1TARGET= uno_$(TARGET)
.ENDIF

SHL1STDLIBS = $(SALLIB)

SHL1DEPN=
.IF "$(COM)" == "MSC"
SHL1IMPLIB=i$(TARGET)
.ELSE
SHL1IMPLIB=uno_$(TARGET)
.ENDIF
.IF "$(OS)"!="FREEBSD" && "$(OS)"!="NETBSD" && "$(OS)"!="OPENBSD" && "$(OS)"!="DRAGONFLY"
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
.ENDIF

SHL1VERSIONMAP=$(TARGET).map
SHL1RPATH=URELIB

DEF1NAME=$(SHL1TARGET)


SHL2TARGET  := $(NAMEpurpenv_helper)
DEF2NAME    := $(SHL2TARGET)
.IF "$(GUI)$(COM)"=="WNTGCC"
SHL2VERSIONMAP:=uno_purpenvhelper$(COMID).map
.ELIF "$(COMNAME)"=="mscx"
SHL2VERSIONMAP:=$(SHL2TARGET)X.map
.ELSE
SHL2VERSIONMAP:=$(SHL2TARGET).map
.ENDIF			# "$(GUI)$(COM)"=="WNTGCC"
SHL2DEF     := $(MISC)$/$(SHL2TARGET).def
.IF "$(COM)" == "MSC"
SHL2IMPLIB  := i$(SHL2TARGET)
.ELSE
SHL2IMPLIB  := $(SHL2TARGET)
.ENDIF
SHL2STDLIBS := $(CPPULIB) $(SALLIB)
SHL2RPATH   := URELIB
SHL2OBJS    := \
    $(SLO)$/helper_purpenv_Environment.obj 	\
    $(SLO)$/helper_purpenv_Mapping.obj      \
    $(SLO)$/helper_purpenv_Proxy.obj
SHL2DEPN=$(SHL1TARGETN)

# --- Targets ------------------------------------------------------
.INCLUDE :	target.mk

ALLTAR:
	$(MAKE) $(MAKECMDGOALS) -f extra.mk
