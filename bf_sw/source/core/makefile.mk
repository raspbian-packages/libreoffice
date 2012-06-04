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
BFPRJ=..$/..

PRJNAME=binfilter
TARGET=sw_core

NO_HIDS=TRUE

# --- Settings -----------------------------------------------------

.INCLUDE :  $(PRJ)$/inc$/bf_sw$/swpre.mk
.INCLUDE :  settings.mk
.INCLUDE :  $(PRJ)$/inc$/bf_sw$/sw.mk

INC+= -I$(PRJ)$/inc$/bf_sw
.IF "$(CALLTARGETS)"=="core"
RC_SUBDIRS=
.ENDIF

# --- Files --------------------------------------------------------

SUBLIBS1= \
        $(SLB)$/sw_graphic.lib \
        $(SLB)$/sw_para.lib \
        $(SLB)$/sw_attr.lib \
        $(SLB)$/sw_crsr.lib \
        $(SLB)$/sw_view.lib \
        $(SLB)$/sw_frmedt.lib \
        $(SLB)$/sw_ole.lib \
        $(SLB)$/sw_fields.lib \
        $(SLB)$/sw_tox.lib \
        $(SLB)$/sw_bastyp.lib

SUBLIBS2= \
        $(SLB)$/sw_draw.lib \
        $(SLB)$/sw_sw3io.lib \
        $(SLB)$/sw_swg.lib \
        $(SLB)$/sw_layout.lib \
        $(SLB)$/sw_text.lib \
        $(SLB)$/sw_txtnode.lib \
        $(SLB)$/sw_doc.lib \
        $(SLB)$/sw_docnode.lib \
        $(SLB)$/sw_unocore.lib

#-------------------------------------------------------------------------

################################################################

LIB1TARGET=$(SLB)$/sw_core1.lib
LIB1FILES= \
        $(SUBLIBS1)

LIB2TARGET=$(SLB)$/sw_core2.lib
LIB2FILES= \
        $(SUBLIBS2)

.INCLUDE :  target.mk

################################################################

#-------------------------------------------------------------------------


