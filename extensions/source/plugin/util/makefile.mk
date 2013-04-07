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
PRJ=..$/..$/..

PRJNAME=extensions
TARGET=pl
TARGETTYPE=GUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(WITH_MOZILLA)" != "NO"

LIB1TARGET = $(SLB)$/plall.lib
LIB1FILES  = \
    $(SLB)$/plbase.lib	\
    $(SHL1LINKLIB)

.IF "$(GUI)" == "UNX"
.IF "$(GUIBASE)"=="aqua"
.IF "$(WITH_MOZILLA)"=="YES"
SHL1LINKLIB = $(SLB)$/plaqua.lib
.ENDIF
.ELSE
SHL1LINKLIB = $(SLB)$/plunx.lib
.ENDIF # $(GUIBASE)==aqua
.IF "$(OS)" == "SOLARIS"
SHL1OWNLIBS = -lsocket
.ENDIF # SOLARIS
.ENDIF # UNX

.IF "$(GUI)" == "WNT"
SHL1LINKLIB = $(SLB)$/plwin.lib
SHL1OWNLIBS = \
    $(VERSIONLIB)	\
    $(OLE32LIB)	\
    $(ADVAPI32LIB)
.ENDIF # WNT

SHL1TARGET= $(TARGET)$(DLLPOSTFIX)
SHL1IMPLIB= i$(TARGET)

SHL1VERSIONMAP=$(SOLARENV)/src/component.map
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)

SHL1LIBS=$(LIB1TARGET)

SHL1STDLIBS+= \
    $(TKLIB)            \
    $(VCLLIB)			\
    $(SVLLIB)			\
    $(UNOTOOLSLIB)      \
    $(TOOLSLIB)			\
    $(UCBHELPERLIB)		\
    $(CPPUHELPERLIB)	\
    $(CPPULIB)			\
    $(SALLIB)           \
    $(SALHELPERLIB)

.IF "$(GUIBASE)"=="unx"
SHL1STDLIBS+=$(X11LINK_DYNAMIC)
.ENDIF

SHL1STDLIBS+=$(SHL1OWNLIBS)

.ENDIF # $(WITH_MOZILLA) != "NO"

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk




ALLTAR : $(MISC)/pl.component

$(MISC)/pl.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        pl.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt pl.component
