#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

PRJ = ..$/..
PRJNAME = ucb
TARGET = cachemap
TARGETTYPE = CUI
LIBTARGET = NO
ENABLE_EXCEPTIONS = TRUE

.INCLUDE: settings.mk

OBJFILES = \
    $(OBJ)$/cachemapobject1.obj \
    $(OBJ)$/cachemapobjectcontainer2.obj \
    $(OBJ)$/cachemapobject3.obj \
    $(OBJ)$/cachemaptest.obj

APP1TARGET = $(TARGET)
APP1OBJS = $(OBJFILES)
APP1STDLIBS = \
    $(CPPULIB) \
    $(CPPUHELPERLIB) \
    $(SALLIB) \
    $(SALHELPERLIB)
APP1DEF = $(MISC)$/$(APP1TARGET).def

.INCLUDE: target.mk
