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
#***********************************************************************/

PRJ = ..
PRJNAME = postprocess
TARGET = packcomponents

.INCLUDE: settings.mk

my_components = \
    cached1 \
    calc \
    component/animations/source/animcore/animcore \
    component/avmedia/util/avmedia \
    component/basctl/util/basctl \
    component/basic/util/sb \
    component/chart2/source/controller/chartcontroller \
    component/chart2/source/model/chartmodel \
    component/chart2/source/tools/charttools \
    component/chart2/source/view/chartview \
    component/canvas/source/factory/canvasfactory \
    component/canvas/source/simplecanvas/simplecanvas \
    component/canvas/source/vcl/vclcanvas \
    component/comphelper/util/comphelp \
	component/cppcanvas/source/uno/mtfrenderer \
    component/cui/util/cui \
    component/dbaccess/source/ext/macromigration/dbmm \
    component/dbaccess/source/filter/xml/dbaxml \
    component/dbaccess/util/dba \
    component/dbaccess/util/dbu \
    component/dbaccess/util/sdbt \
    component/dtrans/util/mcnttype \
    component/eventattacher/source/evtatt \
    component/fileaccess/source/fileacc \
    component/filter/source/config/cache/filterconfig1 \
    component/filter/source/flash/flash \
    component/filter/source/msfilter/msfilter \
    component/filter/source/odfflatxml/odfflatxml \
    component/filter/source/pdf/pdffilter \
    component/filter/source/placeware/placeware \
    component/filter/source/svg/svgfilter \
    component/filter/source/t602/t602filter \
    component/filter/source/xmlfilteradaptor/xmlfa \
    component/filter/source/xmlfilterdetect/xmlfd \
    component/filter/source/xsltdialog/xsltdlg \
    component/filter/source/xsltfilter/xsltfilter \
    component/forms/util/frm \
    component/formula/util/for \
    component/framework/util/fwk \
    component/framework/util/fwl \
    component/framework/util/fwm \
    component/hwpfilter/source/hwp \
    component/i18npool/source/search/i18nsearch \
    component/i18npool/util/i18npool \
    component/linguistic/source/lng \
    component/lotuswordpro/util/lwpfilter \
    component/oox/util/oox \
    component/package/source/xstor/xstor \
    component/package/util/package2 \
    component/reportdesign/util/rpt \
    component/reportdesign/util/rptui \
    component/reportdesign/util/rptxml \
    component/sax/source/expatwrap/expwrap \
    component/sax/source/fastparser/fastsax \
    component/sc/util/sc \
    component/sc/util/scd \
    component/sc/util/scfilt \
    component/sc/util/vbaobj \
    component/scaddins/source/analysis/analysis \
    component/scaddins/source/datefunc/date \
    component/sccomp/source/solver/solver \
    component/scripting/source/basprov/basprov \
    component/scripting/source/dlgprov/dlgprov \
    component/scripting/source/protocolhandler/protocolhandler \
    component/scripting/source/pyprov/mailmerge \
    component/scripting/source/stringresource/stringresource \
    component/scripting/source/vbaevents/vbaevents \
    component/scripting/util/scriptframe \
    component/sd/util/sd \
    component/sd/util/sdd \
    component/sd/util/sdfilt \
    component/sfx2/util/sfx \
    component/slideshow/util/slideshow \
    component/sot/util/sot \
    component/starmath/util/sm \
    component/starmath/util/smd \
    component/svl/source/fsstor/fsstorage \
    component/svl/source/passwordcontainer/passwordcontainer \
    component/svl/util/svl \
    component/svtools/source/hatchwindow/hatchwindowfactory \
    component/svtools/util/svt \
    component/svx/util/svx \
    component/svx/util/svxcore \
    component/svx/util/textconversiondlgs \
    component/sw/util/msword \
    component/sw/util/sw \
    component/sw/util/swd \
    component/sw/util/vbaswobj \
    component/toolkit/util/tk \
    component/unotools/util/utl \
    component/unoxml/source/rdf/unordf \
    component/unoxml/source/service/unoxml \
    component/uui/util/uui \
    component/vbahelper/util/msforms \
    component/writerfilter/util/writerfilter \
    component/writerperfect/util/msworksfilter \
    component/writerperfect/util/visiofilter \
    component/writerperfect/util/wpft \
    component/writerperfect/util/wpgfilter \
    component/xmloff/source/transform/xof \
    component/xmloff/util/xo \
    component/xmlscript/util/xcr \
    component/xmlsecurity/util/xmlsecurity \
    component/xmlsecurity/util/xsec_fw \
    configmgr \
    ctl \
    dbase \
    dbpool2 \
    dbtools \
    embobj \
    flat \
    fpicker \
    fps_office \
    guesslang \
    hyphen \
    lnth \
    localebe1 \
    mysql \
    odbc \
    sdbc2 \
    spell \
    srtrs1 \
    syssh \
    ucb1 \
    ucpexpand1 \
    ucpext \
    ucpfile1 \
    ucpftp1 \
    ucphier1 \
    ucppkg1 \
    ucptdoc1 \
	ucpcmis1 \

.IF "$(BUILD_TYPE)" != "$(BUILD_TYPE:s/DESKTOP//)"
my_components += \
    abp \
    bib \
    component/desktop/source/deployment/deployment \
    component/desktop/source/deployment/gui/deploymentgui \
    component/desktop/source/migration/services/migrationoo2 \
    component/desktop/source/migration/services/migrationoo3 \
    component/desktop/source/offacc/offacc \
    component/desktop/source/splash/spl \
    dbp \
    log \
    oooimprovecore \
    pcr \
    res \
    scn \
    tvhlp1 \
    ucpchelp1 \
    updatefeed \
    updchk \
    updchk.uno \
    xmx
.ENDIF

.IF "$(DISABLE_PYTHON)" != "TRUE"
my_components += pythonloader
.ENDIF

.IF "$(OS)" != "WNT" && "$(OS)" != "MACOSX" && "$(OS)" != "IOS" && "$(OS)" != "ANDROID"
my_components += component/desktop/unx/splash/splash
.ENDIF

.IF "$(DISABLE_ATL)" == ""
my_components += emboleobj
.END

.IF "$(DISABLE_NEON)" != "TRUE"
my_components += ucpdav1
.END

.IF "$(ENABLE_CAIRO_CANVAS)" == "TRUE"
my_components += component/canvas/source/cairo/cairocanvas
.END

.IF "$(ENABLE_GCONF)" != ""
my_components += gconfbe1
.END

.IF "$(ENABLE_GIO)" != ""
my_components += ucpgio
.END

.IF "$(ENABLE_GNOMEVFS)" != ""
my_components += ucpgvfs
.END

.IF "$(ENABLE_KAB)" != ""
my_components += kab1
.END

.IF "$(ENABLE_KDE)" != ""
my_components += kdebe1
.END

.IF "$(ENABLE_KDE4)" != ""
my_components += kde4be1
.END

.IF "$(ENABLE_OPENGL)" == "TRUE"
my_components += component/slideshow/source/engine/OGLTrans/ogltrans
.END

.IF "$(ENABLE_LOMENUBAR)" == "TRUE"
my_components += component/framework/util/lomenubar
.END

.IF "$(SOLAR_JAVA)" == "TRUE"
my_components += \
    LuceneHelpWrapper \
    component/xmerge/source/bridge/XMergeBridge \
    component/filter/source/xsltfilter/XSLTFilter.jar \
    component/filter/source/xsltvalidate/XSLTValidate \
    component/scripting/java/ScriptFramework \
    component/scripting/java/ScriptProviderForJava \
    component/wizards/com/sun/star/wizards/agenda/agenda \
    component/wizards/com/sun/star/wizards/fax/fax \
    component/wizards/com/sun/star/wizards/form/form \
    hsqldb \
    jdbc \
    component/wizards/com/sun/star/wizards/letter/letter \
    component/wizards/com/sun/star/wizards/query/query \
    component/wizards/com/sun/star/wizards/report/report \
    component/wizards/com/sun/star/wizards/table/table \
    component/wizards/com/sun/star/wizards/web/web
.END

.IF "$(WITH_BINFILTER)" != "NO"
my_components += \
    bf_migratefilter \
    bindet
.END

.IF "$(WITH_LDAP)" == "YES"
my_components += ldapbe2
.END

.IF "$(WITH_MOZILLA)" != "NO"
my_components += pl
.END

.IF "$(ENABLE_XMLSEC)" == "YES"
.IF "$(OS)" == "WNT"
my_components += component/xmlsecurity/util/xsec_xmlsec.windows
.ELSE
my_components += component/xmlsecurity/util/xsec_xmlsec
.END
.END

.IF "$(OS)" == "MACOSX"
my_components += \
    MacOSXSpell \
    fps_aqua \
    macab1 \
    macbe1 \
    component/vcl/vcl.macosx
.END

.IF "$(GUIBASE)" == "aqua"
my_components += component/avmedia/source/quicktime/avmediaQuickTime
.END

.IF "$(OS)" == "WNT"
my_components += \
    ado \
    fop \
    fps \
    java_uno_accessbridge \
    smplmail \
    wininetbe1 \
    component/dtrans/source/generic/dtrans \
    component/dtrans/util/dnd \
    component/dtrans/util/ftransl \
    component/dtrans/util/sysdtrans \
    component/vcl/vcl.windows
.END

.IF "$(OS)" != "MACOSX" && "$(OS)" != "WNT" && "$(OS)" != "IOS" && "$(OS)" != "ANDROID"
my_components += \
    desktopbe1 \
    component/vcl/vcl.unx
.END

.IF "$(OS)" == "WNT" && "$(DISABLE_ATL)" == ""
my_components += \
    emser \
    oleautobridge
.END

.IF "$(OS)" == "WNT" && "$(ENABLE_DIRECTX)" != ""
my_components += \
    component/avmedia/source/win/avmediawin \
    component/canvas/source/directx/directx9canvas \
    component/canvas/source/directx/gdipluscanvas
.END

.IF "$(OS)" == "WNT" && "$(ENABLE_DIRECTX)" != "" && "$(USE_DIRECTX5)" != ""
my_components += component/canvas/source/directx/directx5canvas
.END

.IF "$(OS)" != "MACOSX" && "$(SYSTEM_MOZILLA)" != "YES" && \
    "$(WITH_MOZILLA)" != "NO"
my_components += mozab
.ELSE
my_components += mozbootstrap
.END

.IF "$(OS)" != "MACOSX" && "$(OS)" != "WNT" && "$(ENABLE_KDE4)" != ""
my_components += fps_kde4
.END

.IF "$(OS)" != "WNT"
my_components += cmdmail
.END

.IF "$(OS)" != "WNT" && "$(ENABLE_EVOAB2)" != ""
my_components += evoab
.END

.IF "$(OS)" != "WNT" && "$(ENABLE_GSTREAMER)" != ""
my_components += component/avmedia/source/gstreamer/avmediagstreamer
.END

.INCLUDE: target.mk

ALLTAR : $(MISC)/services.rdb

$(MISC)/services.rdb .ERRREMOVE : $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/services.input $(my_components:^"$(SOLARXMLDIR)/":+".component")
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt $(MISC)/services.input

$(MISC)/services.input : makefile.mk
    echo \
        '<list>$(my_components:^"<filename>":+".component</filename>")</list>' \
        > $@

.IF "$(ENABLE_SCRIPTING_BEANSHELL)" == "YES"
ALLTAR : $(MISC)/scriptproviderforbeanshell.rdb

$(MISC)/scriptproviderforbeanshell.rdb .ERRREMOVE : \
        $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/scriptproviderforbeanshell.input \
        $(SOLARXMLDIR)/component/scripting/java/ScriptProviderForBeanShell.component
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/scriptproviderforbeanshell.input

$(MISC)/scriptproviderforbeanshell.input : makefile.mk
    echo \
        '<list><filename>component/scripting/java/ScriptProviderForBeanShell.component</filename></list>' \
        > $@
.END

.IF "$(ENABLE_SCRIPTING_JAVASCRIPT)" == "YES"
ALLTAR : $(MISC)/scriptproviderforjavascript.rdb

$(MISC)/scriptproviderforjavascript.rdb .ERRREMOVE : \
        $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/scriptproviderforjavascript.input \
        $(SOLARXMLDIR)/component/scripting/java/ScriptProviderForJavaScript.component
    $(XSLTPROC) --nonet --stringparam prefix $(SOLARXMLDIR)/ -o $@ \
        $(SOLARENV)/bin/packcomponents.xslt \
        $(MISC)/scriptproviderforjavascript.input

$(MISC)/scriptproviderforjavascript.input : makefile.mk
    echo \
        '<list><filename>component/scripting/java/ScriptProviderForJavaScript.component</filename></list>' \
        > $@
.END
