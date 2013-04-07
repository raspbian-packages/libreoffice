# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Module_Module,sc))

$(eval $(call gb_Module_add_targets,sc,\
	AllLangResTarget_sc \
	Library_sc \
	Library_scd \
	Library_scfilt \
	Library_scui \
	Library_vbaobj \
	Package_uiconfig \
	Package_xml \
))

$(eval $(call gb_Module_add_check_targets,sc,\
    CppunitTest_sc_filters_test \
    CppunitTest_sc_ucalc \
))

$(eval $(call gb_Module_add_subsequentcheck_targets,sc,\
    CppunitTest_sc_macros_test \
    CppunitTest_sc_cellrangesbase \
    CppunitTest_sc_subsequent_filters_test \
    CppunitTest_sc_tableautoformatfield \
    JunitTest_sc_complex \
    JunitTest_sc_unoapi \
))

# vim: set noet sw=4 ts=4:
