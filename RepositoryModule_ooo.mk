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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org. If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Module_Module,ooo))

$(eval $(call gb_Module_add_moduledirs,ooo,\
    animations \
    basctl \
    basebmp \
    basegfx \
    basic \
    bean \
    canvas \
    chart2 \
    comphelper \
    cppcanvas \
    cui \
    dbaccess \
    desktop \
    drawinglayer \
    dtrans \
    editeng \
    eventattacher \
    fileaccess \
    filter \
    forms \
    formula \
    framework \
    hwpfilter \
    i18npool \
    i18nutil \
    idl \
    linguistic \
    lotuswordpro \
    MathMLDTD \
    Mesa \
    nlpsolver \
    o3tl \
    offapi \
    oovbaapi \
    oox \
    package \
    padmin \
    psprint_config \
    regexp \
    reportdesign \
    sane \
    sax \
    sc \
    scaddins \
    sccomp \
    scripting \
    sd \
    sfx2 \
    slideshow \
    sot \
    starmath \
    svl \
    svtools \
    svx \
    sw \
    swext \
    twain \
    toolkit \
    tools \
    ucbhelper \
    udkapi \
    unixODBC \
    unotools \
    unoxml \
    uui \
    vbahelper \
    vcl \
    wizards \
    writerfilter \
    writerperfect \
    xmerge \
    xmlsecurity \
    xml2cmp \
    xmloff \
    xmlreader \
    xmlscript \
))

ifneq ($(SOLAR_JAVA),)
 ifneq "$(OOO_JUNIT_JAR)" ""
$(eval $(call gb_Module_add_moduledirs,ooo,\
    qadevOOo \
))
  endif
endif

# these have only migrated subsequentcheck for now
$(eval $(call gb_Module_add_moduledirs,ooo,\
    configmgr \
    sal \
    ucb \
))

# vim: set noet ts=4 sw=4:
