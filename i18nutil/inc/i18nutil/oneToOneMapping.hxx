/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef INCLUDED_I18NUTIL_TRANSLITERATION_ONETOONEMAPPING_HXX
#define INCLUDED_I18NUTIL_TRANSLITERATION_ONETOONEMAPPING_HXX

#include <utility>
#include <rtl/ustring.hxx>
#include "i18nutildllapi.h"

namespace com { namespace sun { namespace star { namespace i18n {

class widthfolding;

typedef std::pair< sal_Unicode, sal_Unicode > OneToOneMappingTable_t;

#define MAKE_PAIR(item1,item2) std::make_pair< sal_Unicode, sal_Unicode >((sal_Unicode)item1,(sal_Unicode)item2)

typedef sal_Int8 UnicodePairFlag;
typedef struct _UnicodePairWithFlag
{
    sal_Unicode     first;
    sal_Unicode     second;
    UnicodePairFlag flag;
} UnicodePairWithFlag;

class I18NUTIL_DLLPUBLIC oneToOneMapping
{
private:
    // no copy, no substitution
    I18NUTIL_DLLPRIVATE oneToOneMapping( const oneToOneMapping& );
    I18NUTIL_DLLPRIVATE oneToOneMapping& operator=( const oneToOneMapping& );
public:
    oneToOneMapping( OneToOneMappingTable_t *rpTable, const size_t rnSize, const size_t rnUnitSize = sizeof(OneToOneMappingTable_t) );
    virtual ~oneToOneMapping();

    // binary search
    virtual sal_Unicode find( const sal_Unicode nKey ) const;

    // translator
    sal_Unicode operator[] ( const sal_Unicode nKey ) const { return find( nKey ); };

protected:
    OneToOneMappingTable_t *mpTable;
    size_t                  mnSize;
};

class I18NUTIL_DLLPUBLIC oneToOneMappingWithFlag : public oneToOneMapping
{
    friend class widthfolding;

private:
    // no copy, no substitution
    I18NUTIL_DLLPRIVATE oneToOneMappingWithFlag( const oneToOneMappingWithFlag& );
    I18NUTIL_DLLPRIVATE oneToOneMappingWithFlag& operator=( const oneToOneMappingWithFlag& );
public:
    oneToOneMappingWithFlag( UnicodePairWithFlag *rpTableWF, const size_t rnSize, const UnicodePairFlag rnFlag );
    virtual ~oneToOneMappingWithFlag();

    // make index for fast search
    void makeIndex();

    // index search
    virtual sal_Unicode find( const sal_Unicode nKey ) const;
protected:
    UnicodePairWithFlag  *mpTableWF;
    UnicodePairFlag       mnFlag;
    UnicodePairWithFlag **mpIndex[256];
    sal_Bool              mbHasIndex;
};

} } } }

#endif // _I18N_TRANSLITERATION_ONETOONEMAPPING_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
