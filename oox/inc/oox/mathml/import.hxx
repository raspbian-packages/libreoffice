/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License or as specified alternatively below. You may obtain a copy of
 * the License at http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * Major Contributor(s):
 * Copyright (C) 2011 Lubos Lunak <l.lunak@suse.cz> (initial developer)
 *
 * All Rights Reserved.
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */
#ifndef _STARMATHIMPORT_HXX
#define _STARMATHIMPORT_HXX

#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <tools/gen.hxx>

#include <oox/dllapi.h>

namespace oox
{

namespace formulaimport
{
class XmlStream;
}

/**
 Interface class, StarMath will implement readFormulaOoxml() to read OOXML
 representing the formula and getFormulaSize() to provide the size of the resulting
 formula.
 */
class OOX_DLLPUBLIC FormulaImportBase
{
public:
    FormulaImportBase();
    virtual void readFormulaOoxml( oox::formulaimport::XmlStream& stream ) = 0;
    virtual Size getFormulaSize() const = 0;
};

} // namespace

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
