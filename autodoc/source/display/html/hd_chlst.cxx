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
#include "hd_chlst.hxx"


// NOT FULLY DEFINED SERVICES
#include <ary/ceslot.hxx>
#include <ary/cpp/c_gate.hxx>
#include <ary/cpp/c_namesp.hxx>
#include <ary/cpp/c_class.hxx>
#include <ary/cpp/c_enum.hxx>
#include <ary/cpp/c_tydef.hxx>
#include <ary/cpp/c_funct.hxx>
#include <ary/cpp/c_vari.hxx>
#include <ary/cpp/c_enuval.hxx>
#include <ary/loc/loc_file.hxx>
#include <ary/loc/locp_le.hxx>
#include <ary/doc/d_oldcppdocu.hxx>
#include <ary/info/ci_attag.hxx>
#include <ary/info/ci_text.hxx>
#include <ary/info/all_dts.hxx>
#include "hd_docu.hxx"
#include "opageenv.hxx"
#include "protarea.hxx"
#include "strconst.hxx"


using namespace csi;
using html::Table;
using html::TableRow;
using html::TableCell;
using html::Font;
using html::SizeAttr;
using html::BgColorAttr;
using html::WidthAttr;


const int   ixPublic = 0;
const int   ixProtected = 1;
const int   ixPrivate = 2;

struct ChildList_Display::S_AreaCo
{
  public:
    ProtectionArea      aArea;
    Area_Result *       pResult;

                        S_AreaCo(
                            Area_Result &       o_rResult,
                            const char *        i_sLabel,
                            const char *        i_sTitle );
                        ~S_AreaCo();

    void                PerformResult();

  private:
    csi::xml::Element & Out()                   { return pResult->rOut; }
};


const ary::info::DocuText &
ShortDocu( const ary::cpp::CodeEntity & i_rCe )
{
    static const ary::info::DocuText
        aNull_;

    const ary::doc::OldCppDocu *
            pInfo = dynamic_cast< const ary::doc::OldCppDocu* >( i_rCe.Docu().Data() );
    if (pInfo == 0)
        return aNull_;

    return pInfo->Short().CText();
}


ChildList_Display::ChildList_Display( OuputPage_Environment & io_rEnv )
    :   HtmlDisplay_Impl( io_rEnv ),
        pShortDocu_Display( new Docu_Display(io_rEnv) ),
        pActiveParentClass(0),
        pActiveParentEnum(0),
        // pSglArea,
        // aMemberAreas,
        peClassesFilter(0)
{
}

ChildList_Display::ChildList_Display( OuputPage_Environment & io_rEnv,
                                      const ary::cpp::Class & i_rClass )
    :   HtmlDisplay_Impl( io_rEnv ),
        pShortDocu_Display( new Docu_Display(io_rEnv) ),
        pActiveParentClass(&i_rClass),
        pActiveParentEnum(0),
        // pSglArea,
        // aMemberAreas,
        peClassesFilter(0)
{
}

ChildList_Display::ChildList_Display( OuputPage_Environment & io_rEnv,
                                      const ary::cpp::Enum &  i_rEnum )
    :   HtmlDisplay_Impl( io_rEnv ),
        pShortDocu_Display( new Docu_Display(io_rEnv) ),
        pActiveParentClass(0),
        pActiveParentEnum(&i_rEnum),
        // pSglArea,
        // aMemberAreas,
        peClassesFilter(0)
{
}

ChildList_Display::~ChildList_Display()
{
}

void
ChildList_Display::Run_Simple( Area_Result &       o_rResult,
                               ary::SlotAccessId   i_nSlot,
                               const char *        i_sListLabel,
                               const char *        i_sListTitle )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot( i_nSlot ) );
    if ( pSlot->Size() == 0 )
        return;

    pSglArea = new S_AreaCo( o_rResult,
                             i_sListLabel,
                             i_sListTitle );

    pSlot->StoreAt(*this);

    pSglArea->PerformResult();
    pSglArea = 0;
}

void
ChildList_Display::Run_GlobalClasses( Area_Result &        o_rResult,
                                      ary::SlotAccessId    i_nSlot,
                                      const char *         i_sListLabel,
                                      const char *         i_sListTitle,
                                      ary::cpp::E_ClassKey i_eFilter )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot( i_nSlot ) );
    if ( pSlot->Size() == 0 )
        return;

    pSglArea = new S_AreaCo( o_rResult,
                             i_sListLabel,
                             i_sListTitle );

    SetClassesFilter(i_eFilter);
    pSlot->StoreAt(*this);
    UnsetClassesFilter();

    pSglArea->PerformResult();
    pSglArea = 0;
}

void
ChildList_Display::Run_Members( Area_Result &           o_rResult_public,
                                Area_Result &           o_rResult_protected,
                                Area_Result &           o_rResult_private,
                                ary::SlotAccessId       i_nSlot,
                                const char *            i_sListLabel_public,
                                const char *            i_sListLabel_protected,
                                const char *            i_sListLabel_private,
                                const char *            i_sListTitle )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot(i_nSlot) );
    if ( pSlot->Size() == 0 )
        return;

    aMemberAreas[ixPublic] = new S_AreaCo( o_rResult_public,
                                         i_sListLabel_public,
                                         i_sListTitle );
    aMemberAreas[ixProtected] = new S_AreaCo( o_rResult_protected,
                                            i_sListLabel_protected,
                                            i_sListTitle );
    aMemberAreas[ixPrivate] = new S_AreaCo( o_rResult_private,
                                          i_sListLabel_private,
                                          i_sListTitle );

    pSlot->StoreAt(*this);

    aMemberAreas[ixPublic]->PerformResult();
    aMemberAreas[ixProtected]->PerformResult();
    aMemberAreas[ixPrivate]->PerformResult();

    aMemberAreas[ixPublic] = 0;
    aMemberAreas[ixProtected] = 0;
    aMemberAreas[ixPrivate] = 0;
}

void
ChildList_Display::Run_MemberClasses( Area_Result &         o_rResult_public,
                                      Area_Result &         o_rResult_protected,
                                      Area_Result &         o_rResult_private,
                                      ary::SlotAccessId     i_nSlot,
                                      const char *          i_sListLabel_public,
                                      const char *          i_sListLabel_protected,
                                      const char *          i_sListLabel_private,
                                      const char *          i_sListTitle,
                                      ary::cpp::E_ClassKey  i_eFilter )
{
    ary::Slot_AutoPtr
            pSlot( ActiveParent().Create_Slot(i_nSlot) );
    if ( pSlot->Size() == 0 )
        return;

    aMemberAreas[ixPublic] = new S_AreaCo( o_rResult_public,
                                           i_sListLabel_public,
                                           i_sListTitle );
    aMemberAreas[ixProtected] = new S_AreaCo( o_rResult_protected,
                                              i_sListLabel_protected,
                                              i_sListTitle );
    aMemberAreas[ixPrivate] = new S_AreaCo( o_rResult_private,
                                            i_sListLabel_private,
                                            i_sListTitle );

    SetClassesFilter(i_eFilter);
    pSlot->StoreAt(*this);
    UnsetClassesFilter();

    aMemberAreas[ixPublic]->PerformResult();
    aMemberAreas[ixProtected]->PerformResult();
    aMemberAreas[ixPrivate]->PerformResult();

    aMemberAreas[ixPublic] = 0;
    aMemberAreas[ixProtected] = 0;
    aMemberAreas[ixPrivate] = 0;
}

void
ChildList_Display::do_Process( const ary::cpp::Namespace & i_rData )
{
    Write_ListItem( i_rData.LocalName(),
                    Path2ChildNamespace(i_rData.LocalName()),
                    ShortDocu( i_rData ),
                    GetArea().GetTable() );
}

void
ChildList_Display::do_Process( const ary::cpp::Class & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    if (peClassesFilter)
    {
         if (*peClassesFilter != i_rData.ClassKey() )
            return;
    }

    String  sLink;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        sLink = ClassFileName(i_rData.LocalName());

    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLink = Path2Child( ClassFileName(i_rData.LocalName()), pActiveParentClass->LocalName() );
    }

    if (peClassesFilter)
    {
        Write_ListItem( i_rData.LocalName(),
                        sLink,
                        ShortDocu( i_rData ),
                        GetArea(i_rData.Protection())
                            .GetTable() );
    }
    else
    {
        Write_ListItem( i_rData.LocalName(),
                        sLink,
                        ShortDocu( i_rData ),
                        GetArea(i_rData.Protection())
                            .GetTable(i_rData.ClassKey()) );
    }
}

void
ChildList_Display::do_Process( const ary::cpp::Enum & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    String  sLink;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        sLink = EnumFileName(i_rData.LocalName());
    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLink = Path2Child( EnumFileName(i_rData.LocalName()),
                            pActiveParentClass->LocalName() );
    }

    Write_ListItem( i_rData.LocalName(),
                    sLink,
                    ShortDocu( i_rData ),
                    GetArea(i_rData.Protection()).GetTable() );
}

void
ChildList_Display::do_Process( const ary::cpp::Typedef & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    String  sLink;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        sLink = TypedefFileName(i_rData.LocalName());
    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLink = Path2Child( TypedefFileName(i_rData.LocalName()),
                            pActiveParentClass->LocalName() );
    }

    Write_ListItem( i_rData.LocalName(),
                    sLink,
                    ShortDocu( i_rData ),
                    GetArea(i_rData.Protection()).GetTable() );
}

void
ChildList_Display::do_Process( const ary::cpp::Function & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    String  sLinkPrePath;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        const ary::loc::File &
                rFile = Env().Gate().Locations().Find_File( i_rData.Location() );
        sLinkPrePath = HtmlFileName( "o-", rFile.LocalName() );
    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLinkPrePath = Path2Child( HtmlFileName( "o", "" ),
                                   pActiveParentClass->LocalName() );
    }

    // Out
    Table & rOut = GetArea(i_rData.Protection()).GetTable();
    TableRow * dpRow = new TableRow;
    rOut << dpRow;
    TableCell & rCell1 = dpRow->AddCell();

    rCell1
        << SyntaxText_PreName( i_rData, Env().Gate() )
        << new html::LineBreak;
    rCell1
        >> *new html::Link( OperationLink(
                                Env().Gate(),
                                i_rData.LocalName(),
                                i_rData.CeId(),
                                sLinkPrePath) )
            << i_rData.LocalName();
    rCell1
        << SyntaxText_PostName( i_rData, Env().Gate() );
    TableCell &
        rCell2 = dpRow->AddCell();
    rCell2
        << new WidthAttr("50%")
        << " ";

    pShortDocu_Display->Assign_Out( rCell2 );
    ShortDocu( i_rData ).StoreAt( *pShortDocu_Display );
    pShortDocu_Display->Unassign_Out();
}

void
ChildList_Display::do_Process( const ary::cpp::Variable & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    String  sLinkPrePath;
    if ( i_rData.Protection() == ary::cpp::PROTECT_global )
    {
        const ary::loc::File &
            rFile = Env().Gate().Locations().Find_File( i_rData.Location() );
        sLinkPrePath = HtmlFileName( "d-", rFile.LocalName() );
    }
    else
    {
        csv_assert( pActiveParentClass != 0 );
        sLinkPrePath = Path2Child( HtmlFileName( "d", "" ),
                                   pActiveParentClass->LocalName() );
    }

    TableRow * dpRow = new TableRow;
    GetArea(i_rData.Protection()).GetTable() << dpRow;

    *dpRow << new html::BgColorAttr("white");
    csi::xml::Element &
        rCell1 = dpRow->AddCell();

    dshelp::Get_LinkedTypeText( rCell1, Env(), i_rData.Type() );
    rCell1
        << " "
        >> *new html::Link( DataLink(i_rData.LocalName(), sLinkPrePath.c_str()) )
            >> *new html::Strong
                << i_rData.LocalName()
                << ";";

    TableCell & rShortDocu = dpRow->AddCell();
    pShortDocu_Display->Assign_Out( rShortDocu );
    ShortDocu( i_rData ).StoreAt( *pShortDocu_Display );
    pShortDocu_Display->Unassign_Out();
}

void
ChildList_Display::do_Process( const ary::cpp::EnumValue & i_rData )
{
    if ( Ce_IsInternal(i_rData) )
        return;

    Table & rOut = GetArea().GetTable();

    TableRow * dpRow = new TableRow;
    rOut << dpRow;

    *dpRow << new html::BgColorAttr("white");
    dpRow->AddCell()
        << new WidthAttr("20%")
        << new xml::AnAttribute("valign", "top")
        >> *new html::Label(i_rData.LocalName())
            >> *new html::Bold
                << i_rData.LocalName();

    TableCell & rValueDocu = dpRow->AddCell();
    pShortDocu_Display->Assign_Out( rValueDocu );
    i_rData.Docu().Accept( *pShortDocu_Display );
    pShortDocu_Display->Unassign_Out();
}

void
ChildList_Display::do_StartSlot()
{
}

void
ChildList_Display::do_FinishSlot()
{
}

const ary::cpp::Gate *
ChildList_Display::inq_Get_ReFinder() const
{
    return & Env().Gate();
}

void
ChildList_Display::Write_ListItem( const String &               i_sLeftText,
                                   const char *                 i_sLink,
                                   const ary::info::DocuText &  i_rRightText,
                                   csi::xml::Element &          o_rOut )
{
    TableRow * dpRow = new TableRow;
    o_rOut << dpRow;

    *dpRow << new html::BgColorAttr("white");
    dpRow->AddCell()
        << new WidthAttr("20%")
        >> *new html::Link( i_sLink )
                >> *new html::Bold
                        << i_sLeftText;

    TableCell & rShortDocu = dpRow->AddCell();
    pShortDocu_Display->Assign_Out( rShortDocu );
    i_rRightText.StoreAt( *pShortDocu_Display );
    pShortDocu_Display->Unassign_Out();
}

const ary::AryGroup &
ChildList_Display::ActiveParent()
{
    return pActiveParentClass != 0
                ?   static_cast< const ary::AryGroup& >(*pActiveParentClass)
                :   pActiveParentEnum != 0
                        ?   static_cast< const ary::AryGroup& >(*pActiveParentEnum)
                        :   static_cast< const ary::AryGroup& >(*Env().CurNamespace());
}

ProtectionArea &
ChildList_Display::GetArea()
{
    return pSglArea->aArea;
}

ProtectionArea &
ChildList_Display::GetArea( ary::cpp::E_Protection i_eProtection )
{
    switch ( i_eProtection )
    {
         case ary::cpp::PROTECT_public:
                        return aMemberAreas[ixPublic]->aArea;
         case ary::cpp::PROTECT_protected:
                        return aMemberAreas[ixProtected]->aArea;
         case ary::cpp::PROTECT_private:
                        return aMemberAreas[ixPrivate]->aArea;
        default:
                        return pSglArea->aArea;
    }
}


//*******************               ********************//

ChildList_Display::
S_AreaCo::S_AreaCo( Area_Result &       o_rResult,
                    const char *        i_sLabel,
                    const char *        i_sTitle )
    :   aArea(i_sLabel, i_sTitle),
        pResult(&o_rResult)
{
}

ChildList_Display::
S_AreaCo::~S_AreaCo()
{
}

void
ChildList_Display::
S_AreaCo::PerformResult()
{
    bool bUsed = aArea.WasUsed_Area();
    pResult->rChildrenExist = bUsed;
    if ( bUsed )
    {
        Create_ChildListLabel( Out(), aArea.Label() );

        if ( aArea.Size() == 1 )
        {
            Out() << aArea.ReleaseTable();
        }
        else
        {
             Table * pTable = aArea.ReleaseTable( ary::cpp::CK_class );
            if (pTable != 0)
                Out() << pTable;
             pTable = aArea.ReleaseTable( ary::cpp::CK_struct );
            if (pTable != 0)
                Out() << pTable;
             pTable = aArea.ReleaseTable( ary::cpp::CK_union );
            if (pTable != 0)
                Out() << pTable;
        }
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
