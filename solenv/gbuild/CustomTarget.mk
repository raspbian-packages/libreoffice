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

# N.B.: putting the "+" there at the start activates the GNU make job server
define gb_CustomTarget__command
+$(call gb_Helper_abbreviate_dirs,\
	mkdir -p $(call gb_CustomTarget_get_workdir,$(2)) && \
	O='$(OUTDIR)' R='$(REPODIR)' S='$(SRCDIR)' W='$(WORKDIR)' gb_AWK='$(gb_AWK)' \
	gb_SourceEnvAndRecurse_STAGE=gbuild \
	gb_XSLTPROC='$(gb_XSLTPROC)' GBUILDDIR='$(GBUILDDIR)' SRCDIR='$(SRCDIR)' \
	$(MAKE) -C $(call gb_CustomTarget_get_workdir,$(2)) -f $< && \
	touch $(1))

endef

$(call gb_CustomTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),MAK,3)
	$(call gb_CustomTarget__command,$@,$*)

.PHONY: $(call gb_CustomTarget_get_clean_target,%)
$(call gb_CustomTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),MAK,3)
	$(call gb_Helper_abbreviate_dirs,\
		rm -rf $(call gb_CustomTarget_get_workdir,$*) && \
		rm -f $(call gb_CustomTarget_get_target,$*))

define gb_CustomTarget__get_makefile
$(SRCDIR)/$(1)/Makefile
endef

define gb_CustomTarget_CustomTarget
$(call gb_CustomTarget_get_target,$(1)) : \
  $(call gb_CustomTarget__get_makefile,$(1))
endef


define gb_CustomTarget_add_dependency
$(eval $(call gb_CustomTarget_get_target,$(1)) : \
	$(SRCDIR)/$(2))
endef

define gb_CustomTarget_add_dependencies
$(foreach dependency,$(2),$(call gb_CustomTarget_add_dependency,$(1),$(dependency)))
endef

define gb_CustomTarget_add_outdir_dependency
$(eval $(call gb_CustomTarget_get_target,$(1)) : $(2))
endef

define gb_CustomTarget_add_outdir_dependencies
$(foreach dependency,$(2),$(call gb_CustomTarget_add_outdir_dependency,$(1),$(dependency)))

endef

# vim: set noet sw=4:
