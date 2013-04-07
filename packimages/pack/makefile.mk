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
PRJ=..
PRJNAME=packimages
TARGET=packimages

.INCLUDE:  settings.mk
# point to an existing directory as default that poses no threat
# to a "find" that looks for "*.png"
RSCCUSTOMIMG*=$(PRJ)
.INCLUDE: target.mk

IMAGES := $(COMMONBIN)$/images.zip
SORTED_LIST=$(RES)$/img$/sorted.lst
# Custom sets, at 24x24 & 16x16 fall-back to Tango preferentially
# (Tango fallbacks to Industrial for the missing icons)
CUSTOM_IMAGE_SETS=$(WITH_THEMES)
CUSTOM_IMAGES+=$(foreach,i,$(CUSTOM_IMAGE_SETS) images_$i)
CUSTOM_PREFERRED_FALLBACK_1*=-c $(SOLARSRC)$/ooo_custom_images$/tango
CUSTOM_PREFERRED_FALLBACK_2*=-c $(SOLARSRC)$/ooo_custom_images$/industrial

ALLTAR : $(IMAGES) $(CUSTOM_IMAGES) $(COMMONBIN)$/images_brand.zip

$(IMAGES) $(CUSTOM_IMAGES) : $(SORTED_LIST)

$(SORTED_LIST) : image-sort.lst
    +-$(MKDIR) $(RES)$/img
    +$(PERL) $(SOLARENV)$/bin$/image-sort.pl image-sort.lst $(SOLARXMLDIR) $@

$(RES)$/img$/commandimagelist.ilst .PHONY : $(SORTED_LIST)
    @@-$(MKDIR) $(RES)$/img
#+-$(RM) $@ $@.$(INPATH)
    $(FIND) $(SOLARSRC)$/$(RSCDEFIMG)/cmd -name "*.png" | sed "s#$(SOLARSRC)$/$(RSCDEFIMG)#%MODULE%#" | $(PERL) $(SOLARENV)$/bin$/sort.pl > $@.$(INPATH)
    $(PERL) $(SOLARENV)$/bin$/diffmv.pl $@.$(INPATH) $@

$(COMMONBIN)$/images.zip .PHONY: $(RES)$/img$/commandimagelist.ilst
    $(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -l $(SOLARCOMMONRESDIR)$/img -s $(SORTED_LIST) -l $(RES)$/img -o $@

images_% : $(RES)$/img$/commandimagelist.ilst
    $(PERL) $(SOLARENV)$/bin$/packimages.pl -g $(SOLARSRC)$/$(RSCDEFIMG) -m $(SOLARSRC)$/$(RSCDEFIMG) -c $(RSCCUSTOMIMG) -c $(SOLARSRC)$/ooo_custom_images$/$(@:s/images_//) -c $(MISC)$/$(@:s/images_//) $(CUSTOM_PREFERRED_FALLBACK_1) $(CUSTOM_PREFERRED_FALLBACK_2) -l $(SOLARCOMMONRESDIR)$/img -l $(RES)$/img -s $(SORTED_LIST) -o $(COMMONBIN)$/$@.zip

# make sure to have one to keep packing happy
$(COMMONBIN)$/images_brand.zip:
    @$(TOUCH) $@
