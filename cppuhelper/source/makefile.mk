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

PRJNAME=cppuhelper
TARGET=cppuhelper

ENABLE_EXCEPTIONS=TRUE
USE_DEFFILE=TRUE

# not strictly a bootstrap service but containing
# bootstrap code that may require generated files
# without "-L" (light) switch
BOOTSTRAP_SERVICE=TRUE

.IF "$(COM)" != "MSC"
UNIXVERSIONNAMES=UDK
.ENDIF

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

# --- Files --------------------------------------------------------

UNOUCRRDB=$(SOLARBINDIR)$/udkapi.rdb
UNOUCRDEP=$(UNOUCRRDB)
UNOUCROUT=$(OUT)$/inc$/$(TARGET)
INCPRE+=$(OUT)$/inc$/$(TARGET) $(OUT)$/inc$/private

CPPUMAKERFLAGS= -C

UNOTYPES= \
        com.sun.star.beans.PropertyAttribute \
        com.sun.star.beans.PropertyValue \
        com.sun.star.beans.XFastPropertySet \
        com.sun.star.beans.XMultiPropertySet \
        com.sun.star.beans.XPropertyAccess \
        com.sun.star.beans.XPropertySet \
        com.sun.star.beans.XPropertySetOption \
        com.sun.star.bridge.UnoUrlResolver \
        com.sun.star.bridge.XUnoUrlResolver \
        com.sun.star.connection.SocketPermission \
        com.sun.star.container.XElementAccess \
        com.sun.star.container.XEnumerationAccess \
        com.sun.star.container.XHierarchicalNameAccess \
        com.sun.star.container.XNameAccess \
        com.sun.star.container.XNameContainer \
        com.sun.star.container.XSet \
        com.sun.star.io.FilePermission \
        com.sun.star.io.IOException \
        com.sun.star.lang.DisposedException \
        com.sun.star.lang.WrappedTargetRuntimeException \
        com.sun.star.lang.XComponent \
        com.sun.star.lang.XEventListener \
        com.sun.star.lang.XInitialization \
        com.sun.star.lang.XMultiComponentFactory \
        com.sun.star.lang.XMultiServiceFactory \
        com.sun.star.lang.XServiceInfo \
        com.sun.star.lang.XSingleComponentFactory \
        com.sun.star.lang.XSingleServiceFactory \
        com.sun.star.lang.XTypeProvider \
        com.sun.star.loader.XImplementationLoader \
        com.sun.star.reflection.XArrayTypeDescription \
        com.sun.star.reflection.XCompoundTypeDescription \
        com.sun.star.reflection.XEnumTypeDescription \
        com.sun.star.reflection.XIdlClass \
        com.sun.star.reflection.XIdlField2 \
        com.sun.star.reflection.XIdlReflection \
        com.sun.star.reflection.XIndirectTypeDescription \
        com.sun.star.reflection.XInterfaceAttributeTypeDescription \
        com.sun.star.reflection.XInterfaceAttributeTypeDescription2 \
        com.sun.star.reflection.XInterfaceMemberTypeDescription \
        com.sun.star.reflection.XInterfaceMethodTypeDescription \
        com.sun.star.reflection.XInterfaceTypeDescription2 \
        com.sun.star.reflection.XMethodParameter \
        com.sun.star.reflection.XStructTypeDescription \
        com.sun.star.reflection.XTypeDescription \
        com.sun.star.reflection.XUnionTypeDescription \
        com.sun.star.registry.XImplementationRegistration \
        com.sun.star.registry.XRegistryKey \
        com.sun.star.registry.XSimpleRegistry \
        com.sun.star.security.RuntimePermission \
        com.sun.star.security.XAccessController \
        com.sun.star.uno.DeploymentException \
        com.sun.star.uno.RuntimeException \
        com.sun.star.uno.XAggregation \
        com.sun.star.uno.XComponentContext \
        com.sun.star.uno.XCurrentContext \
        com.sun.star.uno.XUnloadingPreference \
        com.sun.star.uno.XWeak \
        com.sun.star.util.XMacroExpander

.IF "$(debug)" != ""
# msvc++: no inlining for debugging
.IF "$(COM)" == "MSC"
CFLAGS += -Ob0
.ENDIF
.ENDIF

SLOFILES= \
        $(SLO)$/typeprovider.obj 	\
        $(SLO)$/exc_thrower.obj 	\
        $(SLO)$/servicefactory.obj 	\
        $(SLO)$/bootstrap.obj 		\
        $(SLO)$/implbase.obj 		\
        $(SLO)$/implbase_ex.obj 	\
        $(SLO)$/propshlp.obj 		\
        $(SLO)$/weak.obj		\
        $(SLO)$/interfacecontainer.obj	\
        $(SLO)$/stdidlclass.obj 	\
        $(SLO)$/factory.obj		\
        $(SLO)$/component_context.obj	\
        $(SLO)$/component.obj		\
        $(SLO)$/shlib.obj		\
        $(SLO)$/tdmgr.obj		\
        $(SLO)$/implementationentry.obj	\
        $(SLO)$/access_control.obj	\
        $(SLO)$/macro_expander.obj \
            $(SLO)$/unourl.obj \
        $(SLO)$/propertysetmixin.obj \
        $(SLO)$/findsofficepath.obj

OBJFILES = $(OBJ)$/findsofficepath.obj

.IF "$(COM)" == "MSC"
SHL1TARGET=$(TARGET)$(UDK_MAJOR)$(COMID)
.ELSE
SHL1TARGET=uno_$(TARGET)$(COMID)
.ENDIF

SHL1STDLIBS= \
        $(SALLIB)		\
        $(SALHELPERLIB)	\
        $(CPPULIB)
.IF "$(OS)" == "WNT"
SHL1STDLIBS += $(ADVAPI32LIB)
.ENDIF

SHL1DEPN=
.IF "$(COM)" == "MSC"
SHL1IMPLIB=i$(TARGET)
.ELSE
SHL1IMPLIB=uno_$(TARGET)$(COMID)
.ENDIF
SHL1OBJS = $(SLOFILES)
SHL1RPATH=URELIB

SHL1DEF=$(MISC)$/$(SHL1TARGET).def

DEF1NAME=$(SHL1TARGET)

.IF "$(COMNAME)"=="msci"
SHL1VERSIONMAP=msvc_win32_intel.map
.ELIF "$(COMNAME)"=="mscx"
SHL1VERSIONMAP=msvc_win32_x86_64.map
.ELIF "$(COMNAME)"=="sunpro5"
SHL1VERSIONMAP=cc5_solaris_sparc.map
.ELIF "$(COMNAME)"=="gcc3"
SHL1VERSIONMAP=gcc3.map
.ENDIF

# --- Targets ------------------------------------------------------

.IF "$(diag)"!=""
CFLAGS += -DDIAG=$(diag)
.ENDIF

.INCLUDE :	target.mk
