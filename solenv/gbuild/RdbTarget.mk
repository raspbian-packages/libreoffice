# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2010 Red Hat, Inc., David Tardon <dtardon@redhat.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

gb_RdbTarget__get_old_component_target = $(OUTDIR)/xml/$(1).component
gb_RdbTarget__get_rdbs = \
    $(foreach component,$(1),$(call gb_ComponentTarget_get_target,$(component))) \
    $(foreach component,$(2),$(call gb_RdbTarget__get_old_component_target,$(component)))

$(call gb_RdbTarget_get_target,%) :
	$(call gb_Output_announce,$*,$(true),RDB,1)
	$(call gb_Helper_abbreviate_dirs,\
		mkdir -p $(dir $@) && \
		$(if $(strip $(call gb_RdbTarget__get_rdbs,$(COMPONENTS),$(OLD_COMPONENTS))),, \
			$(error no components to register, check RdbTarget is included in gb_Module_add_check_targets - no rdb files)) \
		echo '<?xml version="1.0"?><components xmlns="http://openoffice.org/2010/uno-components">' > $@ && \
		$(gb_AWK) -- \
			' BEGIN { RS=">"; } \
			/^<\?xml version.*/ { next; } \
			/.*[^\r\n\t\s].*/ { print $$0 ">"; }' \
            $(call gb_RdbTarget__get_rdbs,$(COMPONENTS),$(OLD_COMPONENTS)) \
			>> $@ && \
		echo '</components>' >> $@)

.PHONY : $(call gb_RdbTarget_get_clean_target,%)
$(call gb_RdbTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),RDB,1)
	$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_RdbTarget_get_outdir_target,$*) $(call gb_RdbTarget_get_target,$*))

$(call gb_RdbTarget_get_outdir_target,%) :
	$(call gb_Deliver_deliver,$<,$@)

define gb_RdbTarget_RdbTarget
$(call gb_RdbTarget_get_target,$(1)) : COMPONENTS :=
$(call gb_RdbTarget_get_target,$(1)) : OLD_COMPONENTS :=
$(call gb_RdbTarget_get_clean_target,$(1)) : COMPONENTS :=
$(call gb_RdbTarget_get_clean_target,$(1)) : OLD_COMPONENTS :=
ifeq ($(2),$(true))
$(call gb_RdbTarget_get_outdir_target,$(1)) : $(call gb_RdbTarget_get_target,$(1))
$(call gb_Deliver_add_deliverable,$(call gb_ResTarget_get_outdir_target,$(1)),$(call gb_RdbTarget_get_target,$(1)))
endif
# depend on the last sourced makefile, which should contain the declaration of this target
# thus, when you change the RdbTarget_*.mk, the target will get rebuild
$(call gb_RdbTarget_get_target,$(1)) : $(lastword $(MAKEFILE_LIST))

$$(eval $$(call gb_Module_register_target,$(call gb_RdbTarget_get_target,$(1)),$(call gb_RdbTarget_get_clean_target,$(1))))
endef

define gb_RdbTarget_add_component
$(call gb_RdbTarget_get_target,$(1)) : $(call gb_ComponentTarget_get_target,$(2))
$(call gb_RdbTarget_get_target,$(1)) : COMPONENTS += $(2)
$(call gb_RdbTarget_get_clean_target,$(1)) : COMPONENTS += $(2)

endef

define gb_RdbTarget_add_components
$(foreach component,$(2),$(call gb_RdbTarget_add_component,$(1),$(component)))

endef

define gb_RdbTarget_add_old_component
$(call gb_RdbTarget_get_target,$(1)) : $(call gb_RdbTarget__get_old_component_target,$(2))
$(call gb_RdbTarget_get_target,$(1)) : OLD_COMPONENTS += $(2)
$(call gb_RdbTarget_get_clean_target,$(1)) : OLD_COMPONENTS += $(2)

endef

define gb_RdbTarget_add_old_components
$(foreach component,$(2),$(call gb_RdbTarget_add_old_component,$(1),$(component)))

endef

# vim: set noet sw=4:
