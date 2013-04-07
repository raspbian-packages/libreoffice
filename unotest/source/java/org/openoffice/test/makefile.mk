#*************************************************************************
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
#***********************************************************************/

PRJ = ../../../../..
PRJNAME = test
TARGET = test

.IF "$(OOO_JUNIT_JAR)" != ""

PACKAGE = org/openoffice/test
JAVAFILES = \
    Argument.java \
    FileHelper.java \
    OfficeConnection.java \
    OfficeFileUrl.java \
    TestArgument.java \
    UnoApiTest.java \

JARFILES = juh.jar ridl.jar unoil.jar OOoRunner.jar
EXTRAJARFILES = $(OOO_JUNIT_JAR)

JARTARGET        = $(TARGET).jar
JARCLASSDIRS     = $(PACKAGE)
JARCLASSEXCLUDES = $(PACKAGE)/tools/*
JARCLASSPATH     = $(JARFILES)
 # expect $(OOO_JUNIT_JAR) to be on CLASSPATH wherever test.jar is used (also,
 # on Windows, $(OOO_JUNIT_JAR) could be an absolute pathname with drive letter
 # like X:/path and some JVMs would refuse to load test.jar if its MANIFEST.MF
 # Class-Path contained such a pathname that looks like an unknown URL with
 # scheme X)

.END

.INCLUDE: settings.mk
.INCLUDE: target.mk
