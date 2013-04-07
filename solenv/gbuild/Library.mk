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


# Library class

# defined globally in TargetLocations.mk
#  gb_Library_OUTDIRLOCATION := $(OUTDIR)/lib
#  gb_Library_DLLDIR := $(WORKDIR)/LinkTarget/Library
# defined by platform
#  gb_Library_DEFS
#  gb_Library_DLLFILENAMES
#  gb_Library_FILENAMES
#  gb_Library_Library_platform
#  gb_Library_TARGETS

# doesn't do anything, just used for hooking up component target
.PHONY: $(call gb_Library__get_final_target,%)

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
.PHONY : $(WORKDIR)/Clean/OutDir/lib/%$(gb_Library_PLAINEXT)
$(WORKDIR)/Clean/OutDir/lib/%$(gb_Library_PLAINEXT) :
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(OUTDIR)/lib/$*$(gb_Library_PLAINEXT) \
			$(AUXTARGETS))

# EVIL: gb_StaticLibrary and gb_Library need the same deliver rule because they are indistinguishable on windows
$(gb_Library_OUTDIRLOCATION)/%$(gb_Library_PLAINEXT) : 
	$(call gb_Helper_abbreviate_dirs,\
		$(call gb_Deliver_deliver,$<,$@) \
			$(foreach target,$(AUXTARGETS), && $(call gb_Deliver_deliver,$(dir $<)/$(notdir $(target)),$(target))))

define gb_Library_Library
ifeq (,$$(findstring $(1),$$(gb_Library_KNOWNLIBS)))
$$(eval $$(call gb_Output_info,Currently known libraries are: $(sort $(gb_Library_KNOWNLIBS)),ALL))
$$(eval $$(call gb_Output_error,Library $(1) must be registered in Repository.mk))
endif
$(call gb_Library_get_target,$(1)) : AUXTARGETS :=
$(call gb_Library__Library_impl,$(1),$(call gb_Library_get_linktargetname,$(1)))

endef

define gb_Library__Library_impl
$(call gb_LinkTarget_LinkTarget,$(2))
$(call gb_LinkTarget_set_targettype,$(2),Library)
$(call gb_LinkTarget_add_defs,$(2),\
	$(gb_Library_DEFS) \
)
$(call gb_Library__get_final_target,$(1)) : $(call gb_Library_get_target,$(1))
$(call gb_Library_get_target,$(1)) : $(call gb_LinkTarget_get_target,$(2))
$(call gb_Library_get_clean_target,$(1)) : $(call gb_LinkTarget_get_clean_target,$(2))
$(call gb_Library_Library_platform,$(1),$(2),$(gb_Library_DLLDIR)/$(call gb_Library_get_dllname,$(1)))
$$(eval $$(call gb_Module_register_target,$(call gb_Library__get_final_target,$(1)),$(call gb_Library_get_clean_target,$(1))))
$(call gb_Deliver_add_deliverable,$(call gb_Library_get_target,$(1)),$(call gb_LinkTarget_get_target,$(2)),$(1))

endef

# The dependency from workdir component target to outdir library should ensure
# that gb_CppunitTest_add_component can transitively depend on the library.
# But the component target also must be delivered, so a new phony target
# gb_Library__get_final_target has been invented for that purpose...
define gb_Library_set_componentfile
$(call gb_ComponentTarget_ComponentTarget,$(2),$(call gb_Library__get_componentprefix,$(1)),\
	$(call gb_Library_get_runtime_filename,$(if $(MERGELIBS),$(if $(filter $(gb_MERGED_LIBS),$(1)),merged,$(1)),$(1))))
$(call gb_Library__get_final_target,$(1)) : \
	$(call gb_ComponentTarget_get_outdir_target,$(2))
$(call gb_ComponentTarget_get_target,$(2)) :| $(call gb_Library_get_target,$(1))
$(call gb_Library_get_clean_target,$(1)) : \
	$(call gb_ComponentTarget_get_clean_target,$(2))
endef

gb_Library__get_componentprefix = \
	$(call gb_Library__get_layer_componentprefix,$(call \
		gb_Library_get_layer,$(1)))

gb_Library__get_layer_componentprefix = \
	$(patsubst $(1):%,%,$(or \
		$(filter $(1):%,$(gb_Library__COMPONENTPREFIXES)), \
		$(call gb_Output_error,no ComponentTarget native prefix for layer '$(1)')))

gb_Library__COMPONENTPREFIXES := \
    NONE:vnd.sun.star.expand:\dLO_LIB_DIR/ \
    OOO:vnd.sun.star.expand:\dLO_LIB_DIR/ \
    URELIB:vnd.sun.star.expand:\dURE_INTERNAL_LIB_DIR/ \

define gb_Library__forward_to_Linktarget
gb_Library_$(1) = $$(call gb_LinkTarget_$(1),$$(call gb_Library_get_linktargetname,$$(1)),$$(2),$$(3))

endef

gb_Library_get_runtime_filename = \
 $(or $(call gb_Library_get_dllname,$(1)),$(call gb_Library_get_filename,$(1)))

$(eval $(foreach method,\
	add_cobject \
	add_cobjects \
	add_cxxobject \
	add_cxxobjects \
	add_objcobject \
	add_objcobjects \
	add_objcxxobject \
	add_objcxxobjects \
	add_asmobject \
	add_asmobjects \
	add_exception_objects \
	add_noexception_objects \
	add_generated_cobjects \
	add_generated_cxxobjects \
	add_generated_exception_objects \
	add_library_objects \
	add_grammar \
	add_grammars \
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
	export_objects_list \
	add_nativeres \
,\
	$(call gb_Library__forward_to_Linktarget,$(method))\
))

# vim: set noet sw=4:
