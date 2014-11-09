# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# for VERSION
include $(SRCDIR)/external/jfreereport/version.mk

$(eval $(call gb_ExternalPackage_ExternalPackage,jfreereport_libformula,jfreereport_libformula))

$(eval $(call gb_ExternalPackage_use_external_project,jfreereport_libformula,jfreereport_libformula))

$(eval $(call gb_ExternalPackage_add_file,jfreereport_libformula,$(LIBO_SHARE_JAVA_FOLDER)/libformula-$(LIBFORMULA_VERSION).jar,dist/libformula-$(LIBFORMULA_VERSION).jar))

# vim: set noet sw=4 ts=4:
