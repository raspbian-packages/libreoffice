# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
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

# set tmpdir to some mixed case path, suitable for native tools
gb_TMPDIR:=$(if $(TMPDIR),$(shell cygpath -m $(TMPDIR)),$(shell cygpath -m /tmp))

# please make generic Windows modifications to windows.mk
include $(GBUILDDIR)/platform/windows.mk

gb_CC := cl
gb_CXX := cl
gb_LINK := link
gb_AWK := awk
gb_CLASSPATHSEP := ;
gb_RC := rc
gb_YACC := bison

# use CC/CXX if they are nondefaults
ifneq ($(origin CC),default)
gb_CC := $(CC)
gb_GCCP := $(CC)
endif
ifneq ($(origin CXX),default)
gb_CXX := $(CXX)
endif

gb_OSDEFS := \
	-DWINVER=0x0500 \
	-D_WIN32_IE=0x0500 \
	-DNT351 \
	-DWIN32 \
	-DWNT \

gb_COMPILERDEFS := \
	-DMSC \
	-D_CRT_NON_CONFORMING_SWPRINTFS \
	-D_CRT_NONSTDC_NO_DEPRECATE \
	-D_CRT_SECURE_NO_DEPRECATE \
	-D_MT \
	-D_DLL \
	-DBOOST_MEM_FN_ENABLE_CDECL \
	-DCPPU_ENV=msci \
	-DM1500 \

gb_CPUDEFS := -D_X86_=1

gb_RCDEFS := \
	 -DWINVER=0x0400 \
	 -DWIN32 \

gb_RCFLAGS := \
	 -V

# FIXME: When porting to Windows64, we should use only:
# /c /Cp
gb_AFLAGS := /c /Cp /coff /safeseh

gb_CFLAGS := \
	-Gd \
	-GR \
	-Gs \
	-GS \
	-MD \
	-nologo \
	-Wall \
	-wd4005 \
	-wd4061 \
	-wd4127 \
	-wd4180 \
	-wd4189 \
	-wd4191 \
	-wd4217 \
	-wd4242 \
	-wd4244 \
	-wd4245 \
	-wd4250 \
	-wd4251 \
	-wd4255 \
	-wd4275 \
	-wd4290 \
	-wd4294 \
	-wd4350 \
	-wd4355 \
	-wd4365 \
	-wd4503 \
	-wd4505 \
	-wd4511 \
	-wd4512 \
	-wd4514 \
	-wd4555 \
	-wd4611 \
	-wd4619 \
	-wd4625 \
	-wd4626 \
	-wd4640 \
	-wd4668 \
	-wd4675 \
	-wd4686 \
	-wd4692 \
	-wd4706 \
	-wd4710 \
	-wd4711 \
	-wd4373 \
	-wd4738 \
	-wd4786 \
	-wd4800 \
	-wd4820 \
	-wd4826 \
	-wd4917 \
	-Zc:forScope,wchar_t- \
	-Zm500 \

gb_CXXFLAGS := \
	-Gd \
	-GR \
	-Gs \
	-GS \
	-Gy \
	-MD \
	-nologo \
	-Wall \
	-wd4005 \
	-wd4061 \
	-wd4127 \
	-wd4180 \
	-wd4189 \
	-wd4191 \
	-wd4217 \
	-wd4242 \
	-wd4244 \
	-wd4245 \
	-wd4250 \
	-wd4251 \
	-wd4275 \
	-wd4290 \
	-wd4294 \
	-wd4350 \
	-wd4355 \
	-wd4365 \
	-wd4503 \
	-wd4505 \
	-wd4511 \
	-wd4512 \
	-wd4514 \
	-wd4555 \
	-wd4611 \
	-wd4619 \
	-wd4625 \
	-wd4626 \
	-wd4640 \
	-wd4668 \
	-wd4675 \
	-wd4686 \
	-wd4692 \
	-wd4706 \
	-wd4710 \
	-wd4711 \
	-wd4373 \
	-wd4738 \
	-wd4786 \
	-wd4800 \
	-wd4820 \
	-wd4826 \
	-wd4917 \
	-wd4996 \
	-Zc:forScope,wchar_t- \
	-Zm500 \

gb_STDLIBS := \
	uwinapi \
	advapi32 \

ifneq ($(EXTERNAL_WARNINGS_NOT_ERRORS),TRUE)
gb_CFLAGS_WERROR := -WX -DLIBO_WERROR
gb_CXXFLAGS_WERROR := -WX -DLIBO_WERROR
endif

gb_LinkTarget_EXCEPTIONFLAGS := \
	-DEXCEPTIONS_ON \
	-EHa \

gb_LinkTarget_NOEXCEPTIONFLAGS := \
	-DEXCEPTIONS_OFF \

gb_LinkTarget_LDFLAGS := \
	-MACHINE:IX86 \
	-MANIFEST \
	$(patsubst %,-LIBPATH:%,$(filter-out .,$(subst ;, ,$(subst \,/,$(ILIB))))) \

gb_DEBUG_CFLAGS := -Zi

# this does not use CFLAGS so it is not overridable
ifneq ($(ENABLE_CRASHDUMP),)
gb_CFLAGS+=-Zi
gb_CXXFLAGS+=-Zi
endif

ifeq ($(gb_SYMBOL),$(true))
gb_CFLAGS+=-Zi
gb_CXXFLAGS+=-Zi
endif

ifneq ($(gb_DEBUGLEVEL),0)
gb_COMPILEROPTFLAGS :=
else
gb_COMPILEROPTFLAGS := -Ob1 -Oxs -Oy-
endif

gb_COMPILERNOOPTFLAGS := -Od

ifeq ($(gb_FULLDEPS),$(true))
gb_COMPILERDEPFLAGS := -showIncludes
define gb_create_deps
| $(GBUILDDIR)/filter-showIncludes.pl $(2) $(1) $(3); exit $${PIPESTATUS[0]}
endef
else
gb_COMPILERDEPFLAGS :=
define gb_create_deps
endef
endif

# Helper class

gb_Helper_SRCDIR_NATIVE := $(shell cygpath -m $(SRCDIR) | $(gb_AWK) -- '{ print tolower(substr($$0,1,1)) substr($$0,2) }')
gb_Helper_WORKDIR_NATIVE := $(shell cygpath -m $(WORKDIR) | $(gb_AWK) -- '{ print tolower(substr($$0,1,1)) substr($$0,2) }')
gb_Helper_OUTDIR_NATIVE := $(shell cygpath -m $(OUTDIR) | $(gb_AWK) -- '{ print tolower(substr($$0,1,1)) substr($$0,2) }')
gb_Helper_REPODIR_NATIVE := $(shell cygpath -m $(REPODIR) | $(gb_AWK) -- '{ print tolower(substr($$0,1,1)) substr($$0,2) }')

define gb_Helper_abbreviate_dirs_native
R=$(gb_Helper_REPODIR_NATIVE) && $(subst $(REPODIR)/,$$R/,$(subst $(gb_Helper_REPODIR_NATIVE)/,$$R/,O=$(gb_Helper_OUTDIR_NATIVE) && W=$(gb_Helper_WORKDIR_NATIVE) && S=$(gb_Helper_SRCDIR_NATIVE))) && \
$(subst $(REPODIR)/,$$R/,$(subst $(SRCDIR)/,$$S/,$(subst $(OUTDIR)/,$$O/,$(subst $(WORKDIR)/,$$W/,$(subst $(gb_Helper_REPODIR_NATIVE)/,$$R/,$(subst $(gb_Helper_SRCDIR_NATIVE)/,$$S/,$(subst $(gb_Helper_OUTDIR_NATIVE)/,$$O/,$(subst $(gb_Helper_WORKDIR_NATIVE)/,$$W/,$(1)))))))))
endef

gb_Helper_set_ld_path := PATH="$${PATH}:$(OUTDIR)/bin"

# convert parameters filesystem root to native notation
# does some real work only on windows, make sure not to
# break the dummy implementations on unx*
define gb_Helper_convert_native
$(patsubst -I$(OUTDIR)%,-I$(gb_Helper_OUTDIR_NATIVE)%, \
$(patsubst $(OUTDIR)%,$(gb_Helper_OUTDIR_NATIVE)%, \
$(patsubst $(WORKDIR)%,$(gb_Helper_WORKDIR_NATIVE)%, \
$(patsubst $(SRCDIR)%,$(gb_Helper_SRCDIR_NATIVE)%, \
$(1)))))
endef

# YaccTarget class

define gb_YaccTarget__command
$(call gb_Output_announce,$(2),$(true),YAC,3)
$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(dir $(3)) && \
	$(gb_YACC) $(T_YACCFLAGS) --defines=$(4) -o $(3) $(1) )

endef

# CObject class

define gb_CObject__command
$(call gb_Output_announce,$(2),$(true),C  ,3)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CC) \
		$(DEFS) \
		$(T_CFLAGS) \
		-Fd$(PDBFILE) \
		$(gb_COMPILERDEPFLAGS) \
		-I$(realpath $(dir $(3))) \
		$(INCLUDE) \
		-c $(realpath $(3)) \
		-Fo$(1)) $(call gb_create_deps,$(1),$(4),$(realpath $(3)))
endef


# CxxObject class

define gb_CxxObject__command
$(call gb_Output_announce,$(2),$(true),CXX,3)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	$(gb_CXX) \
		$(DEFS) \
		$(T_CXXFLAGS) \
		-Fd$(PDBFILE) \
		$(gb_COMPILERDEPFLAGS) \
		-I$(realpath $(dir $(3))) \
		$(INCLUDE_STL) $(INCLUDE) \
		-c $(realpath $(3)) \
		-Fo$(1)) $(call gb_create_deps,$(1),$(4),$(realpath $(3)))
endef


# AsmObject class

gb_AsmObject_get_source = $(1)/$(2).asm

define gb_AsmObject__command
$(call gb_Output_announce,$(2),$(true),ASM,3)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) $(dir $(4)) && \
	"$(ML_EXE)" $(gb_AFLAGS) -D$(COM) /Fo$(1) $(3)) && \
	echo "$(1) : $(realpath $(3))" > $(4)
endef


# LinkTarget class

gb_LinkTarget_CFLAGS := $(gb_CFLAGS) $(gb_CFLAGS_WERROR)
gb_LinkTarget_CXXFLAGS := $(gb_CXXFLAGS) $(gb_CXXFLAGS_WERROR)

gb_LinkTarget_INCLUDE :=\
	$(filter-out %/stl, $(subst -I. , ,$(SOLARINC))) \
	$(foreach inc,$(subst ;, ,$(JDKINC)),-I$(inc)) \

gb_LinkTarget_INCLUDE_STL := $(filter %/stl, $(subst -I. , ,$(SOLARINC)))

gb_LinkTarget_get_pdbfile = $(call gb_LinkTarget_get_target,)pdb/$(1).pdb

define gb_LinkTarget__command
$(call gb_Output_announce,$(2),$(true),LNK,4)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	rm -f $(1) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),100, \
		$(call gb_Helper_convert_native,$(foreach object,$(CXXOBJECTS),$(call gb_CxxObject_get_target,$(object))) \
		$(foreach object,$(GENCXXOBJECTS),$(call gb_GenCxxObject_get_target,$(object))) \
		$(foreach object,$(COBJECTS),$(call gb_CObject_get_target,$(object))) \
		$(foreach object,$(GENCOBJECTS),$(call gb_GenCObject_get_target,$(object))) \
		$(foreach object,$(ASMOBJECTS),$(call gb_AsmObject_get_target,$(object))) \
		$(foreach extraobjectlist,$(EXTRAOBJECTLISTS),$(shell cat $(extraobjectlist))) \
		$(NATIVERES))) && \
	$(gb_LINK) \
		$(if $(filter Library CppunitTest,$(TARGETTYPE)),$(gb_Library_TARGETTYPEFLAGS)) \
		$(if $(filter StaticLibrary,$(TARGETTYPE)),$(gb_StaticLibrary_TARGETTYPEFLAGS)) \
		$(if $(filter Executable,$(TARGETTYPE)),$(gb_Executable_TARGETTYPEFLAGS)) \
		$(if $(filter YES,$(TARGETGUI)), -SUBSYSTEM:WINDOWS, -SUBSYSTEM:CONSOLE) \
		$(T_LDFLAGS) \
		$(if $(gb_PRODUCT),,-NODEFAULTLIB) \
		@$${RESPONSEFILE} \
		$(foreach lib,$(LINKED_LIBS),$(call gb_Library_get_filename,$(lib))) \
		$(foreach lib,$(LINKED_STATIC_LIBS),$(call gb_StaticLibrary_get_filename,$(lib))) \
		$(LIBS) \
		$(if $(filter-out StaticLibrary,$(TARGETTYPE)),$(if $(gb_PRODUCT),,oldnames.lib $(if $(filter libcmtd,$(LINKED_LIBS)),,msvcrtd.lib) msvcprtd.lib kernel32.lib) user32.lib) \
		$(if $(DLLTARGET),-out:$(DLLTARGET) -implib:$(1),-out:$(1)); RC=$$?; rm $${RESPONSEFILE} \
	$(if $(DLLTARGET),; if [ ! -f $(DLLTARGET) ]; then rm -f $(1) && false; fi) \
	$(if $(filter Library,$(TARGETTYPE)),; if [ -f $(DLLTARGET).manifest ]; then mt.exe $(MTFLAGS) -manifest $(DLLTARGET).manifest -outputresource:$(DLLTARGET)\;2; fi) \
	$(if $(filter Executable,$(TARGETTYPE)),; if [ -f $(1).manifest ]; then mt.exe $(MTFLAGS) -manifest $(1).manifest -outputresource:$(1)\;1; fi) \
	; exit $$RC)
endef


# Flags common for PE executables (EXEs and DLLs) 
gb_Windows_PE_TARGETTYPEFLAGS := \
    -release \
    -opt:noref \
    -incremental:no \
    -debug \
    -safeseh \
	-nxcompat \
	-dynamicbase \

# Library class


gb_Library_DEFS := -D_DLL
gb_Library_TARGETTYPEFLAGS := \
    -DLL \
    $(gb_Windows_PE_TARGETTYPEFLAGS)

gb_Library_get_rpath :=

gb_Library_SYSPRE := i
gb_Library_PLAINEXT := .lib

gb_Library_win32_OLDNAMES := oldnames

gb_Library_PLAINLIBS_NONE += \
	advapi32 \
	comdlg32 \
	crypt32 \
	d3d9 \
	d3dx \
	ddraw \
	gdi32 \
	gdiplus \
	gnu_getopt \
	imm32\
	kernel32 \
	jawt \
	libcmt \
	libcmtd \
	mpr \
	msi \
	msimg32 \
	msvcrt \
	msvcprt \
	$(gb_Library_win32_OLDNAMES) \
	ole32 \
	oleaut32 \
	shell32 \
	shlwapi \
	user32 \
	uuid \
	uwinapi \
	version \
	winmm \
	winspool \

gb_Library_LAYER := \
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):OOO) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):OOO) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):URELIB) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):OOO) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):URELIB) \

gb_Library_FILENAMES :=\
	$(foreach lib,$(gb_Library_TARGETS),$(lib):$(gb_Library_SYSPRE)$(lib)$(gb_Library_PLAINEXT)) \

gb_Library_DLLEXT := .dll
gb_Library_MAJORVER := 3
gb_Library_RTEXT := MSC$(gb_Library_DLLEXT)
gb_Library_OOOEXT := $(gb_Library_DLLPOSTFIX)$(gb_Library_DLLEXT)
gb_Library_UNOEXT := .uno$(gb_Library_DLLEXT)
gb_Library_UNOVEREXT := $(gb_Library_MAJORVER)$(gb_Library_DLLEXT)
gb_Library_RTVEREXT := $(gb_Library_MAJORVER)$(gb_Library_RTEXT)

gb_Library_DLLFILENAMES :=\
	$(foreach lib,$(gb_Library_OOOLIBS),$(lib):$(lib)$(gb_Library_OOOEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_NONE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_URE),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_PLAINLIBS_OOO),$(lib):$(lib)$(gb_Library_DLLEXT)) \
	$(foreach lib,$(gb_Library_RTLIBS),$(lib):$(lib)$(gb_Library_RTEXT)) \
	$(foreach lib,$(gb_Library_RTVERLIBS),$(lib):$(lib)$(gb_Library_RTVEREXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_URE),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOLIBS_OOO),$(lib):$(lib)$(gb_Library_UNOEXT)) \
	$(foreach lib,$(gb_Library_UNOVERLIBS),$(lib):$(lib)$(gb_Library_UNOVEREXT)) \

define gb_Library_Library_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(3))

$(call gb_LinkTarget_set_auxtargets,$(2),\
	$(patsubst %.lib,%.exp,$(call gb_LinkTarget_get_target,$(2))) \
	$(3).manifest \
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
	$(patsubst %.dll,%.pdb,$(3)) \
	$(patsubst %.dll,%.ilk,$(3)) \
)

$(call gb_Library_get_target,$(1)) \
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS := $(OUTDIR)/bin/$(notdir $(3))

ifneq ($(ENABLE_CRASHDUMP),)
$(call gb_Library_get_target,$(1)) \
$(call gb_Library_get_clean_target,$(1)) : AUXTARGETS +=  \
		$(OUTDIR)/bin/$(notdir $(patsubst %.dll,%.pdb,$(3))) \
		$(OUTDIR)/bin/$(notdir $(patsubst %.dll,%.ilk,$(3))) \

endif

$(call gb_Library_add_default_nativeres,$(1),$(1)/default)

$(call gb_Deliver_add_deliverable,$(OUTDIR)/bin/$(notdir $(3)),$(3),$(1))

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

endef

define gb_Library_add_default_nativeres
$(call gb_WinResTarget_WinResTarget_init,$(2))
$(call gb_WinResTarget_add_file,$(2),solenv/inc/shlinfo)
$(call gb_WinResTarget_set_defs,$(2),\
		$$(DEFS) \
		-DVERVARIANT="$(BUILD)" \
		-DRES_APP_VENDOR="$(OOO_VENDOR)" \
		-DORG_NAME="$(call gb_Library_get_dllname,$(1))"\
		-DINTERNAL_NAME="$(subst $(gb_Library_DLLEXT),,$(call gb_Library_get_dllname,$(1)))" \
		-DADDITIONAL_VERINFO1="" \
		-DADDITIONAL_VERINFO2="" \
		-DADDITIONAL_VERINFO3="" \
)
$(call gb_Library_add_nativeres,$(1),$(2))
$(call gb_Library_get_clean_target,$(1)) : $(call gb_WinResTarget_get_clean_target,$(2))

endef

define gb_LinkTarget_add_nativeres
$(call gb_LinkTarget_get_target,$(1)) : $(call gb_WinResTarget_get_target,$(2))
$(call gb_LinkTarget_get_target,$(1)) : NATIVERES += $(call gb_WinResTarget_get_target,$(2))

endef

define gb_Library_get_dllname
$(patsubst $(1):%,%,$(filter $(1):%,$(gb_Library_DLLFILENAMES)))
endef


# StaticLibrary class

gb_StaticLibrary_DEFS :=
gb_StaticLibrary_TARGETTYPEFLAGS := -LIB
gb_StaticLibrary_SYSPRE :=
gb_StaticLibrary_PLAINEXT := .lib
gb_StaticLibrary_JPEGEXT := lib$(gb_StaticLibrary_PLAINEXT)

gb_StaticLibrary_FILENAMES := \
	$(foreach lib,$(gb_StaticLibrary_JPEGLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_JPEGEXT)) \
	$(foreach lib,$(gb_StaticLibrary_PLAINLIBS),$(lib):$(gb_StaticLibrary_SYSPRE)$(lib)$(gb_StaticLibrary_PLAINEXT)) \

gb_StaticLibrary_FILENAMES := $(patsubst salcpprt:salcpprt%,salcpprt:cpprtl%,$(gb_StaticLibrary_FILENAMES))

define gb_StaticLibrary_StaticLibrary_platform
$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

$(call gb_LinkTarget_set_auxtargets,$(2),\
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
)

endef

# Executable class

gb_Executable_EXT := .exe
gb_Executable_TARGETTYPEFLAGS := $(gb_Windows_PE_TARGETTYPEFLAGS)

gb_Executable_get_rpath :=

define gb_Executable_Executable_platform
$(call gb_LinkTarget_set_auxtargets,$(2),\
	$(patsubst %.exe,%.pdb,$(call gb_LinkTarget_get_target,$(2))) \
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
	$(call gb_LinkTarget_get_target,$(2)).manifest \
)

$(call gb_Executable_get_target,$(1)) \
$(call gb_Executable_get_clean_target,$(1)) : AUXTARGETS := $(call gb_Executable_get_target,$(1)).manifest
$(call gb_Deliver_add_deliverable,$(call gb_Executable_get_target,$(1)).manifest,$(call gb_LinkTarget_get_target,$(2)).manifest,$(1))

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE := $(call gb_LinkTarget_get_pdbfile,$(2))
$(call gb_LinkTarget_get_target,$(2)) : TARGETGUI :=

endef

# CppunitTest class

gb_CppunitTest_DEFS := -D_DLL
# cppunittester.exe is in the cppunit subdirectory of ${OUTDIR}/bin,
# thus it won't find its DLLs unless ${OUTDIR}/bin is added to PATH.
# PATH is the Cygwin one while ${OUTDIR} is a Win32 pathname, thus
# cygpath -u.
gb_CppunitTest_CPPTESTPRECOMMAND := PATH="`cygpath -u $(OUTDIR)`/bin:$${PATH}"

gb_CppunitTest_SYSPRE := itest_
gb_CppunitTest_EXT := .lib
gb_CppunitTest_LIBDIR := $(gb_Helper_OUTDIRLIBDIR)
gb_CppunitTest_get_filename = $(gb_CppunitTest_SYSPRE)$(1)$(gb_CppunitTest_EXT)
gb_CppunitTest_get_libfilename = test_$(1).dll

define gb_CppunitTest_CppunitTest_platform
$(call gb_LinkTarget_set_dlltarget,$(2),$(3))

$(call gb_LinkTarget_set_auxtargets,$(2),\
	$(patsubst %.lib,%.exp,$(call gb_LinkTarget_get_target,$(2))) \
	$(3).manifest \
	$(patsubst %.dll,%.pdb,$(3)) \
	$(call gb_LinkTarget_get_pdbfile,$(2)) \
	$(patsubst %.dll,%.ilk,$(3)) \
)

$(call gb_LinkTarget_get_target,$(2)) \
$(call gb_LinkTarget_get_headers_target,$(2)) : PDBFILE = $(call gb_LinkTarget_get_pdbfile,$(2))

endef

# JunitTest class

gb_defaultlangiso := en-US
gb_smoketest_instset := $(SRCDIR)/instsetoo_native/$(INPATH)/OpenOffice/archive/install/$(gb_defaultlangiso)/OOo_*_install-arc_$(gb_defaultlangiso).zip

ifeq ($(OOO_TEST_SOFFICE),)


# Work around Windows problems with long pathnames (see issue 50885) by
# installing into the temp directory instead of the module output tree (in which
# case $(target).instpath contains the path to the temp installation,
# which is removed after smoketest); can be removed once issue 50885 is fixed;
# on other platforms, a single installation to solver is created in
# smoketestoo_native.

# for now, no dependency on $(shell ls $(gb_smoketest_instset))
# because that doesn't work before the instset is built
# and there is not much of a benefit anyway (gbuild not knowing about smoketest)
define gb_JunitTest_JunitTest_platform_longpathname_hack
$(call gb_JunitTest_get_target,$(1)) : $(call gb_JunitTest_get_target,$(1)).instpath
$(call gb_JunitTest_get_target,$(1)) : CLEAN_CMD = $(call gb_Helper_abbreviate_dirs,rm -rf `cat $$@.instpath` $$@.instpath)

$(call gb_JunitTest_get_target,$(1)).instpath : 
	INST_DIR=$$$$(cygpath -m `mktemp -d -t testinst.XXXXXX`) \
	&& unzip -d "$$$${INST_DIR}"  $$(gb_smoketest_instset) \
	&& mv "$$$${INST_DIR}"/OOo_*_install-arc_$$(gb_defaultlangiso) "$$$${INST_DIR}"/opt\
	&& mkdir -p $$(dir $$@) \
	&& echo "$$$${INST_DIR}" > $$@

endef
else # OOO_TEST_SOFFICE
gb_JunitTest_JunitTest_platform_longpathname_hack =
endif # OOO_TEST_SOFFICE

define gb_JunitTest_JunitTest_platform
$(call gb_JunitTest_JunitTest_platform_longpathname_hack,$(1))

$(call gb_JunitTest_get_target,$(1)) : DEFS := \
	-Dorg.openoffice.test.arg.soffice="$$$${OOO_TEST_SOFFICE:-path:`cat $(call gb_JunitTest_get_target,$(1)).instpath`/opt/OpenOffice.org 3/program/soffice.exe}" \
	-Dorg.openoffice.test.arg.env=PATH \
	-Dorg.openoffice.test.arg.user=file:///$(call gb_JunitTest_get_userdir,$(1)) \

endef

# SrsPartTarget class

ifeq ($(gb_FULLDEPS),$(true))
define gb_SrsPartTarget__command_dep
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(INCLUDE) \
		$(DEFS) \
		$(2) \
		-f - \
	| $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
		-v OBJECTFILE=$(call gb_SrsPartTarget_get_target,$(1)) \
		-v OUTDIR=$(OUTDIR)/ \
		-v WORKDIR=$(WORKDIR)/ \
		-v SRCDIR=$(SRCDIR)/ \
		-v REPODIR=$(REPODIR)/ \
	> $(call gb_SrsPartTarget_get_dep_target,$(1)))
endef
else
gb_SrsPartTarget__command_dep =
endif

# WinResTarget class

gb_WinResTarget_POSTFIX :=.res

define gb_WinResTarget__command
$(call gb_Output_announce,$(2),$(true),RES,3)
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	$(gb_RC) \
		$(DEFS) $(FLAGS) \
		-I$(dir $(3)) \
		$(INCLUDE) \
		-Fo$(1) \
		$(RCFILE) )
endef

$(eval $(call gb_Helper_make_dep_targets,\
	WinResTarget \
))

ifeq ($(gb_FULLDEPS),$(true))
define gb_WinResTarget__command_dep
$(call gb_Helper_abbreviate_dirs_native,\
	$(OUTDIR)/bin/makedepend$(gb_Executable_EXT) \
		$(INCLUDE) \
		$(DEFS) \
		$(2) \
		-f - \
	| $(gb_AWK) -f $(GBUILDDIR)/processdeps.awk \
		-v OBJECTFILE=$(call gb_WinResTarget_get_target,$(1)) \
		-v OUTDIR=$(OUTDIR)/ \
		-v WORKDIR=$(WORKDIR)/ \
		-v SRCDIR=$(SRCDIR)/ \
		-v REPODIR=$(REPODIR)/ \
	> $(call gb_WinResTarget_get_dep_target,$(1)))
endef
else
gb_WinResTarget__command_dep =
endif

# Python
gb_PYTHON_PRECOMMAND :=  PATH="$${PATH}:$(OUTDIR)/bin" PYTHONHOME="$(OUTDIR)/lib/python" PYTHONPATH="$(OUTDIR)/lib/python;$(OUTDIR)/lib/python/lib-dynload"

# vim: set noet sw=4:
