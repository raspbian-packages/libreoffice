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

PRJNAME	= accessibility
PRJ		= ..$/..$/..$/..$/..
TARGET	= java_accessibility
PACKAGE	= org$/openoffice$/java$/accessibility

# --- Settings -----------------------------------------------------

.INCLUDE :  settings.mk

JAVADIR = $(OUT)$/misc$/java
JARFILES = jurt.jar unoil.jar ridl.jar
JAVAFILES = \
    logging$/XAccessibleEventLog.java \
    logging$/XAccessibleHypertextLog.java \
    logging$/XAccessibleTextLog.java \
    AbstractButton.java \
    AccessibleActionImpl.java \
    AccessibleComponentImpl.java \
    AccessibleEditableTextImpl.java \
    AccessibleExtendedState.java \
    AccessibleHypertextImpl.java \
    AccessibleIconImpl.java \
    AccessibleKeyBinding.java \
    AccessibleObjectFactory.java \
    AccessibleRoleAdapter.java \
    AccessibleSelectionImpl.java \
    AccessibleStateAdapter.java \
    AccessibleTextImpl.java \
    AccessibleValueImpl.java \
    Alert.java \
    Application.java \
    Button.java \
    CheckBox.java \
    ComboBox.java \
    Component.java \
    Container.java \
    DescendantManager.java \
    Dialog.java \
    FocusTraversalPolicy.java \
    Frame.java \
    Icon.java \
    Label.java \
    List.java \
    Menu.java \
    MenuItem.java \
    MenuContainer.java \
    NativeFrame.java \
    Paragraph.java \
    RadioButton.java \
    ScrollBar.java \
    Separator.java \
    Table.java \
    TextComponent.java \
    ToggleButton.java \
    ToolTip.java \
    Tree.java \
    Window.java

JAVACLASSFILES = $(foreach,i,$(JAVAFILES) $(CLASSDIR)$/$(PACKAGE)$/$(i:s/.java//).class) $(CLASSDIR)$/$(PACKAGE)$/Build.class

JARTARGET               = $(TARGET).jar
JARCOMPRESS             = TRUE
JARCLASSDIRS            = $(PACKAGE)

# --- Targets ------------------------------------------------------

.INCLUDE :  target.mk

# Enable logging in non-product only
.IF "$(PRODUCT)"!=""
DEBUGSWITCH = false
PRODUCTSWITCH = true
.ELSE
PRODUCTSWITCH = false
DEBUGSWITCH = true
.ENDIF

$(JAVADIR)$/$(PACKAGE)$/%.java: makefile.mk
    @@-$(MKDIRHIER) $(JAVADIR)$/$(PACKAGE)
    @-echo package org.openoffice.java.accessibility\; > $@
    @-echo public class Build { >> $@
    @-echo public static final boolean DEBUG = $(DEBUGSWITCH)\; >> $@
    @-echo public static final boolean PRODUCT = $(PRODUCTSWITCH)\; >> $@
    @-echo } >> $@

$(CLASSDIR)$/$(PACKAGE)$/Build.class : $(JAVADIR)$/$(PACKAGE)$/Build.java
    -$(JAVAC) -d $(CLASSDIR) $(JAVADIR)$/$(PACKAGE)$/Build.java

