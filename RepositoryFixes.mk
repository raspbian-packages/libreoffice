# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
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

# fixes for all the libraries that are named with too much creativity and do
# not follow any of the established nameschemes

# Make has no support for 'or' clauses in conditionals,
# we use a filter expression instead.
ifneq (,$(filter SOLARIS GCC,$(OS) $(COM)))
gb_Library_FILENAMES := $(patsubst comphelper:libcomphelper%,comphelper:libcomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppuhelper:libcppuhelper%,cppuhelper:libuno_cppuhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst jvmfwk:libuno_jvmfwk%,jvmfwk:libjvmfwk%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst salhelper:libsalhelper%,salhelper:libuno_salhelper%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ucbhelper:libucbhelper%,ucbhelper:libucbhelper4%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ucb:libucb%,ucb:libucb1%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ucpfile:libucpfile%,ucpfile:libucpfile1%,$(gb_Library_FILENAMES))
endif

ifeq ($(OS),WNT)
gb_Library_DLLFILENAMES := $(patsubst comphelper:comphelper%,comphelper:comphelp%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst icuuc:icuuc%,icuuc:icuuc40%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ucbhelper:ucbhelper%,ucbhelper:ucbhelper4%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ucb:libucb%,ucb:libucb1%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ucpfile:libucpfile%,ucpfile:libucpfile1%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_DLLFILENAMES))

gb_Library_FILENAMES := $(patsubst sb:isb%,sb:basic%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst sfx:isfx%,sfx:sfx%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst svt:isvt%,svt:svtool%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst tl:itl%,tl:itools%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst vbahelper:ivbahelper%,vbahelper:vbahelper%,$(gb_Library_FILENAMES))

gb_StaticLibrary_FILENAMES := $(patsubst graphite:graphite%,graphite:graphite_dll%,$(gb_StaticLibrary_FILENAMES))

ifeq ($(COM),GCC)
gb_Library_FILENAMES := $(patsubst crypto:icrypto%,crypto:crypto%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst graphite2_off:%.dll.a,graphite2_off:%.a,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:irdf%,rdf:librdf.dll$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:issl%,ssl:ssl%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:libxml2$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt$(gb_Library_IARCEXT),$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst z:iz%,z:zlib%,$(gb_Library_FILENAMES))


# Libraries not provided by mingw(-w64), available only in the Windows
# SDK. So if these actually are liked with somewhere, we can't
# cross-compile that module then using MinGW. That needs to be fixed
# then, and we need to use these libraries through run-time lookup of
# APIs. Or something.
gb_Library_SDKLIBFILENAMES:=

gb_Library_DLLFILENAMES := $(filter-out $(foreach lib,$(gb_Library_SDKLIBFILENAMES),$(lib):%),$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES += $(foreach lib,$(gb_Library_SDKLIBFILENAMES),$(lib):$(WINDOWS_SDK_HOME)/lib/$(lib).lib)

gb_Library_DLLFILENAMES := $(patsubst crypto:crypto%,crypto:libeay32%,$(gb_Library_DLLFILENAMES))
gb_Library_DLLFILENAMES := $(patsubst ssl:ssl%,ssl:ssleay32%,$(gb_Library_DLLFILENAMES))

else # $(COM) != GCC

gb_Library_FILENAMES := $(patsubst cairo:icairo%,cairo:cairo%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst comphelper:icomphelper%,comphelper:icomphelp%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst cppunit:icppunit%,cppunit:icppunit_dll%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst crypto:icrypto%,crypto:libeay32%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst crypto:libcrypto%,crypto:libcrypto_static%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst db47:idb47%,db47:libdb47%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst i18nisolang1:ii18nisolang1%,i18nisolang1:ii18nisolang%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst lpsolve55:ilpsolve55%,lpsolve55:lpsolve55%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst package2:ipackage2%,package2:ipackage%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst rdf:irdf%,rdf:librdf%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:issl%,ssl:ssleay32%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst ssl:libssl%,ssl:libssl_static%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xml2:ixml2%,xml2:libxml2%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xmlsec1:ixmlsec1%,xmlsec1:libxmlsec%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xmlsec1-mscrypto:ixmlsec1-mscrypto%,xmlsec1-mscrypto:libxmlsec-mscrypto%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xmlsec1-nss:ixmlsec1-nss%,xmlsec1-nss:libxmlsec-nss%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst xslt:ixslt%,xslt:libxslt%,$(gb_Library_FILENAMES))
gb_Library_FILENAMES := $(patsubst z:z%,z:zlib%,$(gb_Library_FILENAMES))


# change the names of all import libraries that don't have an "i" prefix as in our standard naming schema
gb_Library_NOILIBFILENAMES := $(gb_Library_PLAINLIBS_NONE)
gb_Library_NOILIBFILENAMES += \
    graphite2_off \
	icudt icuin icule icuuc \
    msvcprt \
    nspr4 \
    nss3 \
    shlwapi \
    version \

gb_Library_FILENAMES := $(filter-out $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):%),$(gb_Library_FILENAMES))
gb_Library_FILENAMES += $(foreach lib,$(gb_Library_NOILIBFILENAMES),$(lib):$(lib)$(gb_Library_PLAINEXT))

ifneq ($(gb_PRODUCT),$(true))
gb_Library_FILENAMES := $(patsubst msvcrt:msvcrt%,msvcrt:msvcrtd%,$(gb_Library_FILENAMES))
endif

endif # ifeq ($(COM),GCC)

endif # ifeq ($(OS),WNT)

# vim: set noet sw=4 ts=4:
