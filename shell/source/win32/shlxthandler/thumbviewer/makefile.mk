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

PRJ=..$/..$/..$/..
PRJNAME=shell
TARGET=thumbviewer
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

CFLAGS+=-DWIN32_LEAN_AND_MEAN -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CDEFS+=-D_WIN32_IE=0x501
.IF "$(COM)" == "GCC"
CDEFS+=-DDONT_HAVE_GDIPLUS
.ENDIF

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/$(TARGET).obj

.IF "$(BUILD_X64)"!=""
# ----------------- x64 ------------------------
CFLAGS_X64+=-DWIN32_LEAN_AND_MEAN -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
CDEFS_X64+=-D_WIN32_IE=0x501

SLOFILES_X64=$(SLO_X64)$/$(TARGET).obj
.ENDIF # "$(BUILD_X64)"!=""


# --- Targets ------------------------------------------------------

.INCLUDE :	set_wntx64.mk
.INCLUDE :	target.mk
INCLUDE!:=$(subst,/stl, $(INCLUDE))
 
.INCLUDE :	tg_wntx64.mk

