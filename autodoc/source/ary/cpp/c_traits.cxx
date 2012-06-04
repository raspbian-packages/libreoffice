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
#include <ary/cpp/c_traits.hxx>


// NOT FULLY DEFINED SERVICES
#include <ary/namesort.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/usedtype.hxx>
#include <ary/getncast.hxx>
#include "cs_ce.hxx"
#include "cs_def.hxx"
#include "cs_type.hxx"




namespace ary
{
namespace cpp
{



//********************      Ce_Traits      ********************//
Ce_Traits::entity_base_type &
Ce_Traits::EntityOf_(id_type i_id)
{
    csv_assert(i_id.IsValid());
    return Ce_Storage::Instance_()[i_id];
}

//********************      CeNode_Traits      ********************//
const symtree::Node<CeNode_Traits> *
CeNode_Traits::NodeOf_(const entity_base_type & i_entity)
{
    if (is_type<Namespace>(i_entity))
        return & ary_cast<Namespace>(i_entity).AsNode();
    else if (is_type<Class>(i_entity))
        return & ary_cast<Class>(i_entity).AsNode();
    return 0;
}

symtree::Node<CeNode_Traits> *
CeNode_Traits::NodeOf_(entity_base_type & io_entity)
{
    if (is_type<Namespace>(io_entity))
        return & ary_cast<Namespace>(io_entity).AsNode();
    else if (is_type<Class>(io_entity))
        return & ary_cast<Class>(io_entity).AsNode();
    return 0;
}

Ce_Traits::entity_base_type *
CeNode_Traits::ParentOf_(const entity_base_type & i_entity)
{
    Ce_Traits::id_type
        ret = i_entity.Owner();
    if (ret.IsValid())
    {
        if (is_type<EnumValue>(i_entity))
        {   // Return not the Enum, but the owner of the Enum:
            ret = EntityOf_(ret).Owner();
            csv_assert(ret.IsValid());
        }
        return &EntityOf_(ret);
    }
    return 0;
}

Ce_id
CeNode_Search( const CodeEntity &  i_entity,
               const String &      i_localKey )
{
    if (is_type<Namespace>(i_entity))
        return ary_cast<Namespace>(i_entity).Search_Child(i_localKey);
    else if (is_type<Class>(i_entity))
        return ary_cast<Class>(i_entity).Search_Child(i_localKey);
    return Ce_id(0);
}




//********************      Ce_Compare      ********************//
const Ce_Compare::key_type &
Ce_Compare::KeyOf_(const entity_base_type & i_entity)
{
    return i_entity.LocalName();
}

bool
Ce_Compare::Lesser_( const key_type &    i_1,
                     const key_type &    i_2 )
{
    static ::ary::LesserName    less_;
    return less_(i_1,i_2);
}


//********************      Ce_GlobalCompare      ********************//
void
Get_Qualified( StreamStr &          o_out,
               const CodeEntity &   i_ce )
{
    if (i_ce.LocalName().empty())
        return;
    if (i_ce.Owner().IsValid())
        Get_Qualified(o_out, Ce_Traits::EntityOf_(i_ce.Owner()));

    o_out << i_ce.LocalName() << "::";
}


bool
Ce_GlobalCompare::Lesser_( const key_type &    i_1,
                           const key_type &    i_2 )
{
    static ::ary::LesserName    less_;

    if (i_1.LocalName() != i_2.LocalName())
        return less_(i_1.LocalName(), i_2.LocalName());

    csv_assert(i_1.Owner().IsValid() AND i_2.Owner().IsValid());

    static StreamStr
        aBuffer1_(300);
    static StreamStr
        aBuffer2_(300);
    aBuffer1_.reset();
    aBuffer2_.reset();

    Get_Qualified(aBuffer1_, Ce_Traits::EntityOf_(i_1.Owner()));
    Get_Qualified(aBuffer2_, Ce_Traits::EntityOf_(i_2.Owner()));
    if (aBuffer1_.size() >= 2)
        aBuffer1_.pop_back(2);
    if (aBuffer2_.size() >= 2)
        aBuffer2_.pop_back(2);
    return less_(aBuffer1_.c_str(), aBuffer2_.c_str());
}



//********************      Def_Traits      ********************//
Def_Traits::entity_base_type &
Def_Traits::EntityOf_(id_type i_id)
{
    csv_assert(i_id.IsValid());
    return Def_Storage::Instance_()[i_id];
}

//********************      Def_Compare      ********************//
const Def_Compare::key_type &
Def_Compare::KeyOf_(const entity_base_type & i_entity)
{
    return i_entity.LocalName();
}

bool
Def_Compare::Lesser_( const key_type &    i_1,
                      const key_type &    i_2 )
{
    static ::ary::LesserName    less_;
    return less_(i_1,i_2);
}



//********************      Type_Traits      ********************//
Type_Traits::entity_base_type &
Type_Traits::EntityOf_(id_type i_id)
{
    csv_assert(i_id.IsValid());
    return Type_Storage::Instance_()[i_id];
}

//********************      Type_Compare      ********************//
const UsedType_Compare::key_type &
UsedType_Compare::KeyOf_(const entity_base_type & i_entity)
{
    csv_assert( is_type<UsedType>(i_entity) );
    return ary_cast<UsedType>(i_entity);
}

bool
UsedType_Compare::Lesser_( const key_type &    i_1,
                           const key_type &    i_2 )
{
    return i_1 < i_2;
}



}   // namespace cpp
}   // namespace ary

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
