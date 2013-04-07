#   -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
#  
#   Version: MPL 1.1 / GPLv3+ / LGPLv3+
#  
#   The contents of this file are subject to the Mozilla Public License Version
#   1.1 (the "License"); you may not use this file except in compliance with
#   the License or as specified alternatively below. You may obtain a copy of
#   the License at http://www.mozilla.org/MPL/
#  
#   Software distributed under the License is distributed on an "AS IS" basis,
#   WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
#   for the specific language governing rights and limitations under the
#   License.
#  
#   Major Contributor(s):
#   [ Copyright (C) 2011 Bjoern Michaelsen <bjoern.michaelsen@canonical.com> (initial developer) ]
#  
#   All Rights Reserved.
#  
#   For minor contributions see the git repository.
#  
#   Alternatively, the contents of this file may be used under the terms of
#   either the GNU General Public License Version 3 or later (the "GPLv3+"), or
#   the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
#   in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
#   instead of those above.


# speed up if no target need to parse all modules
gb_SpeedUpTargets_WRAPPEDBUILD:=
ifneq ($(strip $(MAKECMDGOALS)),)
ifeq ($(filter-out id tags docs distro-pack-install fetch help debugrun $(SRCDIR)/Env.Host.sh,$(MAKECMDGOALS)),)
gb_SpeedUpTargets_WRAPPEDBUILD:=T
endif
endif

ifeq ($(strip $(gb_PARTIALBUILD)),)
ifneq ($(strip $(MAKECMDGOALS)),)
ifeq ($(filter-out clean distclean id tags docs distro-pack-install fetch help debugrun $(SRCDIR)/Env.Host.sh,$(MAKECMDGOALS)),)
gb_SpeedUpTargets_WRAPPEDBUILD:=T
endif
endif
endif

ifneq ($(strip $(gb_SpeedUpTargets_WRAPPEDBUILD)),)
gb_Module_add_target=
gb_Module_add_check_target=
gb_Module_add_subsequentcheck_target=
gb_Module_add_moduledir=
gb_FULLDEPS=
endif

# vim:set shiftwidth=4 softtabstop=4 noexpandtab:

