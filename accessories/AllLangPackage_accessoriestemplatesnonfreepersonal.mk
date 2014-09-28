# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_AllLangPackage_AllLangPackage,accessoriestemplatesnonfreepersonal,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates-nonfree/personal/lang))

$(eval $(call gb_AllLangPackage_use_unpacked,accessoriestemplatesnonfreepersonal,templates-pack))

$(eval $(call gb_AllLangPackage_add_files_with_subdir,accessoriestemplatesnonfreepersonal,$(LIBO_SHARE_FOLDER)/template,personal,\
	cs/dummy_templates-nonfree.txt \
	de/dummy_templates-nonfree.txt \
	en-US/resume_1.ott \
	en-US/resume_2.ott \
	en-US/resume_3.ott \
	es/dummy_templates-nonfree.txt \
	fi/dummy_templates-nonfree.txt \
	fr/dummy_templates-nonfree.txt \
	hu/dummy_templates-nonfree.txt \
	it/dummy_templates-nonfree.txt \
	ja/dummy_templates-nonfree.txt \
	ka/dummy_templates-nonfree.txt \
	nl/dummy_templates-nonfree.txt \
	pl/dummy_templates-nonfree.txt \
	pt-BR/dummy_templates-nonfree.txt \
	sv/dummy_templates-nonfree.txt \
	tr/dummy_templates-nonfree.txt \
	zh-CN/dummy_templates-nonfree.txt \
))

# vim: set noet sw=4 ts=4:
