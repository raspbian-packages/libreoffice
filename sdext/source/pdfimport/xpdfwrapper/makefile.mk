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

PRJNAME=sdext
TARGET=xpdfimport
TARGETTYPE=CUI
ENABLE_EXCEPTIONS=TRUE
EXTERNAL_WARNINGS_NOT_ERRORS := TRUE

# --- Settings -----------------------------------------------------

.INCLUDE: settings.mk

.IF "$(SYSTEM_ZLIB)" == "YES"
CFLAGS+=-DSYSTEM_ZLIB
.ENDIF

.IF "$(ENABLE_PDFIMPORT)" == "NO"
@all:
    @echo "PDF Import extension disabled."
.ENDIF

.IF "$(SYSTEM_POPPLER)" == "YES"
CFLAGS += $(POPPLER_CFLAGS) -DSYSTEM_POPPLER
.ELSE
CFLAGS += -I$(SOLARINCDIR)$/xpdf
.ENDIF

# --- Files --------------------------------------------------------
UWINAPILIB:=
APP1TARGET=$(TARGET)
APP1LIBSALCPPRT=
APP1OBJS= \
    $(OBJ)$/wrapper_gpl.obj $(OBJ)/pdfioutdev_gpl.obj $(OBJ)/pnghelper.obj

APP1STDLIBS+=$(ZLIB3RDLIB)

.IF "$(SYSTEM_POPPLER)" == "YES"
APP1STDLIBS+=$(POPPLER_LIBS)
.ELSE
.IF "$(GUI)" == "WNT"
.IF "$(COM)"=="GCC"
APP1STDLIBS+=-lxpdf -lfofi -lGoo -lgdi32 -ladvapi32
.ELSE
APP1STDLIBS+=xpdf.lib fofi.lib Goo.lib gdi32.lib advapi32.lib
.ENDIF
.ELSE
.IF "$(OS)" == "MACOSX" && "$(GUIBASE)"=="unx"
APP1STDLIBS+=-lobjc
.ENDIF
APP1STDLIBS+=-lxpdf -lfofi -lGoo
.ENDIF
.ENDIF
# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk
