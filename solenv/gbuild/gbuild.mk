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

GBUILDDIR:=$(SRCDIR)/solenv/gbuild

# vars needed from the env/calling makefile

# DEBUG
# INPATH
# JAVA_HOME
# JDKINCS
# LIBXML_CFLAGS
# OS
# PRODUCT
# SOLARINC
# SOLARLIB
# UPD

# GXX_INCLUDE_PATH (Linux)
# PTHREAD_CFLAGS (Linux)
# SYSTEM_ICU (Linux)
# SYSTEM_JPEG (Linux)
# SYSTEM_LIBXML (Linux)

.DELETE_ON_ERROR:

SHELL := /bin/sh
MAKEFLAGS+=-r
true := T
false :=
define NEWLINE


endef

define WHITESPACE
 
endef

COMMA :=,

# optional extensions that should never be essential
ifneq ($(wildcard $(GBUILDDIR)/extensions/pre_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/pre_*.mk)
endif

include $(GBUILDDIR)/Output.mk

# BuildDirs uses the Output functions already
include $(GBUILDDIR)/BuildDirs.mk


ifneq ($(strip $(PRODUCT)$(product)),)
gb_PRODUCT := $(true)
else
ifneq ($(strip $(product)),)
gb_PRODUCT := $(true)
else
gb_PRODUCT := $(false)
endif
endif

ifneq ($(strip $(ENABLE_SYMBOLS)$(enable_symbols)),)
gb_SYMBOL := $(true)
else
gb_SYMBOL := $(false)
endif

ifeq ($(strip $(DISABLE_STRIP)$(disable_strip)),)
gb_STRIP := $(true)
else
gb_STRIP := $(false)
endif

gb_DEBUGLEVEL := 0
ifneq ($(strip $(DEBUG)$(debug)),)
gb_DEBUGLEVEL := 1
else ifeq ($(gb_PRODUCT),$(false))
gb_DEBUGLEVEL := 1
endif

ifneq ($(strip $(DBGLEVEL)$(dbglevel)),)
ifneq ($(strip $(dbglevel)),)
gb_DEBUGLEVEL := $(strip $(dbglevel))
else
gb_DEBUGLEVEL := $(strip $(DBGLEVEL))
endif
endif

ifneq ($(gb_DEBUGLEVEL),0)
gb_SYMBOL := $(true)
endif

ifneq ($(nodep),)
gb_FULLDEPS := $(false)
else
# for clean, setuplocal and removelocal goals we switch off dependencies
ifneq ($(filter cleanpackmodule clean setuplocal removelocal showdeliverables help debugrun,$(MAKECMDGOALS)),)
gb_FULLDEPS := $(false)
else
gb_FULLDEPS := $(true)
endif
endif

# save user-supplied flags for latter use
ifneq ($(strip $(CFLAGS)),)
gb__ENV_CFLAGS := $(CFLAGS)
else
# TODO remove after the old build system is abolished
ifneq ($(strip $(ENVCFLAGS)),)
gb__ENV_CFLAGS := $(ENVCFLAGS)
endif
endif
ifneq ($(strip $(CXXFLAGS)),)
gb__ENV_CXXFLAGS := $(CXXFLAGS)
else
# TODO remove after the old build system is abolished
ifneq ($(strip $(ENVCFLAGSCXX)),)
gb__ENV_CXXFLAGS := $(ENVCFLAGSCXX)
endif
endif

gb__ENV_CXXFLAGS += $(HARDEN_CXXFLAGS)
gb__ENV_CFLAGS += $(HARDEN_CFLAGS)

include $(GBUILDDIR)/Helper.mk
include $(GBUILDDIR)/TargetLocations.mk

$(eval $(call gb_Helper_init_registries))
$(eval $(call gb_Helper_add_repositories,$(SRCDIR)))
$(eval $(call gb_Helper_collect_libtargets))

gb_Library_DLLPOSTFIX := lo

# Include platform/cpu/compiler specific config/definitions
ifneq ($(filter gbuild buidpl,$(gb_SourceEnvAndRecurse_STAGE)),)
include $(GBUILDDIR)/platform/$(OS)_$(CPUNAME)_$(COM).mk
endif

ifeq ($(CROSS_COMPILING),YES)
# We can safely Assume all cross-compilation is from Unix systems.
gb_Executable_EXT_for_build :=
else
gb_Executable_EXT_for_build := $(gb_Executable_EXT)
endif

include $(GBUILDDIR)/Tempfile.mk

include $(SRCDIR)/RepositoryFixes.mk

$(eval $(call gb_Helper_collect_knownlibs))

# add user-supplied flags
ifneq ($(strip gb__ENV_CFLAGS),)
gb_LinkTarget_CFLAGS += $(gb__ENV_CFLAGS)
endif
ifneq ($(strip gb__ENV_CXXFLAGS),)
gb_LinkTarget_CXXFLAGS += $(gb__ENV_CXXFLAGS)
endif

gb_CPUDEFS += -D$(CPUNAME)

gb_GLOBALDEFS := \
	-D_REENTRANT \
	-DOSL_DEBUG_LEVEL=$(gb_DEBUGLEVEL) \
	-DSUPD=$(UPD) \
	-DVCL \
	$(gb_OSDEFS) \
	$(gb_COMPILERDEFS) \
	$(gb_CPUDEFS) \


ifeq ($(gb_PRODUCT),$(true))
gb_GLOBALDEFS += \
	-DPRODUCT \

else
gb_GLOBALDEFS += \
	-DDBG_UTIL \
	-D_DEBUG \

endif

ifneq ($(strip $(SOLAR_JAVA)),)
gb_GLOBALDEFS += -DSOLAR_JAVA
endif

ifeq ($(gb_DEBUGLEVEL),0)
gb_GLOBALDEFS += \
	-DOPTIMIZE \
	-DNDEBUG \

else
gb_GLOBALDEFS += \
    -DSAL_LOG_INFO \
    -DSAL_LOG_WARN \

ifneq ($(gb_DEBUGLEVEL),1) # 2 or more
gb_GLOBALDEFS += \
	-DDEBUG \

endif
endif

ifneq ($(strip $(ENABLE_GTK)),)
gb_GLOBALDEFS += -DENABLE_GTK
endif

ifneq ($(strip $(ENABLE_KDE)),)
gb_GLOBALDEFS += -DENABLE_KDE
endif

ifneq ($(strip $(ENABLE_KDE4)),)
gb_GLOBALDEFS += -DENABLE_KDE4
endif

ifeq ($(strip $(ENABLE_GRAPHITE)),TRUE)
gb_GLOBALDEFS += -DENABLE_GRAPHITE
endif

ifeq ($(HAVE_THREADSAFE_STATICS),TRUE)
gb_GLOBALDEFS += -DHAVE_THREADSAFE_STATICS
endif

gb_GLOBALDEFS += $(HARDEN_CPPFLAGS)

gb_GLOBALDEFS := $(sort $(gb_GLOBALDEFS))

include $(GBUILDDIR)/Deliver.mk

$(eval $(call gb_Deliver_init))

include $(SOLARENV)/inc/minor.mk

# We are using a set of scopes that we might as well call classes.

# It is important to include them in the right order as that is
# -- at least in part -- defining precedence. This is not an issue in the
# WORKDIR as there are no nameing collisions there, but OUTDIR is a mess
# and precedence is important there. This is also platform dependant.
# For example:
# $(OUTDIR)/bin/% for executables collides
#	with $(OUTDIR)/bin/%.res for resources on unix
# $(OUTDIR)/lib/%.lib collides
#	on windows (static and dynamic libs)
# $(OUTDIR)/xml/% for packageparts collides
#	with $(OUTDIR)/xml/component/%.component for components
# This is less of an issue with GNU Make versions > 3.82 which matches for
# shortest stem instead of first match. However, upon intoduction this version
# is not available everywhere by default.

ifeq ($(gb_SourceEnvAndRecurse_STAGE),gbuild)
include $(foreach class, \
	ComponentTarget \
	ComponentsTarget \
	AllLangResTarget \
	WinResTarget \
	LinkTarget \
	Library \
	StaticLibrary \
	Executable \
	SdiTarget \
	Package \
	CustomTarget \
	ExternalLib \
	Pyuno \
	RdbTarget \
	CppunitTest \
	Jar \
	JavaClassSet \
	JunitTest \
	Module \
	UnoApiTarget \
	Zip \
	Configuration \
	Extension \
,$(GBUILDDIR)/$(class).mk)
endif

# optional extensions that should never be essential
ifneq ($(wildcard $(GBUILDDIR)/extensions/post_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/post_*.mk)
endif

define gb_Extensions_final_hook
ifneq ($(wildcard $(GBUILDDIR)/extensions/final_*.mk),)
include $(wildcard $(GBUILDDIR)/extensions/final_*.mk)
endif

endef


ifeq ($(SYSTEM_LIBXSLT_FOR_BUILD),YES)
gb_XSLTPROCTARGET :=
gb_XSLTPROC := xsltproc
else
gb_XSLTPROCTARGET := $(call gb_Executable_get_target_for_build,xsltproc)
gb_XSLTPROC := $(gb_Helper_set_ld_path) $(gb_XSLTPROCTARGET)
endif

ifeq ($(SYSTEM_PYTHON),YES)
gb_PYTHONTARGET :=
gb_PYTHON := $(PYTHON)
else ifeq ($(OS),MACOSX)
#fixme: remove this MACOSX ifeq branch by filling in gb_PYTHON_PRECOMMAND in
#gbuild/platform/macosx.mk correctly for mac, e.g. PYTHONPATH and PYTHONHOME
#dirs for in-tree internal python
gb_PYTHONTARGET :=
gb_PYTHON := $(PYTHON)
else ifeq ($(DISABLE_PYTHON),TRUE)
# Build-time python
gb_PYTHON := python
else
gb_PYTHONTARGET := $(OUTDIR)/bin/python
gb_PYTHON := $(gb_PYTHON_PRECOMMAND) $(gb_PYTHONTARGET)
endif

define gb_HelpMessage
NAME
       gbuild - GNU make based build system for LibreOffice

SYNOPSIS
       make [ -f makefile ] [ options ] [ variable=value ... ] [ targets ] ...

IMPORTANT OPTIONS
       -s Silent operation; do not print the commands as they are executed.

       -n Print the commands that would be executed, but do not execute them.
       -k Continue as much as possible after an error.

       -j Specifies the number of jobs (commands) to run simultaneously.
       -l Specifies that no new jobs (commands) should be started if there are
          others jobs running and the load average is at least load.

       -t Touch files (mark them up to date without really changing them)
          instead of running their commands.
       -W Pretend that the target file has just been modified.
       -o Do not remake the file file even if it is older than its
          dependencies, and do not remake anything on account of changes in file.

       -p Print the data base (rules and variable values) that results from
          reading the makefiles.
       --debug=b debug make run, see GNU make man page for details

       (descriptions from GNU make man page)

AVAILABLE TARGETS
       all              build product and run unit tests (default goal)
       build            build product
       unitcheck        run unit tests
       subsequentcheck  run system tests (requires full installation)
       check            run unit tests and if in toplevel subsequentcheck
       clean            remove all generated files
       showdeliverables show the targets delivered to OUTDIR and their source
       debugrun         starts the dev-install instance and allows tests to be run
                        against it

INTERACTIVE VARIABLES:
       DEBUG / debug   If not empty, build with DBGLEVEL=1 (see below).
       ENABLE_SYMBOLS / enable_symbols
                       If not empty, build with debug symbols. Automatically
                       enabled by DEBUG/debug.
       DBGLEVEL / dbglevel
                       If not empty, force the debug level to the specified value. The
                       debug level is passed to the source code through OSL_DEBUG_LEVEL
                       macro.
                       0 = no debug
                       1 = symbols + no optimizations
                       2 = symbols + no optimizations + extra debug output. OSL_TRACE
                           starts being active on this level.
                       3... = symbols + no optimizations + extra debug output (usually
                              extremely verbose). Levels > 2 are not used very much.

       YACCFLAGS       Add as flags for YACC parser generator invocation.
       CPPFLAGS        Add as preprocessor flags for C/C++/ObjC/ObjC++ compilation.
       CFLAGS          Add as compiler flags for plain C compilation.
       CXXFLAGS        Add as compiler flags for C++ compilation.
       OBJCFLAGS       Add as compiler flags for Objective C compilation.
       OBJCXXFLAGS     Add as compiler flags for Objective C++ compilation.
       LDFLAGS         Add as linker flags.
       gb_FULLDEPS     Generate and use dependencies (on by default, handle with care).
       gb_COLOR        Use ASCII color output.
       gb_TITLES       Show progress in terminal title.

endef

.PHONY: help
help:
	$(info $(gb_HelpMessage))
	@true

# vim: set noet sw=4:
