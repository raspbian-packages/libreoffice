# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
#
# Copyright 2009 by Sun Microsystems, Inc.
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
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
# GNU Lesser General Public License version 3 for more details
# (a copy is included in the LICENSE file that accompanied this code).
#
# You should have received a copy of the GNU Lesser General Public License
# version 3 along with OpenOffice.org.	If not, see
# <http://www.openoffice.org/license.html>
# for a copy of the LGPLv3 License.
#
#*************************************************************************

$(eval $(call gb_Helper_register_executables,NONE, \
    bestreversemap \
    bmp \
    bmpsum \
    g2g \
    gencoll_rule \
    genconv_dict \
    gendict \
    genindex_data \
    mkunroll \
    rsc \
    rscdep \
    saxparser \
    so_checksum \
    svidl \
    xml2cmp \
))

$(eval $(call gb_Helper_register_executables,OOO, \
    spadmin.bin \
))

ifeq ($(OS),WNT)

$(eval $(call gb_Helper_register_executables,OOO,\
	crashrep_com \
	gengal \
	guiloader \
	guistdio \
	odbcconfig \
	officeloader \
	quickstart \
	sbase \
	scalc \
	sdraw \
	simpress \
	soffice \
	soffice_bin \
	smath \
	sweb \
	swriter \
	unoinfo \
	unopkg \
	unopkg_bin \
	unopkg_com \
	unopkgio \
))

else

$(eval $(call gb_Helper_register_executables,OOO,\
	soffice.bin \
	unopkg.bin \
	gengal.bin \
))

ifeq ($(OS),MACOSX)

$(eval $(call gb_Helper_register_executables,OOO,\
	officeloader \
))

else

$(eval $(call gb_Helper_register_executables,OOO,\
	oosplash \
))

endif

endif

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
	PptImporter \
	adabas \
	adabasui \
	agg \
	analysis \
	animcore \
	basctl \
	basebmp \
	canvastools \
	communi \
	date \
	evtatt \
	forui \
	odbc \
	odbcbase \
	oox \
	ooxml \
	sdbc \
    AppleRemote \
    avmedia \
	avmediaQuickTime \
    basegfx \
    chartcontroller \
    chartmodel \
    charttools \
    chartview \
    cppcanvas \
    cui \
    dba \
    dbaxml \
    dbmm \
    dbtools \
    dbu \
    deploymentmisc \
    desktop_detector \
    doctok \
    drawinglayer \
    editeng \
	egi \
	eme \
	epb \
	epg \
	epp \
	eps \
	ept \
	era \
	eti \
	exp \
	filterconfig \
	filtertracer \
	flash \
    for \
    frm \
    fwe \
    fwi \
    fwk \
    fwl \
    fwm \
    helplinker \
    hwp \
    i18npaper \
    i18nregexp \
	icd \
	icg \
	idx \
	ime \
	ipb \
	ipd \
	ips \
	ipt \
	ipx \
	ira \
	itg \
	iti \
    lng \
    lwpft \
    merged \
    msfilter \
    msword \
	msworks \
	odfflatxml \
    offacc \
	pdffilter \
	placeware \
	protocolhandler \
    qstart_gtk \
    resourcemodel \
    rpt \
    rptui \
    rptxml \
	rtftok \
    sax \
    sb \
    sc \
    scd \
    scfilt \
    scui \
    sd \
    sdbt \
    sdd \
    sdfilt \
    sdui \
    sfx \
    simplecm \
    sm \
    smd \
	solver \
    sot \
    spa \
    spl \
    sts \
	svgfilter \
    svl \
    svt \
    svx \
    svxcore \
    sw \
    swd \
    swui \
	t602filter \
    textconversiondlgs \
    tk \
    tl \
    unordf \
    unoxml \
    utl \
	uui \
    vbahelper \
    vcl \
    vclplug_gen \
    vclplug_gtk \
    vclplug_gtk3 \
    vclplug_kde \
    vclplug_kde4 \
    vclplug_svp \
	visioimport \
	wpft \
	wpgimport \
    writerfilter \
    xcr \
	xmlfa \
	xmlfd \
    xo \
    xof \
	xsltdlg \
	xsltfilter \
))

ifeq ($(filter MACOSX WNT,$(OS)),)

$(eval $(call gb_Helper_register_libraries,OOOLIBS, \
    spl_unx \
))

endif

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_URE, \
	xmlreader \
))

$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	avmediagst \
	avmediawin \
	collator_data \
	curl \
	deployment \
	deploymentgui \
	dict_ja \
	dict_zh \
	fileacc \
	index_data \
	localedata_en \
	localedata_es \
	localedata_euro \
	localedata_others \
	mcnttype \
	officebean \
	neon \
	package2 \
    reg \
	scriptframe \
    sofficeapp \
    store \
	test \
	unotest \
	textconv_dict \
    ucb \
    ucpfile \
    unopkgapp \
	xmlsecurity \
	xsec_fw \
	xsec_xmlsec \
	xstor \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	dnd \
	dtrans \
	ftransl \
	sysdtrans \
))
endif

ifeq ($(OS),IOS)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
    gcc3_uno \
    sal_textenc \
))
endif

$(eval $(call gb_Helper_register_libraries,RTLIBS, \
    comphelper \
    i18nisolang1 \
    i18nutil \
    jvmaccess \
    ucbhelper \
))

$(eval $(call gb_Helper_register_libraries,RTVERLIBS, \
    cppuhelper \
    salhelper \
))

$(eval $(call gb_Helper_register_libraries,UNOLIBS_OOO, \
	basprov \
	bootstrap \
	cairocanvas \
	canvasfactory \
	configmgr \
	directx5canvas \
	directx9canvas \
	dlgprov \
	expwrap \
	fastsax \
    fsstorage \
	gdipluscanvas \
    hatchwindowfactory \
	i18npool \
	i18nsearch \
    lomenubar \
    migrationoo2 \
    migrationoo3 \
    msforms \
	mtfrenderer \
	nullcanvas \
    OGLTrans \
    passwordcontainer \
    reflection \
	simplecanvas \
    slideshow \
    stocservices \
	stringresource \
	vbaevents \
    vbaobj \
    vbaswobj \
	vclcanvas \
    writerfilter_uno \
	writerfilter_debug \
))

$(eval $(call gb_Helper_register_libraries,UNOLIBS_URE, \
	binaryurp \
))

$(eval $(call gb_Helper_register_libraries,UNOVERLIBS, \
    cppu \
    jvmfwk \
    sal \
))

$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	basegfx_s \
    dtobj \
    libeay32 \
    ssleay32 \
    ooopathutils \
	sample \
    salcpprt \
	vclmain \
	writerperfect \
))

ifeq ($(OS),WNT)
$(eval $(call gb_Helper_register_libraries,PLAINLIBS_OOO, \
	xmlsec1 \
	xmlsec1-nss \
	xmlsec1-mscrypto \
))
else
$(eval $(call gb_Helper_register_static_libraries,PLAINLIBS, \
	xmlsec1 \
	xmlsec1-nss \
	xmlsec1-mscrypto \
))
endif

# vim: set noet sw=4 ts=4:
