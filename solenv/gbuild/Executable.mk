# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
# 
# Copyright 2000, 2011 Oracle and/or its affiliates.
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

# Executable class

# defined by platform
#  gb_Executable_Executable_platform

.PHONY : $(call gb_Executable_get_clean_target,%)
$(call gb_Executable_get_clean_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_Executable_get_target,$*) \
			$(AUXTARGETS))

$(call gb_Executable_get_target,%) :
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Deliver_deliver,$<,$@) \
			$(foreach target,$(AUXTARGETS), && $(call gb_Deliver_deliver,$(dir $<)/$(notdir $(target)),$(target))))

define gb_Executable_Executable
ifeq (,$$(findstring $(1),$$(gb_Executable_KNOWN)))
$$(eval $$(call gb_Output_info,Currently known executables: $(sort $(gb_Executable_KNOWN)),ALL))
$$(eval $$(call gb_Output_error,Executable $(1) must be registered in Repository.mk))
endif
$(call gb_Executable__Executable_impl,$(1),Executable/$(1)$(gb_Executable_EXT))

endef

define gb_Executable__Executable_impl
$(call gb_Executable_set_targettype_gui,$(2))
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype,$(2),Executable)
$(call gb_Executable_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_Executable_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_Executable_Executable_platform,$(1),$(2))
$$(eval $$(call gb_Module_register_target,$(call gb_Executable_get_target,$(1)),$(call gb_Executable_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_Executable_get_target,$(1)),$(call gb_LinkTarget_get_target,$(2)),$(1))

endef

define gb_Executable_set_targettype_gui
$(call gb_LinkTarget_get_target,Executable/$(1)$(gb_Executable_EXT)) : TARGETGUI := $(2)
endef


define gb_Executable_forward_to_Linktarget
gb_Executable_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_Executable_get_linktargetname,$$(1)),$$(2),$$(3))

endef

$(eval $(foreach method,\
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_objcobject \
	add_objcobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_grammar \
	add_grammars \
	add_exception_objects \
	add_noexception_objects \
	add_generated_cobjects \
	add_generated_cxxobjects \
	add_generated_exception_objects \
	add_cflags \
	set_cflags \
	add_cxxflags \
	set_cxxflags \
	add_objcxxflags \
	set_objcflags \
	set_objcxxflags \
	add_defs \
	set_defs \
	set_include \
	add_ldflags \
	set_ldflags \
	add_libs \
	set_library_path_flags \
	add_api \
	add_linked_libs \
	add_linked_static_libs \
	use_external \
	use_externals \
	add_package_headers \
	add_sdi_headers \
	add_nativeres \
,\
	$(call gb_Executable_forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4:
