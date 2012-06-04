#*************************************************************************
#
#  OpenOffice.org - a multi-platform office productivity suite
#
#  The Contents of this file are made available subject to
#  the terms of GNU Lesser General Public License Version 3.
#
#
#    GNU Lesser General Public License Version 3
#    =============================================
#    Copyright 2005 by Sun Microsystems, Inc.
#    901 San Antonio Road, Palo Alto, CA 94303, USA
#
#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Lesser General Public
#    License version 2.1, as published by the Free Software Foundation.
#
#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Lesser General Public License for more details.
#
#    You should have received a copy of the GNU Lesser General Public
#    License along with this library; if not, write to the Free Software
#    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
#    MA  02111-1307  USA
#
#*************************************************************************

PRJ=..$/..

PRJNAME=scp2
TARGET=extensions
TARGETTYPE=CUI

#--- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

.IF "$(SYSTEM_BSH)" == "YES"
SCPDEFS+=-DSYSTEM_BSH -DBSH_JAR=\""file://$(BSH_JAR)"\"
.ENDIF

SCP_PRODUCT_TYPE=osl
PARFILES= \
        module_extensions.par                        \
        module_extensions_sun_templates.par          \
        directory_extensions.par                     \
        file_extensions.par

ULFFILES= \
        module_extensions.ulf                        \
        module_extensions_sun_templates.ulf

#--- File ---------------------------------------------------------
.INCLUDE :  target.mk
