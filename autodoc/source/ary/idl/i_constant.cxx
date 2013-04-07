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

#include <precomp.h>
#include <ary/idl/i_constant.hxx>


// NOT FULLY DECLARED SERVICES
#include <cosv/tpl/processor.hxx>
#include <ary/idl/ik_constant.hxx>


namespace ary
{
namespace idl
{

Constant::Constant( const String &      i_sName,
                    Ce_id               i_nOwner,
                    Ce_id               i_nNameRoom,
                    Type_id             i_nType,
                    const String &      i_sInitValue )
    :   sName(i_sName),
        nNameRoom(i_nNameRoom),
        nOwner(i_nOwner),
        nType(i_nType),
        sInitValue(i_sInitValue)
{
}

Constant::~Constant()
{
}

void
Constant::do_Accept( csv::ProcessorIfc & io_processor ) const
{
    csv::CheckedCall(io_processor, *this);
}


ClassId
Constant::get_AryClass() const
{
    return class_id;
}

const String &
Constant::inq_LocalName() const
{
    return sName;
}

Ce_id
Constant::inq_NameRoom() const
{
    return nNameRoom;
}

Ce_id
Constant::inq_Owner() const
{
    return nOwner;
}

E_SightLevel
Constant::inq_SightLevel() const
{
    return sl_Member;
}


namespace ifc_constant
{

inline const Constant &
constant_cast( const CodeEntity &  i_ce )
{
    csv_assert( i_ce.AryClass() == Constant::class_id );
    return static_cast< const Constant& >(i_ce);
}

Type_id
attr::Type( const CodeEntity & i_ce )
{
    return constant_cast(i_ce).nType;
}

const String &
attr::Value( const CodeEntity & i_ce )
{
    return constant_cast(i_ce).sInitValue;
}

} // namespace ifc_constant


}   //  namespace   idl
}   //  namespace   ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
