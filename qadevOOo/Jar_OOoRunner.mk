#
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
# The Initial Developer of the Original Code is
# 	Peter Foley <pefoley2@verizon.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.
#

#force debug information for OOoRunner
gb_JavaClassSet_JAVACDEBUG:= -g

$(eval $(call gb_Jar_Jar,OOoRunner))

$(eval $(call gb_Jar_set_jarclasspath,OOoRunner,\
	ridl.jar \
	unoil.jar \
))

$(eval $(call gb_Jar_set_manifest,OOoRunner,$(SRCDIR)/qadevOOo/runner/manifest))

$(eval $(call gb_Jar_add_jars,OOoRunner,\
	$(OUTDIR)/bin/ridl.jar \
	$(OUTDIR)/bin/unoil.jar \
	$(OUTDIR)/bin/jurt.jar \
	$(OUTDIR)/bin/juh.jar \
	$(OUTDIR)/bin/java_uno.jar \
))

$(eval $(call gb_Jar_set_packageroot,OOoRunner,\
	base \
	basicrunner \
	complex \
	complexlib \
	convwatch \
	graphical \
	helper \
	ifc \
	lib \
	mod \
	org \
	share \
	stats \
	util \
))

$(eval $(call gb_Jar_add_sourcefiles,OOoRunner,\
	qadevOOo/runner/*/* \
	qadevOOo/runner/*/*/* \
	qadevOOo/tests/java/*/*/* \
	qadevOOo/tests/java/*/*/*/* \
))

$(eval $(call gb_Jar_add_packagefile,OOoRunner,objdsc,$(SRCDIR)/qadevOOo/objdsc))
