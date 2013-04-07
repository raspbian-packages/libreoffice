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

PRJNAME=embedserv
TARGET=emser
LIBTARGET=NO
USE_DEFFILE=TRUE

# --- Settings -----------------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

.IF "$(GUI)" == "WNT" && "$(DISABLE_ATL)"==""

SHL1TARGET= emser$(DLLPOSTFIX)

SHL1IMPLIB= emserimp
SHL1OBJS= \
        $(SLO)$/register.obj \
        $(SLO)$/servprov.obj \
        $(SLO)$/docholder.obj \
        $(SLO)$/ed_ipersiststr.obj \
        $(SLO)$/ed_idataobj.obj \
        $(SLO)$/ed_ioleobject.obj \
        $(SLO)$/ed_iinplace.obj \
        $(SLO)$/iipaobj.obj \
        $(SLO)$/guid.obj \
        $(SLO)$/esdll.obj \
        $(SLO)$/intercept.obj \
        $(SLO)$/syswinwrapper.obj \
        $(SLO)$/tracker.obj

SHL1STDLIBS=\
        $(SALLIB)			\
        $(CPPULIB)			\
        $(CPPUHELPERLIB)	\
        $(OLE32LIB)			\
        $(GDI32LIB)			\
        $(UUIDLIB)				\
        $(OLEAUT32LIB)

.IF "$(COM)"=="MSC"
SHL1STDLIBS+=\
        $(ADVAPI32LIB)
.IF "$(USE_DEBUG_RUNTIME)" == ""
SHL1STDLIBS+=\
        $(ATL_LIB)$/atls.lib
.ELSE
SHL1STDLIBS+=\
        $(ATL_LIB)$/atlsd.lib
.ENDIF
.ENDIF # "$(COM)"=="MSC"


SHL1DEF=		$(MISC)$/$(SHL1TARGET).def

DEF1NAME=		$(SHL1TARGET)
DEF1EXPORTFILE=	exports.dxp

.ENDIF

# --- Targets -------------------------------------------------------------

.INCLUDE :  target.mk



ALLTAR : $(MISC)/emser.component

$(MISC)/emser.component .ERRREMOVE : $(SOLARENV)/bin/createcomponent.xslt \
        emser.component
    $(XSLTPROC) --nonet --stringparam uri \
        '$(COMPONENTPREFIX_BASIS_NATIVE)$(SHL1TARGETN:f)' -o $@ \
        $(SOLARENV)/bin/createcomponent.xslt emser.component
