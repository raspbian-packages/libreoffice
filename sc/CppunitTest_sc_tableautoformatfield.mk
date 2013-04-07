# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Markus Mohrhard <markus.mohrhard@googlemail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#*************************************************************************

$(eval $(call gb_CppunitTest_CppunitTest,sc_tableautoformatfield))

$(eval $(call gb_CppunitTest_add_exception_objects,sc_tableautoformatfield, \
    sc/qa/extras/tableautoformatfield \
))

$(eval $(call gb_CppunitTest_add_linked_libs,sc_tableautoformatfield, \
    avmedia \
    basegfx \
    comphelper \
    cppu \
    cppuhelper \
    drawinglayer \
    editeng \
    fileacc \
    for \
    forui \
    i18nisolang1 \
    msfilter \
    oox \
    sal \
    salhelper \
    sax \
    sb \
    sc \
    sfx \
    sot \
    svl \
    svt \
    svx \
    svxcore \
	test \
    tl \
    tk \
    ucbhelper \
	unotest \
    utl \
    vbahelper \
    vcl \
    xo \
	$(gb_STDLIBS) \
))

$(eval $(call gb_CppunitTest_set_include,sc_tableautoformatfield,\
    -I$(realpath $(SRCDIR)/sc/source/ui/inc) \
    -I$(realpath $(SRCDIR)/sc/inc) \
    $$(INCLUDE) \
    -I$(OUTDIR)/inc \
))

$(eval $(call gb_CppunitTest_add_api,sc_tableautoformatfield,\
    offapi \
    udkapi \
))

$(eval $(call gb_CppunitTest_uses_ure,sc_tableautoformatfield))

$(eval $(call gb_CppunitTest_add_type_rdbs,sc_tableautoformatfield,\
    types \
))

$(eval $(call gb_CppunitTest_add_components,sc_tableautoformatfield,\
    basic/util/sb \
    comphelper/util/comphelp \
    dbaccess/util/dba \
    fileaccess/source/fileacc \
    filter/source/config/cache/filterconfig1 \
    forms/util/frm \
    framework/util/fwk \
    i18npool/util/i18npool \
    oox/util/oox \
    package/source/xstor/xstor \
    package/util/package2 \
    sax/source/expatwrap/expwrap \
    sax/source/fastparser/fastsax \
    sc/util/sc \
    sc/util/scd \
    sc/util/scfilt \
    sc/util/vbaobj \
    scripting/source/basprov/basprov \
    scripting/util/scriptframe \
    sfx2/util/sfx \
    sot/util/sot \
    svl/source/fsstor/fsstorage \
    toolkit/util/tk \
    unotools/util/utl \
    unoxml/source/rdf/unordf \
    unoxml/source/service/unoxml \
))

$(eval $(call gb_CppunitTest_add_old_components,sc_tableautoformatfield,\
    configmgr \
    ucb1 \
    ucpfile1 \
    ucptdoc1 \
))

$(eval $(call gb_CppunitTest_set_args,sc_tableautoformatfield,\
    --headless \
    --protector unoexceptionprotector$(gb_Library_DLLEXT) unoexceptionprotector \
    "-env:CONFIGURATION_LAYERS=xcsxcu:$(call gb_CppunitTarget__make_url,$(OUTDIR)/xml/registry) module:$(call gb_CppunitTarget__make_url,$(OUTDIR)/xml/registry/spool) xcsxcu:$(call gb_CppunitTarget__make_url,$(OUTDIR)/unittest/registry)" \
))
    # .../spool is required for the (somewhat strange) filter configuration

# we need to
# a) explicitly depend on library msword because it is not implied by a link
#    relation
# b) explicitly depend on the sc resource files needed at unit-test runtime
$(call gb_CppunitTest_get_target,sc_tableautoformatfield) : $(call gb_Library_get_target,scfilt) $(WORKDIR)/AllLangRes/sc

# vim: set noet sw=4 ts=4:
