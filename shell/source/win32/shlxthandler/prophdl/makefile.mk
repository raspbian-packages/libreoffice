#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2010 by Sun Microsystems, Inc.
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
TARGET=propertyhdl
LIBTARGET=NO
ENABLE_EXCEPTIONS=TRUE
NO_DEFAULT_STL=YES
USE_DEFFILE=TRUE
.IF "$(BUILD_X64)"!=""
USE_DEFFILE_X64=TRUE
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

.IF "$(COM)" == "GCC"
ALL :
    @echo This does not build with MinGW
.ENDIF


UWINAPILIB =
UWINAPILIB_X64 =

CFLAGS+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
.IF "$(COM)" == "MSC"
CFLAGS+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820 -wd4100
.ENDIF
CDEFS+=-D_WIN32_IE=0x501

# --- Files --------------------------------------------------------

SLOFILES=$(SLO)$/$(TARGET).obj\
    $(SLO)$/stream_helper.obj

SHL1TARGET=$(TARGET)

.IF "$(COM)"=="GCC"
SHL1STDLIBS=$(ZLIB3RDLIB) $(EXPAT3RDLIB) $(COMCTL32LIB)
SHL1LIBS=
.ELSE
SHL1STDLIBS=
SHL1LIBS=$(SOLARLIBDIR)$/zlib.lib\
    $(SOLARLIBDIR)$/expat_xmlparse.lib\
    $(SOLARLIBDIR)$/expat_xmltok.lib
.ENDIF
SHL1STDLIBS+=\
    $(OLEAUT32LIB)\
    $(ADVAPI32LIB)\
    $(OLE32LIB)\
    $(UUIDLIB)\
    $(SHELL32LIB)\
    $(KERNEL32LIB)\
    $(GDI32LIB)\
    $(GDIPLUSLIB)\
    $(SHLWAPILIB)\
    $(PROPSYSLIB)

.IF "$(COM)"!="GCC"
SHL1STDLIBS+=\
    msvcprt.lib

.IF "$(PRODUCT)"!="full"
SHL1STDLIBS+=msvcrt.lib
.ENDIF
.ENDIF

SHL1LIBS+=$(SLB)$/util.lib\
    $(SLB)$/ooofilereader.lib

SHL1DEPN=
SHL1OBJS=$(SLOFILES)
SHL1DEF=$(MISC)$/$(SHL1TARGET).def
DEF1NAME=$(SHL1TARGET)
DEF1EXPORTFILE=exports.dxp

# -------------------- x64 -----------------------
.IF "$(BUILD_X64)"!=""
CFLAGS_X64+=-DISOLATION_AWARE_ENABLED -DWIN32_LEAN_AND_MEAN -DXML_UNICODE -D_NTSDK -DUNICODE -D_UNICODE -D_WIN32_WINNT=0x0501
.IF "$(COM)" == "MSC"
CFLAGS_X64+=-wd4710 -wd4711 -wd4514 -wd4619 -wd4217 -wd4820
.ENDIF
CDEFS_X64+=-D_WIN32_IE=0x501
SLOFILES_X64=$(SLO_X64)$/$(TARGET).obj

SLOFILES_X64= \
    $(SLO_X64)$/propertyhdl.obj\
    $(SLO_X64)$/stream_helper.obj\

SHL1TARGET_X64=$(TARGET)
SHL1LIBS_X64=$(SOLARLIBDIR_X64)$/zlib.lib\
    $(SOLARLIBDIR_X64)$/expat_xmlparse.lib\
    $(SOLARLIBDIR_X64)$/expat_xmltok.lib

SHL1STDLIBS_X64+=\
    $(OLEAUT32LIB_X64)\
    $(ADVAPI32LIB_X64)\
    $(OLE32LIB_X64)\
    $(UUIDLIB_X64)\
    $(SHELL32LIB_X64)\
    $(KERNEL32LIB_X64)\
    $(GDI32LIB_X64)\
    $(USER32LIB_X64) \
    $(GDIPLUSLIB_X64) \
    $(MSVCRT_X64)   \
    $(MSVCPRT_X64)  \
    $(OLDNAMESLIB_X64)\
    $(PROPSYSLIB_X64)

SHL1LIBS_X64+=$(SLB_X64)$/util.lib\
    $(SLB_X64)$/ooofilereader.lib
SHL1OBJS_X64=$(SLOFILES_X64)
SHL1DEF_X64=$(MISC_X64)$/$(SHL1TARGET).def

DEF1NAME_X64=$(SHL1TARGET_X64)
DEF1EXPORTFILE_X64=exports.dxp

.ENDIF # "$(BUILD_X64)"!=""

# --- Targets ------------------------------------------------------
.INCLUDE :  set_wntx64.mk
.INCLUDE :	target.mk
INCLUDE!:=$(subst,/stl, $(INCLUDE))

.INCLUDE :  tg_wntx64.mk
