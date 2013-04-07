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

gb_UnoApiTarget_IDLCTARGET := $(OUTDIR_FOR_BUILD)/bin/idlc$(gb_Executable_EXT_for_build)
gb_UnoApiTarget_IDLCCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_IDLCTARGET)
gb_UnoApiTarget_REGMERGETARGET := $(OUTDIR_FOR_BUILD)/bin/regmerge$(gb_Executable_EXT_for_build)
gb_UnoApiTarget_REGMERGECOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_REGMERGETARGET)
gb_UnoApiTarget_REGCOMPARETARGET := $(OUTDIR_FOR_BUILD)/bin/regcompare$(gb_Executable_EXT_for_build)
gb_UnoApiTarget_REGCOMPARECOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_REGCOMPARETARGET)
gb_UnoApiTarget_CPPUMAKERTARGET := $(OUTDIR_FOR_BUILD)/bin/cppumaker$(gb_Executable_EXT_for_build)
gb_UnoApiTarget_CPPUMAKERCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_CPPUMAKERTARGET)
gb_UnoApiTarget_REGVIEWTARGET := $(OUTDIR_FOR_BUILD)/bin/regview$(gb_Executable_EXT_for_build)
gb_UnoApiTarget_REGVIEWCOMMAND := $(gb_Helper_set_ld_path) SOLARBINDIR=$(OUTDIR_FOR_BUILD)/bin $(gb_UnoApiTarget_REGVIEWTARGET)

define gb_UnoApiTarget_autopackage_inc
$$(eval $$(call gb_Package_Package,$(1)_inc,$$(patsubst %/empty,%,$$(call gb_UnoApiTarget_get_header_target,empty))))
$(foreach onefile,$(gb_UnoApiTarget_HPPFILES_$(1)), \
	$$(eval $$(call gb_Package_add_file,$(1)_inc,$(patsubst $(1)/%,inc/$(1)/%,$(onefile)),$(onefile))))

$(call gb_Package_get_target,$(1)_inc) : $(call gb_UnoApiTarget_get_target,$(1))

endef

# TODO: make multi repo safe
define gb_UnoApiTarget_autopackage_idl
$$(eval $$(call gb_Package_Package,$(1)_idl,$(SRCDIR)))
$(foreach onefile,$(gb_UnoApiTarget_IDLFILES_$(1)), \
	$$(eval $$(call gb_Package_add_file,$(1)_idl,$(patsubst $(1)/%,idl/%,$(onefile)),$(onefile))))

endef

$(call gb_UnoApiTarget_get_header_target,$(1))/% : $(call gb_UnoApiTarget_get_target,$(1))
	mkdir -p $$(dir $$@)

define gb_UnoApiTarget_UnoApiTarget
$$(eval $$(call gb_Module_register_target,$(call gb_UnoApiOutTarget_get_target,$(1)),$(call gb_UnoApiOutTarget_get_clean_target,$(1))))
$(call gb_UnoApiOutTarget_get_target,$(1)) : $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiOutTarget_get_clean_target,$(1)) : $(call gb_UnoApiTarget_get_clean_target,$(1))
$(call gb_UnoApiTarget_get_target,$(1)) : INCLUDE :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_DEPS :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_MERGE :=
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_REFERENCE :=
gb_UnoApiTarget_HPPFILES_$(1) :=
gb_UnoApiTarget_IDLFILES_$(1) :=

endef

define gb_UnoApiTarget_add_idlfiles
$(foreach idl,$(3),$(call gb_UnoApiTarget_add_idlfile,$(1),$(2),$(idl)))

$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done) : $(foreach idl,$(3),$(SRCDIR)/$(2)/$(idl).idl)
	$(gb_UnoApiPartTarget__command)

endef

# for interfaces, exceptions, structs, enums, constant groups
define gb_UnoApiTarget_add_idlfile
$(call gb_UnoApiTarget_get_target,$(1)) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done)
$(call gb_UnoApiPartTarget_get_target,$(2)/idl.done) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/$(3).urd)
gb_UnoApiTarget_HPPFILES_$(1) += $(2)/$(3).hdl
gb_UnoApiTarget_HPPFILES_$(1) += $(2)/$(3).hpp
gb_UnoApiTarget_IDLFILES_$(1) += $(2)/$(3).idl

$(call gb_UnoApiTarget_get_header_target,)$(2)/$(3).hpp :| $(call gb_UnoApiTarget_get_target,$(1))
$(call gb_UnoApiTarget_get_header_target,)$(2)/$(3).hdl :| $(call gb_UnoApiTarget_get_target,$(1))

endef

define gb_UnoApiTarget_add_idlfiles_noheader
$(foreach idl,$(3),$(call gb_UnoApiTarget_add_idlfile_noheader,$(1),$(2),$(idl)))

$(call gb_UnoApiPartTarget_get_target,$(2)/idl_noheader.done) : $(foreach idl,$(3),$(SRCDIR)/$(2)/$(idl).idl)
	$(gb_UnoApiPartTarget__command)

endef

# for old-style services and modules
define gb_UnoApiTarget_add_idlfile_noheader
$(call gb_UnoApiTarget_get_target,$(1)) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/idl_noheader.done)
$(call gb_UnoApiPartTarget_get_target,$(2)/idl_noheader.done) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/$(3).urd)
gb_UnoApiTarget_IDLFILES_$(1) += $(2)/$(3).idl

endef

define gb_UnoApiTarget_add_idlfiles_nohdl
$(foreach idl,$(3),$(call gb_UnoApiTarget_add_idlfile_nohdl,$(1),$(2),$(idl)))

$(call gb_UnoApiPartTarget_get_target,$(2)/idl_nohdl.done) : $(foreach idl,$(3),$(SRCDIR)/$(2)/$(idl).idl)
	$(gb_UnoApiPartTarget__command)

endef

# for new-style services
define gb_UnoApiTarget_add_idlfile_nohdl
$(call gb_UnoApiTarget_get_target,$(1)) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/idl_nohdl.done)
$(call gb_UnoApiPartTarget_get_target,$(2)/idl_nohdl.done) : \
	$(call gb_UnoApiPartTarget_get_target,$(2)/$(3).urd)
gb_UnoApiTarget_HPPFILES_$(1) += $(2)/$(3).hpp
gb_UnoApiTarget_IDLFILES_$(1) += $(2)/$(3).idl

$(call gb_UnoApiTarget_get_header_target,)$(2)/$(3).hpp :| $(call gb_UnoApiTarget_get_target,$(1))

endef

define gb_UnoApiTarget_add_rdbfiles
$(foreach rdb,$(2),$(call gb_UnoApiTarget_add_rdbfile,$(1),$(rdb)))

endef

define gb_UnoApiTarget_add_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_DEPS += -X$(call gb_UnoApiOutTarget_get_target,$(2))
$(call gb_UnoApiTarget_get_target,$(1)) : \
	$(call gb_UnoApiOutTarget_get_target,$(2)) \
	$(call gb_Package_get_target,$(2)_idl)

endef

define gb_UnoApiTarget_add_merge_rdbfiles
$(foreach rdb,$(2),$(call gb_UnoApiTarget_add_merge_rdbfile,$(1),$(rdb)))

endef

define gb_UnoApiTarget_add_merge_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_MERGE += $(call gb_UnoApiOutTarget_get_target,$(2))
$(call gb_UnoApiTarget_get_target,$(1)) : $(call gb_UnoApiOutTarget_get_target,$(2))

endef

define gb_UnoApiTarget_add_reference_rdbfile
$(call gb_UnoApiTarget_get_target,$(1)) : UNOAPI_REFERENCE := $(SRCDIR)/$(strip $(2)).rdb

endef

define gb_UnoApiTarget_set_include
$(call gb_UnoApiTarget_get_target,$(1)) : INCLUDE := $(2)

endef

.PHONY : $(call gb_UnoApiOutTarget_get_clean_target,%)
$(call gb_UnoApiOutTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UNO,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UnoApiOutTarget_get_target,$*)) 

.PHONY : $(call gb_UnoApiTarget_get_clean_target,%)
$(call gb_UnoApiTarget_get_clean_target,%) :
	$(call gb_Output_announce,$*,$(false),UNO,1)
	-$(call gb_Helper_abbreviate_dirs,\
		rm -f $(call gb_UnoApiTarget_get_target,$*)) 
	-rm -rf $(call gb_UnoApiTarget_get_header_target,$*)\
			$(call gb_UnoApiPartTarget_get_target,$*)

# The .urd files are actually created by the gb_UnoApiPartTarget__command,
# invoked for the per-directory .done files.
# The reason why .urd files are tracked is so new files that are added are
# picked up and cause a rebuild, even if older than the .done file (also, as a
# convenience for users who delete them from the workdir by hand; this dummy
# rule plus the dependency from the .done target to the .urd file plus the
# sort/patsubst call in gb_UnoApiPartTarget__command cause command to be
# invoked with the .idl file corresponding to the .urd in that case.
$(call gb_UnoApiPartTarget_get_target,%.urd) :
	@true

define gb_UnoApiPartTarget__command
	$$(call gb_Output_announce,$(2),$(true),IDL,2)
	mkdir -p $(call gb_UnoApiPartTarget_get_target,$(2)) && \
	RESPONSEFILE=$$(call var2file,$$(shell $(gb_MKTEMP)),500,\
		$$(call gb_Helper_convert_native,$$(INCLUDE) $$(DEFS) -O $(call gb_UnoApiPartTarget_get_target,$(2)) -verbose -C \
		$$(sort $$(patsubst $$(call gb_UnoApiPartTarget_get_target,%.urd),$(SRCDIR)/%.idl,$$?)))) && \
	$(gb_UnoApiTarget_IDLCCOMMAND) @$$$${RESPONSEFILE} > /dev/null && \
	rm -f $$$${RESPONSEFILE} && \
	touch $$@

endef

define gb_UnoApiTarget__command
	mkdir -p $(dir $(1)) && \
	RESPONSEFILE=$(call var2file,$(shell $(gb_MKTEMP)),500,$(call gb_Helper_convert_native,$(1) $(4) $(5))) && \
	$(gb_UnoApiTarget_REGMERGECOMMAND) @$${RESPONSEFILE} && \
	rm -f $${RESPONSEFILE}

endef

define gb_UnoApiHeaderTarget__command
$(call gb_Helper_abbreviate_dirs_native,\
	mkdir -p $(dir $(1)) && \
	mkdir -p  $(gb_Helper_MISC) && \
	RESPONSEFILE=`$(gb_MKTEMP)` && \
	echo " -Gc -L -BUCR -O$(call gb_UnoApiTarget_get_header_target,$*) $(7) \
		$(1) \
		" > $${RESPONSEFILE} && \
	$(gb_UnoApiTarget_CPPUMAKERCOMMAND) @$${RESPONSEFILE} && \
	rm -f $${RESPONSEFILE})

endef

# TODO:
# - get idlc switch "-P" (generate .urd into package dir)
# - generate dependencies for included idls
# - empty $? in headertarget?

# TODO: reenable the check
$(call gb_UnoApiTarget_get_target,%):
	$(call gb_Output_announce,$*,$(true),RDB,3)
	$(if $(gb_UnoApiTarget_IDLFILES_$*),$(call gb_UnoApiTarget__command,$@,$*,$<,UCR,$(addprefix $(call gb_UnoApiPartTarget_get_target,),$(patsubst %.idl,%.urd,$(gb_UnoApiTarget_IDLFILES_$*)))))
	$(if $(UNOAPI_MERGE),$(call gb_UnoApiTarget__command,$@,$*,$<,/,$(UNOAPI_MERGE)))
	$(if $(UNOAPI_REFERENCE), \
		$(call gb_Output_announce,$*,$(true),DBc,4) \
	    $(gb_UnoApiTarget_REGCOMPARECOMMAND) -f -t -r1 $(call gb_Helper_convert_native,$(UNOAPI_REFERENCE)) -r2 $(call gb_Helper_convert_native,$@))
	$(if $(gb_UnoApiTarget_IDLFILES_$*), \
		$(call gb_Output_announce,$*,$(true),HPP,4) \
		$(call gb_UnoApiHeaderTarget__command,$@,$*,$<,$?,$(INCLUDE),$(DEFS),$(UNOAPI_DEPS)))

# vim: set noet sw=4 ts=4:
