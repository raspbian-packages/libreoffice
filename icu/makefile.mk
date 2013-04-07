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
PRJ=.

PRJNAME=icu
TARGET=so_icu

# --- Settings -----------------------------------------------------

.INCLUDE :	settings.mk

# --- Files --------------------------------------------------------

.INCLUDE :	icuversion.mk

TARFILE_NAME=icu4c-4_4_2-src
TARFILE_MD5=314e582264c36b3735466c522899aa07
TARFILE_ROOTDIR=icu

#icu4c.8320.freeserif.crash.patch, see
#http://bugs.icu-project.org/trac/ticket/8320 for crash with FreeSerif

PATCH_FILES=\
    icu4c-build.patch \
    icu4c-rpath.patch \
    icu4c.8320.freeserif.crash.patch \
    icu4c-aix.patch \
    icu4c-4_4_2-wchar_t.patch \
    icu4c-warnings.patch \
    icu4c-escapespace.patch \
    icu4c-strict-c.patch \
    CVE-2011-4599.patch \
    arm-assembler.patch \
    icu4c-parallel-buid.patch

.IF "$(OS)"=="ANDROID"
PATCH_FILES+=\
    icu4c-android.patch
.ENDIF

.IF "$(GUI)"=="UNX"

.IF "$(SYSBASE)"!=""
icu_CFLAGS+=-I$(SYSBASE)$/usr$/include
.IF "$(COMNAME)"=="sunpro5"
icu_CFLAGS+=$(C_RESTRICTIONFLAGS)
.ENDIF			# "$(COMNAME)"=="sunpro5"
# add SYSBASE libraries and make certain that they are found *after* the
# icu build internal libraries - in case that icu is available in SYSBASE
# as well
icu_LDFLAGS+= -L../lib  -L../../lib -L../stubdata -L../../stubdata  -L$(SYSBASE)$/usr$/lib
.ENDIF			# "$(SYSBASE)"!=""

.IF "$(OS)"=="MACOSX"
.IF "$(EXTRA_CFLAGS)"!=""
CPP:=gcc -E $(EXTRA_CFLAGS)
CXX:=g++ $(EXTRA_CFLAGS)
CC:=gcc $(EXTRA_CFLAGS)
.EXPORT : CPP
.ENDIF # "$(EXTRA_CFLAGS)"!=""
.ENDIF # "$(OS)"=="MACOSX"

.IF "$(debug) != ""
icu_CFLAGS+=-g $(ARCH_FLAGS)
icu_CXXFLAGS+=-g $(ARCH_FLAGS)
.ELSE
icu_CFLAGS+=-O $(ARCH_FLAGS)
icu_CXXFLAGS+=-O $(ARCH_FLAGS)
.ENDIF
icu_LDFLAGS+=$(EXTRA_LINKFLAGS)

# until someone introduces SOLARIS 64-bit builds
.IF "$(OS)"=="SOLARIS"
DISABLE_64BIT=--enable-64bit-libs=no
.ENDIF			# "$(OS)"=="SOLARIS"

.IF "$(OS)"=="AIX"
DISABLE_64BIT=--enable-64bit-libs=no
LDFLAGSADD+=$(LINKFLAGS) $(LINKFLAGSRUNPATH_OOO)
.ENDIF                  # "$(OS)"=="AIX"

.IF "$(HAVE_LD_HASH_STYLE)"  == "TRUE"
LDFLAGSADD += -Wl,--hash-style=both
.ENDIF

.IF "$(OS)"=="IOS"
# Let's try this...
icu_CFLAGS+=-DUCONFIG_NO_FILE_IO
.ENDIF

.IF "$(OS)"=="ANDROID"
# Problems with uint64_t on Android unless disabling strictness
DISABLE_STRICT=--disable-strict
icu_CFLAGS+=-fno-omit-frame-pointer
icu_CXXFLAGS+=-fno-omit-frame-pointer
.ENDIF

.IF "$(OS)"=="IOS" || "$(OS)"=="ANDROID"
# Problems with uint64_t on Android unless disabling strictness
DISABLE_DYLOAD=--disable-dyload
.ENDIF

.IF "$(HAVE_LD_BSYMBOLIC_FUNCTIONS)"  == "TRUE"
LDFLAGSADD += -Wl,-Bsymbolic-functions -Wl,--dynamic-list-cpp-new -Wl,--dynamic-list-cpp-typeinfo
.ENDIF

CONFIGURE_DIR=source

.IF "$(OS)"=="IOS"
STATIC_OR_SHARED=--enable-static --disable-shared
.ELSE
STATIC_OR_SHARED=--disable-static --enable-shared
.ENDIF

.IF "$(CROSS_COMPILING)"=="YES"
# We require that the cross-build-toolset target from the top Makefile(.in) has bee built
BUILD_AND_HOST=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --with-cross-build=$(posix_PWD)/$(INPATH_FOR_BUILD)/misc/build/icu/source
.ENDIF

.IF "$(OS)"=="ANDROID"
LIBRARY_SUFFIX= --with-library-suffix=lo
icu_LDFLAGS+=-lgnustl_shared
.ENDIF

CONFIGURE_ACTION+=sh -c 'CPPFLAGS="$(EXTRA_CDEFS)" CFLAGS="$(icu_CFLAGS)" CXXFLAGS="$(icu_CXXFLAGS)" LDFLAGS="$(icu_LDFLAGS) $(LDFLAGSADD)" \
./configure --enable-layout $(STATIC_OR_SHARED) $(BUILD_AND_HOST) $(DISABLE_64BIT) $(DISABLE_STRICT) $(DISABLE_DYLOAD) $(LIBRARY_SUFFIX)'

CONFIGURE_FLAGS=

# Use of
# CONFIGURE_ACTION=sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure'
# CONFIGURE_FLAGS=--enable-layout --enable-static --enable-shared=yes --enable-64bit-libs=no
# doesn't work as it would result in
# sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure' --enable-layout ...
# note the position of the single quotes.

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=$(AUGMENT_LIBRARY_PATH) $(GNUMAKE) -j$(EXTMAXPROCESS)
.IF "$(OS)"=="IOS"
OUT2LIB= \
    $(BUILD_DIR)$/lib$/libicudata.a \
    $(BUILD_DIR)$/lib$/libicuuc.a \
    $(BUILD_DIR)$/lib$/libicui18n.a \
    $(BUILD_DIR)$/lib$/libicule.a \
    $(BUILD_DIR)$/lib$/libicutu.a
.ELIF "$(OS)"=="ANDROID"
OUT2LIB= \
    $(BUILD_DIR)$/lib$/libicudatalo.so \
    $(BUILD_DIR)$/lib$/libicuuclo.so \
    $(BUILD_DIR)$/lib$/libicui18nlo.so \
    $(BUILD_DIR)$/lib$/libiculelo.so \
    $(BUILD_DIR)$/lib$/libicutulo.so
.ELSE
OUT2LIB= \
    $(BUILD_DIR)$/lib$/libicudata$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicudata$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR) \
    $(BUILD_DIR)$/lib$/libicudata$(DLLPOST) \
    $(BUILD_DIR)$/lib$/libicuuc$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicuuc$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR) \
    $(BUILD_DIR)$/lib$/libicuuc$(DLLPOST) \
    $(BUILD_DIR)$/lib$/libicui18n$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicui18n$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR) \
    $(BUILD_DIR)$/lib$/libicui18n$(DLLPOST) \
    $(BUILD_DIR)$/lib$/libicule$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicule$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR) \
    $(BUILD_DIR)$/lib$/libicule$(DLLPOST) \
    $(BUILD_DIR)$/lib$/libicutu$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR).$(ICU_MICRO) \
    $(BUILD_DIR)$/lib$/libicutu$(DLLPOST).$(ICU_MAJOR)$(ICU_MINOR) \
    $(BUILD_DIR)$/lib$/libicutu$(DLLPOST)

OUT2BIN= \
    $(BUILD_DIR)$/bin$/genccode \
    $(BUILD_DIR)$/bin$/genbrk \
    $(BUILD_DIR)$/bin$/gencmn
.ENDIF

.ENDIF

.IF "$(GUI)"=="WNT"
CONFIGURE_DIR=source
.IF "$(COM)"=="GCC"
.IF "$(MINGW_SHARED_GCCLIB)"=="YES"
icu_LDFLAGS+=-shared-libgcc
.ENDIF
icu_LDFLAGS+=-L$(COMPATH)$/lib
icu_LIBS=
.IF "$(MINGW_SHARED_GXXLIB)"=="YES"
icu_LIBS+=$(MINGW_SHARED_LIBSTDCPP)
.ENDIF
icu_LDFLAGS+=-Wl,--enable-runtime-pseudo-reloc-v2

.IF "$(CROSS_COMPILING)"=="YES"
# We require that the cross-build-toolset target from the top Makefile(.in) has bee built
BUILD_AND_HOST=--build=$(BUILD_PLATFORM) --host=$(HOST_PLATFORM) --with-cross-build=$(posix_PWD)/$(INPATH_FOR_BUILD)/misc/build/icu/source
.ELSE
BUILD_AND_HOST=--build=i586-pc-mingw32 --enable-64bit-libs=no
.ENDIF

CONFIGURE_ACTION+=sh -c 'CFLAGS="-O -D_MT" CXXFLAGS="-O -D_MT" LDFLAGS="$(icu_LDFLAGS)" LIBS="$(icu_LIBS)" \
./configure $(BUILD_AND_HOST) --enable-layout --disable-static --enable-shared'

CONFIGURE_FLAGS=

# Use of
# CONFIGURE_ACTION=sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure'
# CONFIGURE_FLAGS=--enable-layout --enable-static --enable-shared=yes --enable-64bit-libs=no
# doesn't work as it would result in
# sh -c 'CFLAGS=-O CXXFLAGS=-O ./configure' --enable-layout ...
# note the position of the single quotes.

BUILD_DIR=$(CONFIGURE_DIR)
BUILD_ACTION=$(GNUMAKE)
OUT2LIB=

OUT2BIN= \
    $(BUILD_DIR)$/lib$/icudt$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/lib$/icuuc$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/lib$/icuin$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/lib$/icule$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/lib$/icutu$(ICU_MAJOR)$(ICU_MINOR)$(DLLPOST) \
    $(BUILD_DIR)$/bin$/genccode.exe \
    $(BUILD_DIR)$/bin$/genbrk.exe \
    $(BUILD_DIR)$/bin$/gencmn.exe

.ELSE
BUILD_DIR=source
.IF "full_debug" == ""

# Activating the debug mechanism produces incompatible libraries, you'd have
# at least to relink all modules that are directly using ICU. Note that library
# names get a 'd' appended and you'd have to edit the solenv/inc/libs.mk
# ICU*LIB macros as well. Normally you don't want all this.
#
# Instead, use the normal already existing Release build and edit the
# corresponding *.vcproj file of the section you're interested in. Make sure
# that
# - for the VCCLCompilerTool section the following line exists:
#   DebugInformationFormat="3"
# - and for the VCLinkerTool the line
#   GenerateDebugInformation="TRUE"
# Then delete the corresponding Release output directory, and delete the target
# flag files
# $(OUTPATH)/misc/build/so_built_so_icu
# $(OUTPATH)/misc/build/so_predeliver_so_icu
# and run dmake again, after which you may copy the resulting libraries to your
# OOo/SO installation.
ICU_BUILD_VERSION=Debug
ICU_BUILD_LIBPOST=d
.ELSE
ICU_BUILD_VERSION=Release
ICU_BUILD_LIBPOST=
.ENDIF

.IF "$(CPU)" == "I"
ICU_BUILD_ARCH=Win32
.ELSE
ICU_BUILD_ARCH=x64
.ENDIF

.IF "$(CCNUMVER)" >= "001600000000"
BUILD_ACTION=cd allinone && MSBuild.exe allinone.sln /p:Configuration=$(ICU_BUILD_VERSION) /p:Platform=$(ICU_BUILD_ARCH)
.ELSE
BUILD_ACTION=cd allinone && $(COMPATH)$/vcpackages$/vcbuild.exe allinone.sln "$(ICU_BUILD_VERSION)|$(ICU_BUILD_ARCH)"
.ENDIF

OUT2LIB= \
    $(BUILD_DIR)$/..$/lib$/icudt.lib \
    $(BUILD_DIR)$/..$/lib$/icuin$(ICU_BUILD_LIBPOST).lib \
    $(BUILD_DIR)$/..$/lib$/icuuc$(ICU_BUILD_LIBPOST).lib \
    $(BUILD_DIR)$/..$/lib$/icule$(ICU_BUILD_LIBPOST).lib \
    $(BUILD_DIR)$/..$/lib$/icutu$(ICU_BUILD_LIBPOST).lib

OUT2BIN= \
    $(BUILD_DIR)$/..$/bin$/icudt$(ICU_MAJOR)$(ICU_MINOR).dll \
    $(BUILD_DIR)$/..$/bin$/icuin$(ICU_MAJOR)$(ICU_MINOR)$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/..$/bin$/icuuc$(ICU_MAJOR)$(ICU_MINOR)$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/..$/bin$/icule$(ICU_MAJOR)$(ICU_MINOR)$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/..$/bin$/icutu$(ICU_MAJOR)$(ICU_MINOR)$(ICU_BUILD_LIBPOST).dll \
    $(BUILD_DIR)$/..$/bin$/genccode.exe \
    $(BUILD_DIR)$/..$/bin$/genbrk.exe \
    $(BUILD_DIR)$/..$/bin$/gencmn.exe

.ENDIF
.ENDIF		# "$(GUI)"=="WNT"

# --- Targets ------------------------------------------------------

.INCLUDE : set_ext.mk
.INCLUDE :	target.mk
.INCLUDE :	tg_ext.mk

.IF "$(BINARY_PATCH_FILES)"!=""

$(PACKAGE_DIR)$/so_add_binary :  $(PACKAGE_DIR)$/$(ADD_FILES_FLAG_FILE)
    cd $(PACKAGE_DIR) && gunzip -c $(BACK_PATH)$(BINARY_PATCH_FILES) | tar -xvf -
    $(TOUCH) $(PACKAGE_DIR)$/so_add_binary

$(PACKAGE_DIR)$/$(CONFIGURE_FLAG_FILE) : $(PACKAGE_DIR)$/so_add_binary

.ENDIF

.IF "$(GUI)$(COM)"=="WNTGCC"
ALLTAR : \
    $(LB)$/icudata.lib \
    $(LB)$/icuin$(ICU_BUILD_LIBPOST).lib \
    $(LB)$/icuuc$(ICU_BUILD_LIBPOST).lib \
    $(LB)$/icule$(ICU_BUILD_LIBPOST).lib \
    $(LB)$/icutu$(ICU_BUILD_LIBPOST).lib

$(LB)$/icudata.lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@

$(LB)$/icuin$(ICU_BUILD_LIBPOST).lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@

$(LB)$/icuuc$(ICU_BUILD_LIBPOST).lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@

$(LB)$/icule$(ICU_BUILD_LIBPOST).lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@

$(LB)$/icutu$(ICU_BUILD_LIBPOST).lib : $(PACKAGE_DIR)$/$(PREDELIVER_FLAG_FILE)
    $(TOUCH) $@
.ENDIF

# Since you never know what will be in a patch (for example, it may already
# patch at configure level) or in the case of a binary patch, we remove the
# entire package directory if a patch is newer.
# Changes in this makefile could also make a complete build necessary if
# configure is affected.
$(PACKAGE_DIR)$/$(UNTAR_FLAG_FILE) : makefile.mk

