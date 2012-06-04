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
PRJ=..$/..

PRJNAME=scp2
TARGET=ooo
TARGETTYPE=CUI

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk
.INCLUDE :  i18npool/version.mk

.IF "$(ENABLE_ONLINE_UPDATE)"!=""
SCPDEFS+=-DENABLE_ONLINE_UPDATE
.ENDIF

.IF "$(BUILD_X64)"!=""
SCPDEFS+=-DBUILD_X64
.ENDIF

.IF "$(ENABLE_OPENGL)"=="TRUE"
SCPDEFS+=-DENABLE_OPENGL
.ENDIF

.IF "$(GUI)"=="UNX"
.IF "$(ENABLE_GTK)" != ""
SCPDEFS+=-DENABLE_GTK
PKGCONFIG_MODULES=gtk+-2.0
.INCLUDE: pkg_config.mk
GTK_TWO_FOUR=$(shell @+-$(PKG_CONFIG) --exists 'gtk+-2.0 >= 2.4.0' && echo YES)
.IF "$(GTK_TWO_FOUR)" != ""
SCPDEFS+=-DGTK_TWO_FOUR
.ENDIF
.ENDIF
.IF "$(ENABLE_GTK3)" != ""
SCPDEFS+=-DENABLE_GTK3
.ENDIF
.ENDIF			# "$(GUI)"=="UNX"

.IF "$(ENABLE_NSPLUGIN)" == "YES"
SCPDEFS+=-DENABLE_NSPLUGIN
.ENDIF

.IF "$(MERGELIBS)" == "TRUE"
SCPDEFS+=-DMERGELIBS
.ENDIF

.IF "$(ENABLE_KDE)" != ""
SCPDEFS+=-DENABLE_KDE
.ENDIF

.IF "$(ENABLE_KDE4)" != ""
SCPDEFS+=-DENABLE_KDE4
.ENDIF

.IF "$(ENABLE_KAB)" != ""
SCPDEFS+=-DENABLE_KAB
.ENDIF

.IF "$(ENABLE_EVOAB2)" != ""
SCPDEFS+=-DENABLE_EVOAB2
.ENDIF

.IF "$(ENABLE_DIRECTX)" != ""
SCPDEFS+=-DENABLE_DIRECTX
.ENDIF

.IF "$(ENABLE_XMLSEC)"=="YES"
SCPDEFS+=-DENABLE_XMLSEC
.ENDIF

.IF "$(GUI)"=="UNX"
#X11 MacOSX has no cairo-canvas
.IF "$(OS)" != "MACOSX" || "$(GUIBASE)" == "aqua"
SCPDEFS+=-DENABLE_CAIROCANVAS
.ENDIF
.ENDIF

.IF "$(SYSTEM_EXPAT)" == "YES"
SCPDEFS+=-DSYSTEM_EXPAT
.ENDIF

.IF "$(SYSTEM_CAIRO)" == "YES"
SCPDEFS+=-DSYSTEM_CAIRO
.ENDIF

.IF "$(SYSTEM_LIBXML)" == "YES"
SCPDEFS+=-DSYSTEM_LIBXML
.ENDIF

.IF "$(SYSTEM_LIBXSLT)" == "YES"
SCPDEFS+=-DSYSTEM_LIBXSLT
.ELSE
.INCLUDE :  libxsltversion.mk
SCPDEFS+=-DLIBXSLT_MAJOR=$(LIBXSLT_MAJOR)
.ENDIF

.IF "$(SYSTEM_DB)" == "YES"
SCPDEFS+=-DSYSTEM_DB
.ENDIF

.IF "$(WITH_MOZILLA)" == "NO"
SCPDEFS+=-DWITHOUT_MOZILLA
.ENDIF

.IF "$(WITH_MYSPELL_DICTS)" == "NO"
SCPDEFS+=-DWITHOUT_MYSPELL_DICTS
.ENDIF

.IF "$(SYSTEM_MOZILLA)" == "YES"
SCPDEFS+=-DSYSTEM_MOZILLA
.ENDIF

.IF "$(WITH_LDAP)" == "YES"
SCPDEFS+=-DWITH_LDAP
.ENDIF

.IF "$(SYSTEM_CURL)" == "YES"
SCPDEFS+=-DSYSTEM_CURL
.ENDIF

.IF "$(SYSTEM_STDLIBS)" == "YES"
SCPDEFS+=-DSYSTEM_STDLIBS
.ENDIF

.IF "$(SYSTEM_SNDFILE)" == "YES"
SCPDEFS+=-DSYSTEM_SNDFILE
.ENDIF

.IF "$(SYSTEM_PORTAUDIO)" == "YES"
SCPDEFS+=-DSYSTEM_PORTAUDIO
.ENDIF

.IF "$(SYSTEM_HUNSPELL)" == "YES"
SCPDEFS+=-DSYSTEM_HUNSPELL
.ENDIF

.IF "$(SYSTEM_HYPH)" == "YES"
SCPDEFS+=-DSYSTEM_HYPH
.ENDIF

.IF "$(SYSTEM_MYTHES)" == "YES"
SCPDEFS+=-DSYSTEM_MYTHES
.ENDIF

.IF "$(SYSTEM_LPSOLVE)" == "YES"
SCPDEFS+=-DSYSTEM_LPSOLVE
.ENDIF

.IF "$(SYSTEM_REDLAND)" == "YES"
SCPDEFS+=-DSYSTEM_REDLAND
.ELSE
.INCLUDE :  redlandversion.mk
SCPDEFS+=\
    -DRAPTOR_MAJOR=$(RAPTOR_MAJOR) \
    -DRASQAL_MAJOR=$(RASQAL_MAJOR) \
    -DREDLAND_MAJOR=$(REDLAND_MAJOR)
.ENDIF

.IF "$(SYSTEM_HSQLDB)" == "YES"
SCPDEFS+=-DSYSTEM_HSQLDB -DHSQLDB_JAR=\""file://$(HSQLDB_JAR)"\"
.ENDIF

.IF "$(SYSTEM_SAXON)" == "YES"
SCPDEFS+=-DSYSTEM_SAXON -DSAXON_JAR=\""file://$(SAXON_JAR)"\"
.ENDIF

.IF "$(JDK)" == "gcj"
SCPDEFS+=-DGCJ
.ENDIF

.IF "$(SYSTEM_LUCENE)" == "YES"
SCPDEFS+=-DSYSTEM_LUCENE
.ENDIF

.IF "$(SYSTEM_ICU)" == "YES"
SCPDEFS+=\
    -DSYSTEM_ICU \
    -DICU_MAJOR=$(ICU_MAJOR) \
    -DICU_MINOR=$(ICU_MINOR) \
    -DICU_MICRO=$(ICU_MICRO)
.ELSE
.INCLUDE :  icuversion.mk
SCPDEFS+=\
    -DICU_MAJOR=$(ICU_MAJOR) \
    -DICU_MINOR=$(ICU_MINOR) \
    -DICU_MICRO=$(ICU_MICRO)
.ENDIF

.IF "$(SYSTEM_GRAPHITE)" == "YES"
SCPDEFS+=-DSYSTEM_GRAPHITE
.ENDIF

SCPDEFS+=-DISOLANG_MAJOR=$(ISOLANG_MAJOR)

.IF "$(DISABLE_NEON)" == "TRUE"
SCPDEFS+=-DDISABLE_NEON
.ENDIF
.IF "$(SYSTEM_NEON)" == "YES"
SCPDEFS+=-DSYSTEM_NEON
.ENDIF

.IF "$(SYSTEM_OPENSSL)" == "YES"
SCPDEFS+=-DSYSTEM_OPENSSL
.ENDIF

.IF "$(DISABLE_ATL)"!=""
SCPDEFS+=-DDISABLE_ATL
.ENDIF

.IF "$(DISABLE_PYTHON)" == "TRUE"
SCPDEFS+=-DDISABLE_PYUNO
.ELSE
.IF "$(SYSTEM_PYTHON)" == "YES"
SCPDEFS+=-DSYSTEM_PYTHON
.ENDIF
.ENDIF

.IF "$(SYSTEM_LIBEXTTEXTCAT)" == "YES"
SCPDEFS+=-DSYSTEM_LIBEXTTEXTCAT
.ENDIF

.IF "$(WITH_VC_REDIST)" == "TRUE"
SCPDEFS+=-DWITH_VC_REDIST
.ENDIF

.IF "$(MINGW_GCCDLL)"!=""
SCPDEFS+=-DMINGW_GCCDLL=\""$(MINGW_GCCDLL)"\"
.ENDIF

.IF "$(MINGW_GXXDLL)"!=""
SCPDEFS+=-DMINGW_GXXDLL=\""$(MINGW_GXXDLL)"\"
.ENDIF

.IF "$(SYSTEM_GDKPIXBUF)" == "YES"
SCPDEFS+=-DSYSTEM_GDKPIXBUF
.ENDIF

.IF "$(SYSTEM_GETTEXT)" == "YES"
SCPDEFS+=-DSYSTEM_GETTEXT
.ENDIF

.IF "$(SYSTEM_GLIB)" == "YES"
SCPDEFS+=-DSYSTEM_GLIB
.ENDIF

.IF "$(SYSTEM_LIBCROCO)" == "YES"
SCPDEFS+=-DSYSTEM_LIBCROCO
.ENDIF

.IF "$(SYSTEM_LIBJPEG)" == "YES"
SCPDEFS+=-DSYSTEM_LIBJPEG
.ENDIF

.IF "$(ENABLE_LIBRSVG)" == "INTERNAL"
SCPDEFS+=-DENABLE_LIBRSVG
.ENDIF

.IF "$(SYSTEM_LIBRSVG)" == "YES"
SCPDEFS+=-DSYSTEM_LIBRSVG
.ENDIF

.IF "$(SYSTEM_PANGO)" == "YES"
SCPDEFS+=-DSYSTEM_PANGO
.ENDIF

.IF "$(SYSTEM_LIBGSF)" == "YES"
SCPDEFS+=-DSYSTEM_LIBGSF
.ENDIF

.IF "$(ENABLE_LOMENUBAR)" == "TRUE"
SCPDEFS+=-DENABLE_LOMENUBAR
.ENDIF

# Synchronize with condition to build the jfregqa.dll custom action in setup_native
.IF "$(WINDOWS_SDK_HOME)"!=""
SCPDEFS+=-DHAVE_WINDOWS_SDK
.ENDIF

SCP_PRODUCT_TYPE=osl

ICUVERSION_DEPENDENT_FILES= \
        $(PAR)$/$(SCP_PRODUCT_TYPE)$/file_library_ooo.par \
        $(PAR)$/$(SCP_PRODUCT_TYPE)$/shortcut_ooo.par

ISOLANGVERSION_DEPENDENT_FILES= \
        $(PAR)$/$(SCP_PRODUCT_TYPE)$/file_library_ooo.par

PARFILES=                          \
        installation_ooo.par       \
        scpaction_ooo.par          \
        directory_ooo.par          \
        directory_ooo_macosx.par   \
        file_ooo.par               \
        file_extra_ooo.par         \
        file_font_ooo.par          \
        file_library_ooo.par       \
        file_resource_ooo.par      \
        shortcut_ooo.par           \
        module_ooo.par             \
        module_hidden_ooo.par      \
        module_langpack.par        \
        module_lang_template.par   \
        module_helppack.par        \
        module_help_template.par   \
        module_java.par            \
        module_systemint.par       \
        profile_ooo.par            \
        profileitem_ooo.par        \
        ure.par                    \
        ure_into_ooo.par           \
        ure_standalone.par         \
        common_brand.par           \
        common_brand_readme.par    \
        ooo_brand.par

.IF "$(GUI)"=="WNT"
PARFILES +=                        \
        file_jre_ooo.par           \
        folder_ooo.par             \
        folderitem_ooo.par         \
        registryitem_ooo.par       \
        vc_redist.par              \
        mingw_dlls.par             \
        windowscustomaction_ooo.par
.ENDIF

ULFFILES=                          \
        directory_ooo.ulf          \
        folderitem_ooo.ulf         \
        module_ooo.ulf             \
        module_langpack.ulf        \
        module_helppack.ulf        \
        module_java.ulf            \
        registryitem_ooo.ulf       \
        module_systemint.ulf

# --- File ---------------------------------------------------------
.INCLUDE :  target.mk

.IF "$(SYSTEM_ICU)" != "YES"
$(ICUVERSION_DEPENDENT_FILES) : $(SOLARINCDIR)$/icuversion.mk
.ENDIF

$(ISOLANGVERSION_DEPENDENT_FILES) : $(SOLARINCDIR)$/i18npool/version.mk

