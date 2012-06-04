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

PRJNAME=sal
TARGET=qa_osl_file

ENABLE_EXCEPTIONS=TRUE
VISIBILITY_HIDDEN=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+= $(LFS_CFLAGS)
CXXFLAGS+= $(LFS_CFLAGS)

# --- BEGIN --------------------------------------------------------
SHL1OBJS=  \
    $(SLO)$/osl_File.obj
SHL1TARGET= osl_File
SHL1STDLIBS= $(SALLIB) $(CPPUNITLIB)

SHL1IMPLIB= i$(SHL1TARGET)

DEF1NAME    =$(SHL1TARGET)
SHL1USE_EXPORTS = name

#-------------------------------------------------------------------

SHL2OBJS=$(SLO)$/test_cpy_wrt_file.obj
SHL2TARGET=tcwf
SHL2STDLIBS= $(SALLIB) $(CPPUNITLIB)
SHL2IMPLIB=i$(SHL2TARGET)
SHL2DEF=$(MISC)$/$(SHL2TARGET).def
SHL2USE_EXPORTS = name
DEF2NAME    =$(SHL2TARGET)


# END --------------------------------------------------------------

# --- BEGIN --------------------------------------------------------
SHL3OBJS=  \
    $(SLO)$/osl_old_test_file.obj
SHL3TARGET= osl_old_test_file
SHL3STDLIBS= $(SALLIB) $(CPPUNITLIB)

SHL3IMPLIB= i$(SHL3TARGET)

DEF3NAME    =$(SHL3TARGET)
SHL3USE_EXPORTS = name
# END --------------------------------------------------------------

#------------------------------- All object files -------------------------------
# do this here, so we get right dependencies
SLOFILES=$(SHL1OBJS) $(SHL2OBJS) $(SHL3OBJS)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
.INCLUDE : $(PRJ)$/qa$/cppunit_local.mk
