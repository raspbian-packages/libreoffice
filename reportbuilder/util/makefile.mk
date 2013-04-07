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
PRJNAME=reportbuilder
TARGET=rpt
EXTENSION_VERSION_BASE=1.2.1

    EXTENSION_VERSION=$(EXTENSION_VERSION_BASE)

# --- Settings ----------------------------------
.INCLUDE :  makefile.pmk

.IF "$(L10N_framework)"==""
# ------------------------------------------------------------------
# calready set in util$/makefile.pmk
EXTENSION_ZIPNAME:=report-builder

# create Extension -----------------------------

.IF "$(SOLAR_JAVA)"!=""

XMLFILES =  $(EXTENSIONDIR)$/META-INF$/manifest.xml

# DESCRIPTION_SRC is the source file which is copied into the extension
# It is defaulted to "description.xml", but we want to pre-process it, so we use an intermediate
# file
DESCRIPTION_SRC = $(MISC)$/description.xml

COMPONENT_MERGED_XCU= \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Setup.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/DataAccess.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/ReportDesign.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/ExtendedColorScheme.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Embedding.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Paths.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/Accelerators.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/TypeDetection$/Filter.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/TypeDetection$/Types.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/ReportCommands.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/Controller.xcu \
            $(EXTENSIONDIR)$/registry$/data$/org$/openoffice$/Office$/UI$/DbReportWindowState.xcu \
            $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/UI$/DbReportWindowState.xcs \
            $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/UI$/ReportCommands.xcs \
            $(EXTENSIONDIR)$/registry$/schema$/org$/openoffice$/Office$/ReportDesign.xcs

COMPONENT_OTR_FILES= \
    $(EXTENSIONDIR)$/template$/en-US$/wizard$/report$/default.otr
    
COMPONENT_IMAGES= \
    $(EXTENSIONDIR)$/images$/extension_32.png

COMPONENT_HTMLFILES = $(EXTENSIONDIR)$/THIRDPARTYREADMELICENSE.html \
            $(EXTENSIONDIR)$/readme_en-US.html \
            $(EXTENSIONDIR)$/readme_en-US.txt

COMPONENT_JARFILES = \
    $(EXTENSIONDIR)$/sun-report-builder.jar

COMPONENT_DESCRIPTION= \
    $(foreach,lang,$(alllangiso) $(EXTENSIONDIR)$/description-$(lang).txt)

# .jar files from solver
COMPONENT_EXTJARFILES = \
    $(EXTENSIONDIR)$/sun-report-builder.jar
COMPONENT_EXTJARFILES_COPY = \
    $(OUTDIR)$/bin$/reportbuilderwizard.jar

.IF "$(SYSTEM_JFREEREPORT)" != "YES"
.INCLUDE :  $(OUTDIR)/bin/jfreereport_version.mk
COMPONENT_EXTJARFILES_COPY += \
    $(OUTDIR)$/bin$/flute-$(FLUTE_VERSION).jar				            \
    $(OUTDIR)$/bin$/libserializer-$(LIBSERIALIZER_VERSION).jar			\
    $(OUTDIR)$/bin$/libbase-$(LIBBASE_VERSION).jar                     \
    $(OUTDIR)$/bin$/libfonts-$(LIBFONTS_VERSION).jar					\
    $(OUTDIR)$/bin$/libformula-$(LIBFORMULA_VERSION).jar						\
    $(OUTDIR)$/bin$/liblayout-$(LIBLAYOUT_VERSION).jar					\
    $(OUTDIR)$/bin$/libloader-$(LIBLOADER_VERSION).jar					\
    $(OUTDIR)$/bin$/librepository-$(LIBREPOSITORY_VERSION).jar			\
    $(OUTDIR)$/bin$/libxml-$(LIBXML_VERSION).jar						\
    $(OUTDIR)$/bin$/flow-engine-$(FLOW_ENGINE_VERSION).jar 	        \
    $(OUTDIR)$/bin$/sac.jar
.ENDIF
.IF "$(SYSTEM_APACHE_COMMONS)" != "YES"
COMPONENT_EXTJARFILES_COPY += \
    $(OUTDIR)$/bin$/commons-logging-1.1.1.jar
.ENDIF

COMPONENT_MANIFEST_GENERIC:=TRUE
COMPONENT_MANIFEST_SEARCHDIR:=registry

# make sure to add your custom files here
EXTENSION_PACKDEPS=$(COMPONENT_EXTJARFILES) $(MISC)/$(TARGET).copied $(COMPONENT_HTMLFILES) $(COMPONENT_OTR_FILES) $(COMPONENT_DESCRIPTION) $(COMPONENT_IMAGES) $(EXTENSIONDIR)/components.rdb
.ENDIF
# --- Targets ----------------------------------

.INCLUDE : extension_pre.mk
.INCLUDE : target.mk
.INCLUDE : extension_post.mk
.IF "$(L10N_framework)"==""
$(EXTENSIONDIR)$/%.jar : $(SOLARBINDIR)$/%.jar
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(EXTENSIONDIR)/components.rdb : components.rdb
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(EXTENSIONDIR)$/readme_en-US.% : $(PRJ)$/license$/readme_en-US.%
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(COMPONENT_IMAGES) : $(SOLARSRC)$/$(RSCDEFIMG)$/desktop$/res$/$$(@:f)
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@

$(EXTENSIONDIR)$/THIRDPARTYREADMELICENSE.html : $(PRJ)$/license$/THIRDPARTYREADMELICENSE.html
    @@-$(MKDIRHIER) $(@:d)
    $(COPY) $< $@
    
$(COMPONENT_DESCRIPTION) : $(DESCRIPTION)
    

$(DESCRIPTION_SRC): description.xml
    +-$(RM) $@
    $(COPY) description-en-US.txt $(EXTENSIONDIR)/description-en-US.txt
.IF "$(WITH_LANG)" != ""
    $(XRMEX) -p $(PRJNAME) -i description.xml -o $@ -m $(LOCALIZESDF) -l all
    $(SED) "s/#VERSION#/$(EXTENSION_VERSION)/" < $@ > $@.new
    mv $@.new $@
    @@-$(COPY) $(@:d)/description-*.txt $(EXTENSIONDIR)
.ELSE
    $(SED) "s/#VERSION#/$(EXTENSION_VERSION)/" < $< > $@
.ENDIF

$(MISC)/$(TARGET).copied: $(COMPONENT_EXTJARFILES_COPY)
    @@-$(MKDIRHIER) $(EXTENSIONDIR)
    : $(foreach,i,$(COMPONENT_EXTJARFILES_COPY) && $(COPY) $i $(EXTENSIONDIR)/)
    $(TOUCH) $@
.ENDIF
.ELSE			# "$(SOLAR_JAVA)"!=""
.INCLUDE : target.mk
.ENDIF			# "$(SOLAR_JAVA)"!=""
#
