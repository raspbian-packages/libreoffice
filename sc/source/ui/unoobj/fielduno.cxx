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

#include <svl/smplhint.hxx>
#include <vcl/svapp.hxx>


#include <editeng/eeitem.hxx>

#include <editeng/editeng.hxx>
#include <editeng/editobj.hxx>
#include <editeng/flditem.hxx>
#include <comphelper/servicehelper.hxx>

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/WrapTextMode.hpp>
#include <com/sun/star/text/FilenameDisplayFormat.hpp>

#include "fielduno.hxx"
#include "textuno.hxx"
#include "miscuno.hxx"
#include "docsh.hxx"
#include "hints.hxx"
#include "editsrc.hxx"
#include "cellsuno.hxx"
#include "servuno.hxx"      // fuer IDs
#include "unonames.hxx"
#include "editutil.hxx"

using namespace com::sun::star;

//------------------------------------------------------------------------

//  alles ohne Which-ID, Map nur fuer PropertySetInfo

const SfxItemPropertySet* lcl_GetURLPropertySet()
{
    static SfxItemPropertyMapEntry aURLPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ANCTYPE),  0,  &getCppuType((text::TextContentAnchorType*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ANCTYPES), 0,  &getCppuType((uno::Sequence<text::TextContentAnchorType>*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_REPR),     0,  &getCppuType((rtl::OUString*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_TARGET),   0,  &getCppuType((rtl::OUString*)0),    0, 0},
        {MAP_CHAR_LEN(SC_UNONAME_TEXTWRAP), 0,  &getCppuType((text::WrapTextMode*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_URL),      0,  &getCppuType((rtl::OUString*)0),    0, 0},
        {0,0,0,0,0,0}
    };
    static SfxItemPropertySet aURLPropertySet_Impl( aURLPropertyMap_Impl );
    return &aURLPropertySet_Impl;
}

const SfxItemPropertySet* lcl_GetHeaderFieldPropertySet()
{
    static SfxItemPropertyMapEntry aHeaderFieldPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ANCTYPE),  0,  &getCppuType((text::TextContentAnchorType*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ANCTYPES), 0,  &getCppuType((uno::Sequence<text::TextContentAnchorType>*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TEXTWRAP), 0,  &getCppuType((text::WrapTextMode*)0), beans::PropertyAttribute::READONLY, 0 },
        {0,0,0,0,0,0}
    };
    static SfxItemPropertySet aHeaderFieldPropertySet_Impl( aHeaderFieldPropertyMap_Impl );
    return &aHeaderFieldPropertySet_Impl;
}

const SfxItemPropertySet* lcl_GetFileFieldPropertySet()
{
    static SfxItemPropertyMapEntry aFileFieldPropertyMap_Impl[] =
    {
        {MAP_CHAR_LEN(SC_UNONAME_ANCTYPE),  0,  &getCppuType((text::TextContentAnchorType*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_ANCTYPES), 0,  &getCppuType((uno::Sequence<text::TextContentAnchorType>*)0), beans::PropertyAttribute::READONLY, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_FILEFORM), 0,  &getCppuType((sal_Int16*)0),        0, 0 },
        {MAP_CHAR_LEN(SC_UNONAME_TEXTWRAP), 0,  &getCppuType((text::WrapTextMode*)0), beans::PropertyAttribute::READONLY, 0 },
        {0,0,0,0,0,0}
    };
    static SfxItemPropertySet aFileFieldPropertySet_Impl( aFileFieldPropertyMap_Impl );
    return &aFileFieldPropertySet_Impl;
}

//------------------------------------------------------------------------

#define SCTEXTFIELD_SERVICE         "com.sun.star.text.TextField"
#define SCTEXTCONTENT_SERVICE       "com.sun.star.text.TextContent"

SC_SIMPLE_SERVICE_INFO( ScCellFieldsObj, "ScCellFieldsObj", "com.sun.star.text.TextFields" )
SC_SIMPLE_SERVICE_INFO( ScHeaderFieldsObj, "ScHeaderFieldsObj", "com.sun.star.text.TextFields" )

//------------------------------------------------------------------------

//  ScUnoEditEngine nur um aus einer EditEngine die Felder herauszubekommen...

enum ScUnoCollectMode
{
    SC_UNO_COLLECT_NONE,
    SC_UNO_COLLECT_COUNT,
    SC_UNO_COLLECT_FINDINDEX,
    SC_UNO_COLLECT_FINDPOS
};

class ScUnoEditEngine : public ScEditEngineDefaulter
{
    ScUnoCollectMode    eMode;
    sal_uInt16              nFieldCount;
    TypeId              aFieldType;
    SvxFieldData*       pFound;         // lokale Kopie
    sal_uInt16              nFieldPar;
    xub_StrLen          nFieldPos;
    sal_uInt16              nFieldIndex;

public:
                ScUnoEditEngine(ScEditEngineDefaulter* pSource);
                ~ScUnoEditEngine();

                    //! nPos should be xub_StrLen
    virtual String  CalcFieldValue( const SvxFieldItem& rField, sal_uInt16 nPara, sal_uInt16 nPos,
                                    Color*& rTxtColor, Color*& rFldColor );

    sal_uInt16          CountFields(TypeId aType);
    SvxFieldData*   FindByIndex(sal_uInt16 nIndex, TypeId aType);
    SvxFieldData*   FindByPos(sal_uInt16 nPar, xub_StrLen nPos, TypeId aType);

    sal_uInt16          GetFieldPar() const     { return nFieldPar; }
    xub_StrLen      GetFieldPos() const     { return nFieldPos; }
};

ScUnoEditEngine::ScUnoEditEngine(ScEditEngineDefaulter* pSource) :
    ScEditEngineDefaulter( *pSource ),
    eMode( SC_UNO_COLLECT_NONE ),
    nFieldCount( 0 ),
    aFieldType( NULL ),
    pFound( NULL )
{
    if (pSource)
    {
        EditTextObject* pData = pSource->CreateTextObject();
        SetText( *pData );
        delete pData;
    }
}

ScUnoEditEngine::~ScUnoEditEngine()
{
    delete pFound;
}

String ScUnoEditEngine::CalcFieldValue( const SvxFieldItem& rField,
            sal_uInt16 nPara, sal_uInt16 nPos, Color*& rTxtColor, Color*& rFldColor )
{
    String aRet(EditEngine::CalcFieldValue( rField, nPara, nPos, rTxtColor, rFldColor ));
    if (eMode != SC_UNO_COLLECT_NONE)
    {
        const SvxFieldData* pFieldData = rField.GetField();
        if ( pFieldData )
        {
            if ( !aFieldType || pFieldData->Type() == aFieldType )
            {
                if ( eMode == SC_UNO_COLLECT_FINDINDEX && !pFound && nFieldCount == nFieldIndex )
                {
                    pFound = pFieldData->Clone();
                    nFieldPar = nPara;
                    nFieldPos = nPos;
                }
                if ( eMode == SC_UNO_COLLECT_FINDPOS && !pFound &&
                        nPara == nFieldPar && nPos == nFieldPos )
                {
                    pFound = pFieldData->Clone();
                    nFieldIndex = nFieldCount;
                }
                ++nFieldCount;
            }
        }
    }
    return aRet;
}

sal_uInt16 ScUnoEditEngine::CountFields(TypeId aType)
{
    eMode = SC_UNO_COLLECT_COUNT;
    aFieldType = aType;
    nFieldCount = 0;
    UpdateFields();
    aFieldType = NULL;
    eMode = SC_UNO_COLLECT_NONE;

    return nFieldCount;
}

SvxFieldData* ScUnoEditEngine::FindByIndex(sal_uInt16 nIndex, TypeId aType)
{
    eMode = SC_UNO_COLLECT_FINDINDEX;
    nFieldIndex = nIndex;
    aFieldType = aType;
    nFieldCount = 0;
    UpdateFields();
    aFieldType = NULL;
    eMode = SC_UNO_COLLECT_NONE;

    return pFound;
}

SvxFieldData* ScUnoEditEngine::FindByPos(sal_uInt16 nPar, xub_StrLen nPos, TypeId aType)
{
    eMode = SC_UNO_COLLECT_FINDPOS;
    nFieldPar = nPar;
    nFieldPos = nPos;
    aFieldType = aType;
    nFieldCount = 0;
    UpdateFields();
    aFieldType = NULL;
    eMode = SC_UNO_COLLECT_NONE;

    return pFound;
}

//------------------------------------------------------------------------

ScCellFieldsObj::ScCellFieldsObj(ScDocShell* pDocSh, const ScAddress& rPos) :
    pDocShell( pDocSh ),
    aCellPos( rPos ),
    mpRefreshListeners( NULL )
{
    pDocShell->GetDocument()->AddUnoObject(*this);

    pEditSource = new ScCellEditSource( pDocShell, aCellPos );
}

ScCellFieldsObj::~ScCellFieldsObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    delete pEditSource;

    // increment refcount to prevent double call off dtor
    osl_incrementInterlockedCount( &m_refCount );

    if (mpRefreshListeners)
    {
        lang::EventObject aEvent;
        aEvent.Source.set(static_cast<cppu::OWeakObject*>(this));
        if (mpRefreshListeners)
        {
            mpRefreshListeners->disposeAndClear(aEvent);
            DELETEZ( mpRefreshListeners );
        }
    }
}

void ScCellFieldsObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }

    //  EditSource hat sich selber als Listener angemeldet
}

// XIndexAccess (via XTextFields)

ScCellFieldObj* ScCellFieldsObj::GetObjectByIndex_Impl(sal_Int32 Index) const
{
    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    if ( aTempEngine.FindByIndex( (sal_uInt16)Index, NULL ) )   // in der Zelle ist der Typ egal
    {
        sal_uInt16 nPar = aTempEngine.GetFieldPar();
        xub_StrLen nPos = aTempEngine.GetFieldPos();
        ESelection aSelection( nPar, nPos, nPar, nPos+1 );      // Feld ist 1 Zeichen
        return new ScCellFieldObj( pDocShell, aCellPos, aSelection );
    }
    return NULL;
}

sal_Int32 SAL_CALL ScCellFieldsObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    return aTempEngine.CountFields(NULL);       // Felder zaehlen, in Zelle ist der Typ egal
}

uno::Any SAL_CALL ScCellFieldsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XTextField> xField(GetObjectByIndex_Impl(nIndex));
    if (xField.is())
        return uno::makeAny(xField);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScCellFieldsObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<text::XTextField>*)0);
}

sal_Bool SAL_CALL ScCellFieldsObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Reference<container::XEnumeration> SAL_CALL ScCellFieldsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextFieldEnumeration")));
}

void SAL_CALL ScCellFieldsObj::addContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
                                    throw(uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ScCellFieldsObj::removeContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
                                    throw(uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

// XRefreshable
void SAL_CALL ScCellFieldsObj::refresh(  )
                                    throw (uno::RuntimeException)
{
    if (mpRefreshListeners)
    {
        //  Call all listeners.
        uno::Sequence< uno::Reference< uno::XInterface > > aListeners(mpRefreshListeners->getElements());
        sal_uInt32 nLength(aListeners.getLength());
        if (nLength)
        {
            const uno::Reference< uno::XInterface >* pInterfaces = aListeners.getConstArray();
            if (pInterfaces)
            {
                lang::EventObject aEvent;
                aEvent.Source.set(uno::Reference< util::XRefreshable >(const_cast<ScCellFieldsObj*>(this)));
                sal_uInt32 i(0);
                while (i < nLength)
                {
                    try
                    {
                        while(i < nLength)
                        {
                            static_cast< util::XRefreshListener* >(pInterfaces->get())->refreshed(aEvent);
                            ++pInterfaces;
                            ++i;
                        }
                    }
                    catch(uno::RuntimeException&)
                    {
                        ++pInterfaces;
                        ++i;
                    }
                }
            }
        }
    }
}

void SAL_CALL ScCellFieldsObj::addRefreshListener( const uno::Reference< util::XRefreshListener >& xListener )
                                    throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!mpRefreshListeners)
            mpRefreshListeners = new cppu::OInterfaceContainerHelper(aMutex);
        mpRefreshListeners->addInterface(xListener);
    }
}

void SAL_CALL ScCellFieldsObj::removeRefreshListener( const uno::Reference<util::XRefreshListener >& xListener )
                                    throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (mpRefreshListeners)
            mpRefreshListeners->removeInterface(xListener);
    }
}

//------------------------------------------------------------------------

//  Default-ctor wird fuer SMART_REFLECTION_IMPLEMENTATION gebraucht


ScCellFieldObj::ScCellFieldObj(ScDocShell* pDocSh, const ScAddress& rPos,
                                            const ESelection& rSel) :
    OComponentHelper( getMutex() ),
    pPropSet( lcl_GetURLPropertySet() ),
    pDocShell( pDocSh ),
    aCellPos( rPos ),
    aSelection( rSel )
{
    //  pDocShell ist Null, wenn per ServiceProvider erzeugt

    if (pDocShell)
    {
        pDocShell->GetDocument()->AddUnoObject(*this);
        pEditSource = new ScCellEditSource( pDocShell, aCellPos );
    }
    else
        pEditSource = NULL;
}

uno::Any SAL_CALL ScCellFieldObj::queryAggregation( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( text::XTextField )
    SC_QUERYINTERFACE( text::XTextContent )         // parent of XTextField
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( lang::XUnoTunnel )
    SC_QUERYINTERFACE( lang::XServiceInfo )

    return OComponentHelper::queryAggregation( rType );     // XComponent
}

uno::Sequence<uno::Type> SAL_CALL ScCellFieldObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes(OComponentHelper::getTypes());
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 4 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<text::XTextField>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<beans::XPropertySet>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);
        pPtr[nParentLen + 3] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

namespace
{
    class theScCellFieldObjImplementationId : public rtl::Static< UnoTunnelIdInit, theScCellFieldObjImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL ScCellFieldObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    return theScCellFieldObjImplementationId::get().getSeq();
}

uno::Any SAL_CALL ScCellFieldObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    return OComponentHelper::queryInterface( rType );
}

void SAL_CALL ScCellFieldObj::acquire() throw()
{
    OComponentHelper::acquire();
}

void SAL_CALL ScCellFieldObj::release() throw()
{
    OComponentHelper::release();
}

void ScCellFieldObj::InitDoc( ScDocShell* pDocSh, const ScAddress& rPos,
                                        const ESelection& rSel )
{
    if ( pDocSh && !pEditSource )
    {
        aCellPos = rPos;
        aSelection = rSel;
        pDocShell = pDocSh;

        pDocShell->GetDocument()->AddUnoObject(*this);
        pEditSource = new ScCellEditSource( pDocShell, aCellPos );
    }
}

ScCellFieldObj::~ScCellFieldObj()
{
    if (pDocShell)
        pDocShell->GetDocument()->RemoveUnoObject(*this);

    delete pEditSource;
}

void ScCellFieldObj::Notify( SfxBroadcaster&, const SfxHint& rHint )
{
    //! Updates fuer aSelection (muessen erst noch erzeugt werden) !!!!!!

    if ( rHint.ISA( ScUpdateRefHint ) )
    {
        //! Ref-Update
    }
    else if ( rHint.ISA( SfxSimpleHint ) &&
            ((const SfxSimpleHint&)rHint).GetId() == SFX_HINT_DYING )
    {
        pDocShell = NULL;       // ungueltig geworden
    }

    //  EditSource hat sich selber als Listener angemeldet
}

// per getImplementation gerufen:

SvxFieldItem ScCellFieldObj::CreateFieldItem()
{
    OSL_ENSURE( !pEditSource, "CreateFieldItem mit eingefuegtem Feld" );

    SvxURLField aField;
    aField.SetFormat(SVXURLFORMAT_APPDEFAULT);
    aField.SetURL( aUrl );
    aField.SetRepresentation( aRepresentation );
    aField.SetTargetFrame( aTarget );
    return SvxFieldItem( aField, EE_FEATURE_FIELD );
}

void ScCellFieldObj::DeleteField()
{
    if (pEditSource)
    {
        SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
        pForwarder->QuickInsertText( String(), aSelection );
        pEditSource->UpdateData();

        aSelection.nEndPara = aSelection.nStartPara;
        aSelection.nEndPos  = aSelection.nStartPos;

        //! Broadcast, um Selektion in anderen Objekten anzupassen
        //! (auch bei anderen Aktionen)
    }
}

// XTextField

rtl::OUString SAL_CALL ScCellFieldObj::getPresentation( sal_Bool bShowCommand )
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aRet;

    if (pEditSource)
    {
        //! Feld-Funktionen muessen an den Forwarder !!!
        ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  Typ egal (in Zellen gibts nur URLs)
        SvxFieldData* pField = aTempEngine.FindByPos( aSelection.nStartPara, aSelection.nStartPos, 0 );
        OSL_ENSURE(pField,"getPresentation: Feld nicht gefunden");
        if (pField)
        {
            SvxURLField* pURL = (SvxURLField*)pField;
            if (bShowCommand)
                aRet = pURL->GetURL();
            else
                aRet = pURL->GetRepresentation();
        }
    }

    return aRet;
}

// XTextContent

void SAL_CALL ScCellFieldObj::attach( const uno::Reference<text::XTextRange>& xTextRange )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (xTextRange.is())
    {
        uno::Reference<text::XText> xText(xTextRange->getText());
        if (xText.is())
        {
            xText->insertTextContent( xTextRange, this, sal_True );
        }
    }
}

uno::Reference<text::XTextRange> SAL_CALL ScCellFieldObj::getAnchor() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pDocShell)
        return new ScCellObj( pDocShell, aCellPos );
    return NULL;
}

// XComponent

void SAL_CALL ScCellFieldObj::dispose() throw(uno::RuntimeException)
{
    OComponentHelper::dispose();
}

void SAL_CALL ScCellFieldObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    OComponentHelper::addEventListener( xListener );
}

void SAL_CALL ScCellFieldObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    OComponentHelper::removeEventListener( xListener );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScCellFieldObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    static uno::Reference<beans::XPropertySetInfo> aRef = pPropSet->getPropertySetInfo();
    return aRef;
}

void SAL_CALL ScCellFieldObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameString(aPropertyName);
    rtl::OUString aStrVal;
    if (pEditSource)
    {
        //! Feld-Funktionen muessen an den Forwarder !!!
        ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  Typ egal (in Zellen gibts nur URLs)
        SvxFieldData* pField = aTempEngine.FindByPos( aSelection.nStartPara, aSelection.nStartPos, 0 );
        OSL_ENSURE(pField,"setPropertyValue: Feld nicht gefunden");
        if (pField)
        {
            SvxURLField* pURL = (SvxURLField*)pField;   // ist eine Kopie in der ScUnoEditEngine

            sal_Bool bOk = sal_True;
            if ( aNameString.EqualsAscii( SC_UNONAME_URL ) )
            {
                if (aValue >>= aStrVal)
                    pURL->SetURL( aStrVal );
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_REPR ) )
            {
                if (aValue >>= aStrVal)
                    pURL->SetRepresentation( aStrVal );
            }
            else if ( aNameString.EqualsAscii( SC_UNONAME_TARGET ) )
            {
                if (aValue >>= aStrVal)
                    pURL->SetTargetFrame( aStrVal );
            }
            else
                bOk = false;

            if (bOk)
            {
                pEditEngine->QuickInsertField( SvxFieldItem(*pField, EE_FEATURE_FIELD), aSelection );
                pEditSource->UpdateData();
            }
        }
    }
    else        // noch nicht eingefuegt
    {
        if ( aNameString.EqualsAscii( SC_UNONAME_URL ) )
        {
            if (aValue >>= aStrVal)
                aUrl = String( aStrVal );
        }
        else if ( aNameString.EqualsAscii( SC_UNONAME_REPR ) )
        {
            if (aValue >>= aStrVal)
                aRepresentation = String( aStrVal );
        }
        else if ( aNameString.EqualsAscii( SC_UNONAME_TARGET ) )
        {
            if (aValue >>= aStrVal)
                aTarget = String( aStrVal );
        }
    }
}

uno::Any SAL_CALL ScCellFieldObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Any aRet;
    String aNameString(aPropertyName);

    // anchor type is always "as character", text wrap always "none"

    if ( aNameString.EqualsAscii( SC_UNONAME_ANCTYPE ) )
        aRet <<= text::TextContentAnchorType_AS_CHARACTER;
    else if ( aNameString.EqualsAscii( SC_UNONAME_ANCTYPES ) )
    {
        uno::Sequence<text::TextContentAnchorType> aSeq(1);
        aSeq[0] = text::TextContentAnchorType_AS_CHARACTER;
        aRet <<= aSeq;
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_TEXTWRAP ) )
        aRet <<= text::WrapTextMode_NONE;
    else if (pEditSource)
    {
        //! Feld-Funktionen muessen an den Forwarder !!!
        ScEditEngineDefaulter* pEditEngine = ((ScCellEditSource*)pEditSource)->GetEditEngine();
        ScUnoEditEngine aTempEngine(pEditEngine);

        //  Typ egal (in Zellen gibts nur URLs)
        SvxFieldData* pField = aTempEngine.FindByPos( aSelection.nStartPara, aSelection.nStartPos, 0 );
        OSL_ENSURE(pField,"getPropertyValue: Feld nicht gefunden");
        if (pField)
        {
            SvxURLField* pURL = (SvxURLField*)pField;

            if ( aNameString.EqualsAscii( SC_UNONAME_URL ) )
                aRet <<= rtl::OUString( pURL->GetURL() );
            else if ( aNameString.EqualsAscii( SC_UNONAME_REPR ) )
                aRet <<= rtl::OUString( pURL->GetRepresentation() );
            else if ( aNameString.EqualsAscii( SC_UNONAME_TARGET ) )
                aRet <<= rtl::OUString( pURL->GetTargetFrame() );
        }
    }
    else        // noch nicht eingefuegt
    {
        if ( aNameString.EqualsAscii( SC_UNONAME_URL ) )
            aRet <<= rtl::OUString( aUrl );
        else if ( aNameString.EqualsAscii( SC_UNONAME_REPR ) )
            aRet <<= rtl::OUString( aRepresentation );
        else if ( aNameString.EqualsAscii( SC_UNONAME_TARGET ) )
            aRet <<= rtl::OUString( aTarget );
    }
    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScCellFieldObj )

// XUnoTunnel

sal_Int64 SAL_CALL ScCellFieldObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theScCellFieldObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScCellFieldObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScCellFieldObj::getUnoTunnelId()
{
    return theScCellFieldObjUnoTunnelId::get().getSeq();
}

ScCellFieldObj* ScCellFieldObj::getImplementation(
                                const uno::Reference<text::XTextContent> xObj )
{
    ScCellFieldObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScCellFieldObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

// XServiceInfo

rtl::OUString SAL_CALL ScCellFieldObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ScCellFieldObj" ));
}

sal_Bool SAL_CALL ScCellFieldObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCTEXTFIELD_SERVICE ) ||
           aServiceStr.EqualsAscii( SCTEXTCONTENT_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScCellFieldObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCTEXTFIELD_SERVICE ));
    pArray[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCTEXTCONTENT_SERVICE ));
    return aRet;
}

//------------------------------------------------------------------------

ScHeaderFieldsObj::ScHeaderFieldsObj(ScHeaderFooterContentObj* pContent, sal_uInt16 nP, sal_uInt16 nT) :
    pContentObj( pContent ),
    nPart( nP ),
    nType( nT ),
    mpRefreshListeners( NULL )
{
    OSL_ENSURE( pContentObj, "ScHeaderFieldsObj ohne Objekt?" );

    if (pContentObj)
    {
        pContentObj->acquire();     // darf nicht wegkommen
        pEditSource = new ScHeaderFooterEditSource( pContentObj, nPart );
    }
    else
        pEditSource = NULL;
}

ScHeaderFieldsObj::~ScHeaderFieldsObj()
{
    delete pEditSource;

    if (pContentObj)
        pContentObj->release();

    // increment refcount to prevent double call off dtor
    osl_incrementInterlockedCount( &m_refCount );

    if (mpRefreshListeners)
    {
        lang::EventObject aEvent;
        aEvent.Source = static_cast<cppu::OWeakObject*>(this);
        if (mpRefreshListeners)
        {
            mpRefreshListeners->disposeAndClear(aEvent);
            DELETEZ( mpRefreshListeners );
        }
    }
}

// XIndexAccess (via XTextFields)

ScHeaderFieldObj* ScHeaderFieldsObj::GetObjectByIndex_Impl(sal_Int32 Index) const
{
    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = ((ScHeaderFooterEditSource*)pEditSource)->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    TypeId aTypeId = NULL;
    switch (nType)
    {
        case SC_SERVICE_PAGEFIELD:  aTypeId = TYPE(SvxPageField);    break;
        case SC_SERVICE_PAGESFIELD: aTypeId = TYPE(SvxPagesField);   break;
        case SC_SERVICE_DATEFIELD:  aTypeId = TYPE(SvxDateField);    break;
        case SC_SERVICE_TIMEFIELD:  aTypeId = TYPE(SvxTimeField);    break;
        case SC_SERVICE_TITLEFIELD: aTypeId = TYPE(SvxFileField);    break;
        case SC_SERVICE_FILEFIELD:  aTypeId = TYPE(SvxExtFileField); break;
        case SC_SERVICE_SHEETFIELD: aTypeId = TYPE(SvxTableField);   break;
        // bei SC_SERVICE_INVALID bleibt TypeId Null
    }
    SvxFieldData* pData = aTempEngine.FindByIndex( (sal_uInt16)Index, aTypeId );
    if ( pData )
    {
        sal_uInt16 nPar = aTempEngine.GetFieldPar();
        xub_StrLen nPos = aTempEngine.GetFieldPos();

        sal_uInt16 nFieldType = nType;
        if ( nFieldType == SC_SERVICE_INVALID )
        {
            if ( pData->ISA( SvxPageField ) )         nFieldType = SC_SERVICE_PAGEFIELD;
            else if ( pData->ISA( SvxPagesField ) )   nFieldType = SC_SERVICE_PAGESFIELD;
            else if ( pData->ISA( SvxDateField ) )    nFieldType = SC_SERVICE_DATEFIELD;
            else if ( pData->ISA( SvxTimeField ) )    nFieldType = SC_SERVICE_TIMEFIELD;
            else if ( pData->ISA( SvxFileField ) )    nFieldType = SC_SERVICE_TITLEFIELD;
            else if ( pData->ISA( SvxExtFileField ) ) nFieldType = SC_SERVICE_FILEFIELD;
            else if ( pData->ISA( SvxTableField ) )   nFieldType = SC_SERVICE_SHEETFIELD;
        }

        ESelection aSelection( nPar, nPos, nPar, nPos+1 );      // Field is 1 character
        return new ScHeaderFieldObj( pContentObj, nPart, nFieldType, aSelection );
    }
    return NULL;
}

sal_Int32 SAL_CALL ScHeaderFieldsObj::getCount() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //! Feld-Funktionen muessen an den Forwarder !!!
    ScEditEngineDefaulter* pEditEngine = ((ScHeaderFooterEditSource*)pEditSource)->GetEditEngine();
    ScUnoEditEngine aTempEngine(pEditEngine);

    TypeId aTypeId = NULL;
    switch (nType)
    {
        case SC_SERVICE_PAGEFIELD:  aTypeId = TYPE(SvxPageField);    break;
        case SC_SERVICE_PAGESFIELD: aTypeId = TYPE(SvxPagesField);   break;
        case SC_SERVICE_DATEFIELD:  aTypeId = TYPE(SvxDateField);    break;
        case SC_SERVICE_TIMEFIELD:  aTypeId = TYPE(SvxTimeField);    break;
        case SC_SERVICE_TITLEFIELD: aTypeId = TYPE(SvxFileField);    break;
        case SC_SERVICE_FILEFIELD:  aTypeId = TYPE(SvxExtFileField); break;
        case SC_SERVICE_SHEETFIELD: aTypeId = TYPE(SvxTableField);   break;
    }
    return aTempEngine.CountFields(aTypeId);        // Felder zaehlen
}

uno::Any SAL_CALL ScHeaderFieldsObj::getByIndex( sal_Int32 nIndex )
                            throw(lang::IndexOutOfBoundsException,
                                    lang::WrappedTargetException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    uno::Reference<text::XTextField> xField(GetObjectByIndex_Impl(nIndex));
    if (xField.is())
        return uno::makeAny(xField);
    else
        throw lang::IndexOutOfBoundsException();
}

uno::Type SAL_CALL ScHeaderFieldsObj::getElementType() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return getCppuType((uno::Reference<text::XTextField>*)0);
}

sal_Bool SAL_CALL ScHeaderFieldsObj::hasElements() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return ( getCount() != 0 );
}

uno::Reference<container::XEnumeration> SAL_CALL ScHeaderFieldsObj::createEnumeration()
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    return new ScIndexEnumeration(this, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.TextFieldEnumeration")));
}

void SAL_CALL ScHeaderFieldsObj::addContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
                                    throw(uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

void SAL_CALL ScHeaderFieldsObj::removeContainerListener(
                                const uno::Reference<container::XContainerListener>& /* xListener */ )
                                    throw(uno::RuntimeException)
{
    OSL_FAIL("not implemented");
}

// XRefreshable
void SAL_CALL ScHeaderFieldsObj::refresh(  )
                                    throw (uno::RuntimeException)
{
    if (mpRefreshListeners)
    {
        //  Call all listeners.
        uno::Sequence< uno::Reference< uno::XInterface > > aListeners(mpRefreshListeners->getElements());
        sal_uInt32 nLength(aListeners.getLength());
        if (nLength)
        {
            const uno::Reference< uno::XInterface >* pInterfaces = aListeners.getConstArray();
            if (pInterfaces)
            {
                lang::EventObject aEvent;
                aEvent.Source.set(uno::Reference< util::XRefreshable >(const_cast<ScHeaderFieldsObj*>(this)));
                sal_uInt32 i(0);
                while (i < nLength)
                {
                    try
                    {
                        while(i < nLength)
                        {
                            static_cast< util::XRefreshListener* >(pInterfaces->get())->refreshed(aEvent);
                            ++pInterfaces;
                            ++i;
                        }
                    }
                    catch(uno::RuntimeException&)
                    {
                        ++pInterfaces;
                        ++i;
                    }
                }
            }
        }
    }
}

void SAL_CALL ScHeaderFieldsObj::addRefreshListener( const uno::Reference< util::XRefreshListener >& xListener )
                                    throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (!mpRefreshListeners)
            mpRefreshListeners = new cppu::OInterfaceContainerHelper(aMutex);
        mpRefreshListeners->addInterface(xListener);
    }
}

void SAL_CALL ScHeaderFieldsObj::removeRefreshListener( const uno::Reference<util::XRefreshListener >& xListener )
                                    throw (uno::RuntimeException)
{
    if (xListener.is())
    {
        SolarMutexGuard aGuard;
        if (mpRefreshListeners)
            mpRefreshListeners->removeInterface(xListener);
    }
}

//------------------------------------------------------------------------

SvxFileFormat lcl_UnoToSvxFileFormat( sal_Int16 nUnoValue )
{
    switch( nUnoValue )
    {
        case text::FilenameDisplayFormat::FULL: return SVXFILEFORMAT_FULLPATH;
        case text::FilenameDisplayFormat::PATH: return SVXFILEFORMAT_PATH;
        case text::FilenameDisplayFormat::NAME: return SVXFILEFORMAT_NAME;
        default:
            return SVXFILEFORMAT_NAME_EXT;
    }
}

sal_Int16 lcl_SvxToUnoFileFormat( SvxFileFormat nSvxValue )
{
    switch( nSvxValue )
    {
        case SVXFILEFORMAT_NAME_EXT:    return text::FilenameDisplayFormat::NAME_AND_EXT;
        case SVXFILEFORMAT_FULLPATH:    return text::FilenameDisplayFormat::FULL;
        case SVXFILEFORMAT_PATH:        return text::FilenameDisplayFormat::PATH;
        default:
            return text::FilenameDisplayFormat::NAME;
    }
}

ScHeaderFieldObj::ScHeaderFieldObj(ScHeaderFooterContentObj* pContent, sal_uInt16 nP,
                                            sal_uInt16 nT, const ESelection& rSel) :
    OComponentHelper( getMutex() ),
    pPropSet( (nT == SC_SERVICE_FILEFIELD) ? lcl_GetFileFieldPropertySet() : lcl_GetHeaderFieldPropertySet() ),
    pContentObj( pContent ),
    nPart( nP ),
    nType( nT ),
    aSelection( rSel ),
    nFileFormat( SVXFILEFORMAT_NAME_EXT )
{
    //  pContent ist Null, wenn per ServiceProvider erzeugt

    if (pContentObj)
    {
        pContentObj->acquire();     // darf nicht wegkommen
        pEditSource = new ScHeaderFooterEditSource( pContentObj, nPart );
    }
    else
        pEditSource = NULL;
}

uno::Any SAL_CALL ScHeaderFieldObj::queryAggregation( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    SC_QUERYINTERFACE( text::XTextField )
    SC_QUERYINTERFACE( text::XTextContent )         // parent of XTextField
    SC_QUERYINTERFACE( beans::XPropertySet )
    SC_QUERYINTERFACE( lang::XUnoTunnel )
    SC_QUERYINTERFACE( lang::XServiceInfo )

    return OComponentHelper::queryAggregation( rType );     // XComponent
}

uno::Sequence<uno::Type> SAL_CALL ScHeaderFieldObj::getTypes() throw(uno::RuntimeException)
{
    static uno::Sequence<uno::Type> aTypes;
    if ( aTypes.getLength() == 0 )
    {
        uno::Sequence<uno::Type> aParentTypes(OComponentHelper::getTypes());
        long nParentLen = aParentTypes.getLength();
        const uno::Type* pParentPtr = aParentTypes.getConstArray();

        aTypes.realloc( nParentLen + 4 );
        uno::Type* pPtr = aTypes.getArray();
        pPtr[nParentLen + 0] = getCppuType((const uno::Reference<text::XTextField>*)0);
        pPtr[nParentLen + 1] = getCppuType((const uno::Reference<beans::XPropertySet>*)0);
        pPtr[nParentLen + 2] = getCppuType((const uno::Reference<lang::XUnoTunnel>*)0);
        pPtr[nParentLen + 3] = getCppuType((const uno::Reference<lang::XServiceInfo>*)0);

        for (long i=0; i<nParentLen; i++)
            pPtr[i] = pParentPtr[i];                // parent types first
    }
    return aTypes;
}

namespace
{
    class theScHeaderFieldObjImplementationId : public rtl::Static< UnoTunnelIdInit, theScHeaderFieldObjImplementationId > {};
}

uno::Sequence<sal_Int8> SAL_CALL ScHeaderFieldObj::getImplementationId()
                                                    throw(uno::RuntimeException)
{
    return theScHeaderFieldObjImplementationId::get().getSeq();
}

uno::Any SAL_CALL ScHeaderFieldObj::queryInterface( const uno::Type& rType )
                                                throw(uno::RuntimeException)
{
    return OComponentHelper::queryInterface( rType );
}

void SAL_CALL ScHeaderFieldObj::acquire() throw()
{
    OComponentHelper::acquire();
}

void SAL_CALL ScHeaderFieldObj::release() throw()
{
    OComponentHelper::release();
}

void ScHeaderFieldObj::InitDoc( ScHeaderFooterContentObj* pContent, sal_uInt16 nP,
                                        const ESelection& rSel )
{
    if ( pContent && !pEditSource )
    {
        OSL_ENSURE( !pContentObj, "ContentObj, aber kein EditSource?" );

        aSelection = rSel;
        nPart = nP;
        pContentObj = pContent;

        pContentObj->acquire();     // darf nicht wegkommen
        pEditSource = new ScHeaderFooterEditSource( pContentObj, nPart );
    }
}

ScHeaderFieldObj::~ScHeaderFieldObj()
{
    delete pEditSource;

    if (pContentObj)
        pContentObj->release();
}

// per getImplementation gerufen:

SvxFieldItem ScHeaderFieldObj::CreateFieldItem()
{
    OSL_ENSURE( !pEditSource, "CreateFieldItem mit eingefuegtem Feld" );

    switch (nType)
    {
        case SC_SERVICE_PAGEFIELD:
            {
                SvxPageField aField;
                return SvxFieldItem( aField, EE_FEATURE_FIELD );
            }
        case SC_SERVICE_PAGESFIELD:
            {
                SvxPagesField aField;
                return SvxFieldItem( aField, EE_FEATURE_FIELD );
            }
        case SC_SERVICE_DATEFIELD:
            {
                SvxDateField aField;
                return SvxFieldItem( aField, EE_FEATURE_FIELD );
            }
        case SC_SERVICE_TIMEFIELD:
            {
                SvxTimeField aField;
                return SvxFieldItem( aField, EE_FEATURE_FIELD );
            }
        case SC_SERVICE_TITLEFIELD:
            {
                SvxFileField aField;
                return SvxFieldItem( aField, EE_FEATURE_FIELD );
            }
        case SC_SERVICE_FILEFIELD:
            {
                SvxExtFileField aField;
                aField.SetFormat( (SvxFileFormat) nFileFormat );
                return SvxFieldItem( aField, EE_FEATURE_FIELD );
            }
        case SC_SERVICE_SHEETFIELD:
            {
                SvxTableField aField;
                return SvxFieldItem( aField, EE_FEATURE_FIELD );
            }
    }

    return SvxFieldItem( SvxFieldData(), EE_FEATURE_FIELD );
}

void ScHeaderFieldObj::DeleteField()
{
    if (pEditSource)
    {
        SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
        pForwarder->QuickInsertText( String(), aSelection );
        pEditSource->UpdateData();

        aSelection.nEndPara = aSelection.nStartPara;
        aSelection.nEndPos  = aSelection.nStartPos;

        //! Broadcast, um Selektion in anderen Objekten anzupassen
        //! (auch bei anderen Aktionen)
    }
}

// XTextField

rtl::OUString SAL_CALL ScHeaderFieldObj::getPresentation( sal_Bool /* bShowCommand */ )
                                                    throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aRet;

    if (pEditSource)
    {
        // Feld von der EditEngine formatieren lassen, bShowCommand gibt's nicht

        SvxTextForwarder* pForwarder = pEditSource->GetTextForwarder();
        aRet = pForwarder->GetText( aSelection );
    }

    return aRet;
}

// XTextContent

void SAL_CALL ScHeaderFieldObj::attach( const uno::Reference<text::XTextRange>& xTextRange )
                                throw(lang::IllegalArgumentException, uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (xTextRange.is())
    {
        uno::Reference<text::XText> xText(xTextRange->getText());
        if (xText.is())
        {
            xText->insertTextContent( xTextRange, this, sal_True );
        }
    }
}

uno::Reference<text::XTextRange> SAL_CALL ScHeaderFieldObj::getAnchor() throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (pContentObj)
    {
        uno::Reference<text::XText> xText;
        if ( nPart == SC_HDFT_LEFT )
            xText = pContentObj->getLeftText();
        else if (nPart == SC_HDFT_CENTER)
            xText = pContentObj->getCenterText();
        else
            xText = pContentObj->getRightText();
        return uno::Reference<text::XTextRange>( xText, uno::UNO_QUERY );
    }
    return NULL;
}

// XComponent

void SAL_CALL ScHeaderFieldObj::dispose() throw(uno::RuntimeException)
{
    OComponentHelper::dispose();
}

void SAL_CALL ScHeaderFieldObj::addEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    OComponentHelper::addEventListener( xListener );
}

void SAL_CALL ScHeaderFieldObj::removeEventListener(
                        const uno::Reference<lang::XEventListener>& xListener )
                                                    throw(uno::RuntimeException)
{
    OComponentHelper::removeEventListener( xListener );
}

// XPropertySet

uno::Reference<beans::XPropertySetInfo> SAL_CALL ScHeaderFieldObj::getPropertySetInfo()
                                                        throw(uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    if (nType == SC_SERVICE_FILEFIELD)
    {
        //  file field has different properties
        static uno::Reference<beans::XPropertySetInfo> aFileFieldInfo = pPropSet->getPropertySetInfo();
        return aFileFieldInfo;
    }
    else
    {
        static uno::Reference<beans::XPropertySetInfo> aRef = pPropSet->getPropertySetInfo();
        return aRef;
    }
}

void SAL_CALL ScHeaderFieldObj::setPropertyValue(
                        const rtl::OUString& aPropertyName, const uno::Any& aValue )
                throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                        lang::IllegalArgumentException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;
    String aNameString(aPropertyName);
    if ( nType == SC_SERVICE_FILEFIELD && aNameString.EqualsAscii( SC_UNONAME_FILEFORM ) )
    {
        sal_Int16 nIntVal = 0;
        if ( aValue >>= nIntVal )
        {
            SvxFileFormat eFormat = lcl_UnoToSvxFileFormat( nIntVal );
            if (pEditSource)
            {
                ScEditEngineDefaulter* pEditEngine = ((ScHeaderFooterEditSource*)pEditSource)->GetEditEngine();
                ScUnoEditEngine aTempEngine(pEditEngine);
                SvxFieldData* pField = aTempEngine.FindByPos(
                        aSelection.nStartPara, aSelection.nStartPos, TYPE(SvxExtFileField) );
                OSL_ENSURE(pField,"setPropertyValue: Field not found");
                if (pField)
                {
                    SvxExtFileField* pExtFile = (SvxExtFileField*)pField;   // local to the ScUnoEditEngine
                    pExtFile->SetFormat( eFormat );
                    pEditEngine->QuickInsertField( SvxFieldItem(*pField, EE_FEATURE_FIELD), aSelection );
                    pEditSource->UpdateData();
                }
            }
            else
                nFileFormat = sal::static_int_cast<sal_Int16>(eFormat);     // not inserted yet - store value
        }
    }
}

uno::Any SAL_CALL ScHeaderFieldObj::getPropertyValue( const rtl::OUString& aPropertyName )
                throw(beans::UnknownPropertyException, lang::WrappedTargetException,
                        uno::RuntimeException)
{
    SolarMutexGuard aGuard;

    //! Properties?
    uno::Any aRet;
    String aNameString(aPropertyName);

    // anchor type is always "as character", text wrap always "none"

    if ( aNameString.EqualsAscii( SC_UNONAME_ANCTYPE ) )
        aRet <<= text::TextContentAnchorType_AS_CHARACTER;
    else if ( aNameString.EqualsAscii( SC_UNONAME_ANCTYPES ) )
    {
        uno::Sequence<text::TextContentAnchorType> aSeq(1);
        aSeq[0] = text::TextContentAnchorType_AS_CHARACTER;
        aRet <<= aSeq;
    }
    else if ( aNameString.EqualsAscii( SC_UNONAME_TEXTWRAP ) )
        aRet <<= text::WrapTextMode_NONE;
    else if ( nType == SC_SERVICE_FILEFIELD && aNameString.EqualsAscii( SC_UNONAME_FILEFORM ) )
    {
        SvxFileFormat eFormat = SVXFILEFORMAT_NAME_EXT;
        if (pEditSource)
        {
            ScEditEngineDefaulter* pEditEngine = ((ScHeaderFooterEditSource*)pEditSource)->GetEditEngine();
            ScUnoEditEngine aTempEngine(pEditEngine);
            SvxFieldData* pField = aTempEngine.FindByPos(
                    aSelection.nStartPara, aSelection.nStartPos, TYPE(SvxExtFileField) );
            OSL_ENSURE(pField,"setPropertyValue: Field not found");
            if (pField)
            {
                const SvxExtFileField* pExtFile = (const SvxExtFileField*)pField;
                eFormat = pExtFile->GetFormat();
            }
        }
        else
            eFormat = (SvxFileFormat) nFileFormat;      // not inserted yet - use stored value

        sal_Int16 nIntVal = lcl_SvxToUnoFileFormat( eFormat );
        aRet <<= nIntVal;
    }

    return aRet;
}

SC_IMPL_DUMMY_PROPERTY_LISTENER( ScHeaderFieldObj )

// XUnoTunnel

sal_Int64 SAL_CALL ScHeaderFieldObj::getSomething(
                const uno::Sequence<sal_Int8 >& rId ) throw(uno::RuntimeException)
{
    if ( rId.getLength() == 16 &&
          0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                    rId.getConstArray(), 16 ) )
    {
        return sal::static_int_cast<sal_Int64>(reinterpret_cast<sal_IntPtr>(this));
    }
    return 0;
}

namespace
{
    class theScHeaderFieldObjUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theScHeaderFieldObjUnoTunnelId> {};
}

const uno::Sequence<sal_Int8>& ScHeaderFieldObj::getUnoTunnelId()
{
    return theScHeaderFieldObjUnoTunnelId::get().getSeq();
}

ScHeaderFieldObj* ScHeaderFieldObj::getImplementation(
                                const uno::Reference<text::XTextContent> xObj )
{
    ScHeaderFieldObj* pRet = NULL;
    uno::Reference<lang::XUnoTunnel> xUT( xObj, uno::UNO_QUERY );
    if (xUT.is())
        pRet = reinterpret_cast<ScHeaderFieldObj*>(sal::static_int_cast<sal_IntPtr>(xUT->getSomething(getUnoTunnelId())));
    return pRet;
}

// XServiceInfo

rtl::OUString SAL_CALL ScHeaderFieldObj::getImplementationName() throw(uno::RuntimeException)
{
    return rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( "ScHeaderFieldObj" ));
}

sal_Bool SAL_CALL ScHeaderFieldObj::supportsService( const rtl::OUString& rServiceName )
                                                    throw(uno::RuntimeException)
{
    String aServiceStr( rServiceName );
    return aServiceStr.EqualsAscii( SCTEXTFIELD_SERVICE ) ||
           aServiceStr.EqualsAscii( SCTEXTCONTENT_SERVICE );
}

uno::Sequence<rtl::OUString> SAL_CALL ScHeaderFieldObj::getSupportedServiceNames()
                                                    throw(uno::RuntimeException)
{
    uno::Sequence<rtl::OUString> aRet(2);
    rtl::OUString* pArray = aRet.getArray();
    pArray[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCTEXTFIELD_SERVICE ));
    pArray[1] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( SCTEXTCONTENT_SERVICE ));
    return aRet;
}

//------------------------------------------------------------------------




/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
