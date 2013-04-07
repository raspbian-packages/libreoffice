#*************************************************************************
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
#***********************************************************************/

PRJ = ../..
PRJNAME = unotest
TARGET = cpp

ENABLE_EXCEPTIONS = TRUE
VISIBILITY_HIDDEN = TRUE

.INCLUDE: settings.mk

# --- Files --------------------------------------------------------
CDEFS += -DOOO_DLLIMPLEMENTATION_UNOTEST

CFLAGSCXX += $(CPPUNIT_CFLAGS)

SLOFILES = \
    $(SLO)/bootstrapfixturebase.obj \
    $(SLO)/filters-test.obj \
    $(SLO)/getargument.obj \
    $(SLO)/gettestargument.obj \
    $(SLO)/macros_test.obj \
    $(SLO)/officeconnection.obj \
    $(SLO)/toabsolutefileurl.obj \
    $(SLO)/uniquepipename.obj

.IF "$(CROSS_COMPILING)" == "YES"
SHL1IMPLIB = $(SHL1TARGET)
.ELSE
SHL1IMPLIB = i$(SHL1TARGET)
.ENDIF
SHL1OBJS = $(SLOFILES)
SHL1RPATH = NONE
SHL1STDLIBS = \
    $(COMPHELPERLIB) \
    $(CPPUHELPERLIB) \
    $(CPPULIB) \
    $(CPPUNITLIB) \
    $(SALLIB)
SHL1TARGET = unotest
SHL1USE_EXPORTS = name
DEF1NAME = $(SHL1TARGET)

.INCLUDE: target.mk
