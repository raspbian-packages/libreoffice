
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
PRJNAME=lingucomponent
TARGET=guesslang
ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

#----- Settings ---------------------------------------------------------

.INCLUDE : settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
.IF "$(SYSTEM_LIBEXTTEXTCAT)" == "YES"
LIBTEXTCATLIB=$(LIBEXTTEXTCAT_LIBS)
.ELSE
LIBTEXTCATLIB=-lexttextcat
.ENDIF
.ELSE               # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"
.IF "$(COM)"=="GCC"
LIBTEXTCATLIB=-lilibexttextcat
.ELSE
LIBTEXTCATLIB=ilibexttextcat.lib
.ENDIF
.ENDIF  # "$(GUI)"=="UNX" || "$(GUI)"=="MAC"

SLOFILES = \
        $(SLO)$/altstrfunc.obj \
        $(SLO)$/guess.obj \
        $(SLO)$/guesslang.obj \
        $(SLO)$/simpleguesser.obj


SHL1TARGET= $(TARGET)$(DLLPOSTFIX)

SHL1STDLIBS= \
        $(CPPUHELPERLIB)        \
        $(CPPULIB)              \
        $(LIBTEXTCATLIB)        \
        $(SALLIB)               \
        $(SVLLIB)               \
        $(TOOLSLIB)             \
        $(UNOTOOLSLIB)

# build DLL
.IF "$(GUI)"=="WNT"
SHL1VERSIONMAP=$(SOLARENV)/src/component.map
.ELSE # just a quick hack for GCC fdo#42865
SHL1USE_EXPORTS = name
.ENDIF
SHL1LIBS=       $(SLB)$/$(TARGET).lib
SHL1IMPLIB=     i$(TARGET)
SHL1DEPN=       $(SHL1LIBS)
SHL1DEF=        $(MISC)$/$(SHL1TARGET).def

# build DEF file
DEF1NAME    =$(SHL1TARGET)
DEF1EXPORTFILE= exports.dxp

# --- Targets ------------------------------------------------------

.INCLUDE : target.mk


ALLTAR : $(MISC)/guesslang.component

$(MISC)/guesslang.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        guesslang.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt guesslang.component
