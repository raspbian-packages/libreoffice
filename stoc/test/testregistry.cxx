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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sal/main.h>
#include <osl/module.hxx>
#include <osl/diagnose.h>
#include <osl/process.h>
#include <registry/registry.hxx>


#include <com/sun/star/registry/XSimpleRegistry.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <cppuhelper/factory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/servicefactory.hxx>

#include <com/sun/star/lang/XComponent.hpp>

#if defined ( UNX )
#include <limits.h>
#define _MAX_PATH PATH_MAX
#endif

using namespace com::sun::star;
using namespace com::sun::star::uno;
using namespace com::sun::star::registry;
using namespace com::sun::star::lang;
using namespace com::sun::star::beans;
using namespace osl;

using ::rtl::OUString;
using ::rtl::OUStringToOString;
using ::rtl::OString;

#if OSL_DEBUG_LEVEL > 0
#define TEST_ENSHURE(c, m)   OSL_ENSURE(c, m)
#else
#define TEST_ENSHURE(c, m)   OSL_VERIFY(c)
#endif

namespace stoc_impreg
{
void SAL_CALL mergeKeys(
Reference< registry::XRegistryKey > const & xDest,
Reference< registry::XRegistryKey > const & xSource )
SAL_THROW( (registry::InvalidRegistryException, registry::MergeConflictException) );
}
static void mergeKeys(
Reference< registry::XSimpleRegistry > const & xDest,
OUString const & rBaseNode,
OUString const & rURL )
SAL_THROW( (registry::InvalidRegistryException, registry::MergeConflictException) )
{
Reference< registry::XRegistryKey > xDestRoot( xDest->getRootKey() );
Reference< registry::XRegistryKey > xDestKey;
if (rBaseNode.getLength())
{
xDestKey = xDestRoot->createKey( rBaseNode );
xDestRoot->closeKey();
}
else
{
xDestKey = xDestRoot;
}
Reference< registry::XSimpleRegistry > xSimReg( ::cppu::createSimpleRegistry() );
xSimReg->open( rURL, sal_True, sal_False );
OSL_ASSERT( xSimReg->isValid() );
Reference< registry::XRegistryKey > xSourceKey( xSimReg->getRootKey() );
::stoc_impreg::mergeKeys( xDestKey, xSourceKey );
xSourceKey->closeKey();
xSimReg->close();
xDestKey->closeKey();
}


OString userRegEnv("STAR_USER_REGISTRY=");

OUString getExePath()
{
OUString        exe;
OSL_VERIFY( osl_getExecutableFile( &exe.pData ) == osl_Process_E_None);
#if defined(WIN32) || defined(WNT)
exe = exe.copy(0, exe.getLength() - 16);
#else
exe = exe.copy(0, exe.getLength() - 12);
#endif
return exe;
}

void setStarUserRegistry()
{
Registry *myRegistry = new Registry();

RegistryKey rootKey, rKey, rKey2;

OUString userReg = getExePath();
userReg += OUString(RTL_CONSTASCII_USTRINGPARAM("user.rdb"));
if(myRegistry->open(userReg, REG_READWRITE))
{
TEST_ENSHURE(!myRegistry->create(userReg), "setStarUserRegistry error 1");
}

TEST_ENSHURE(!myRegistry->close(), "setStarUserRegistry error 9");
delete myRegistry;

userRegEnv += OUStringToOString(userReg, RTL_TEXTENCODING_ASCII_US);
    putenv((char *)userRegEnv.getStr());
}

void setLinkInDefaultRegistry(const OUString& linkName, const OUString& linkTarget)
{
    Registry *myRegistry = new Registry();

    RegistryKey rootKey;

    OUString appReg = getExePath();
    appReg += OUString(RTL_CONSTASCII_USTRINGPARAM("stoctest.rdb"));

    TEST_ENSHURE(!myRegistry->open(appReg, REG_READWRITE), "setLinkInDefaultRegistry error 1");
    TEST_ENSHURE(!myRegistry->openRootKey(rootKey), "setLinkInDefaultRegistry error 2");

    TEST_ENSHURE(!rootKey.createLink(linkName, linkTarget), "setLinkInDefaultRegistry error 3");

    TEST_ENSHURE(!rootKey.closeKey(),  "setLinkInDefaultRegistry error 4");
    TEST_ENSHURE(!myRegistry->close(), "setLinkInDefaultRegistry error 5");

    delete myRegistry;
}


void test_SimpleRegistry(
    OUString const & testreg,
    OUString const & testreg2,
    bool bMergeDifferently = true )
{
    Reference<XInterface> xIFace;
    Module module;

    OUString dllName(
        RTL_CONSTASCII_USTRINGPARAM("simplereg.uno" SAL_DLLEXTENSION) );

    if (module.load(dllName))
    {
        // try to get provider from module
        component_getFactoryFunc pCompFactoryFunc = (component_getFactoryFunc)
            module.getFunctionSymbol( OUString(RTL_CONSTASCII_USTRINGPARAM(COMPONENT_GETFACTORY)) );

        if (pCompFactoryFunc)
        {
            XSingleServiceFactory * pRet = (XSingleServiceFactory *)
                (*pCompFactoryFunc)(
                    "com.sun.star.comp.stoc.SimpleRegistry", 0, 0 );
            if (pRet)
            {
                xIFace = pRet;
                pRet->release();
            }
        }
    }

    TEST_ENSHURE( xIFace.is(), "test_SimpleRegistry error1");

    Reference<XSingleServiceFactory> xFactory( Reference<XSingleServiceFactory>::query(xIFace) );
     xIFace.clear();

    TEST_ENSHURE( xFactory.is(), "testloader error11");

    Reference<XInterface> xIFace2 = xFactory->createInstance();
    xFactory.clear();

    TEST_ENSHURE( xIFace2.is(), "testloader error12");

    Reference<XServiceInfo> xServInfo( Reference<XServiceInfo>::query(xIFace2) );

    TEST_ENSHURE( xServInfo.is(), "test_SimpleRegistry error2");

    TEST_ENSHURE( xServInfo->getImplementationName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("com.sun.star.comp.stoc.SimpleRegistry") ), "test_SimpleRegistry error3");
    TEST_ENSHURE( xServInfo->supportsService(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.SimpleRegistry"))), "test_SimpleRegistry error4");
    TEST_ENSHURE( xServInfo->getSupportedServiceNames().getLength() == 1, "test_SimpleRegistry error5");
    xServInfo.clear();

    Reference<XSimpleRegistry> xReg( Reference<XSimpleRegistry>::query(xIFace2) );
    xIFace2.clear();

    TEST_ENSHURE( xReg.is(), "test_SimpleRegistry error6");

    try
    {
        xReg->open(testreg, sal_False, sal_True);

        TEST_ENSHURE( xReg->isValid() != sal_False, "test_SimpleRegistry error 7" );
        TEST_ENSHURE( xReg->isReadOnly() == sal_False, "test_SimpleRegistry error 8" );

        Reference<XRegistryKey> xRootKey(xReg->getRootKey());
        TEST_ENSHURE( xRootKey->isValid(), "test_SimpleRegistry error 9" );

        Reference<XRegistryKey> xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("FirstKey") ));

        Reference<XRegistryKey> xSubKey = xKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("FirstSubKey") ));
        xSubKey->setLongValue(123456789);

        xSubKey = xKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("SecondSubKey") ));
        xSubKey->setAsciiValue(OUString( RTL_CONSTASCII_USTRINGPARAM("ich bin ein acsii value") ));

        xSubKey = xKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("ThirdSubKey") ));
        xSubKey->setStringValue(OUString( RTL_CONSTASCII_USTRINGPARAM("ich bin ein unicode value") ));

        xSubKey = xKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("FourthSubKey") ));
        Sequence<sal_Int8> aSeq((sal_Int8*)"ich bin ein binary value", 25);
        xSubKey->setBinaryValue(aSeq);

        Sequence<OUString> seqNames = xKey->getKeyNames();
        Sequence< Reference<XRegistryKey> > seqKeys = xKey->openKeys();

        OUString name;
        for (sal_Int32 i=0; i < seqNames.getLength(); i++)
        {
            name = seqNames.getArray()[i];
            xSubKey = seqKeys.getArray()[i];

            if (name == OUString( RTL_CONSTASCII_USTRINGPARAM("/FirstKey/FirstSubKey") ))
            {
                TEST_ENSHURE( xSubKey->getLongValue() == 123456789,
                            "test_SimpleRegistry error 10" );
            } else
            if (name == OUString( RTL_CONSTASCII_USTRINGPARAM("/FirstKey/SecondSubKey") ))
            {
                TEST_ENSHURE( xSubKey->getAsciiValue() == OUString( RTL_CONSTASCII_USTRINGPARAM("ich bin ein acsii value") ),
                            "test_SimpleRegistry error 11" );
            } else
            if (name == OUString( RTL_CONSTASCII_USTRINGPARAM("/FirstKey/ThirdSubKey") ))
            {
                TEST_ENSHURE( xSubKey->getStringValue() == OUString( RTL_CONSTASCII_USTRINGPARAM("ich bin ein unicode value") ),
                            "test_SimpleRegistry error 12" );
            } else
            if (name == OUString( RTL_CONSTASCII_USTRINGPARAM("/FirstKey/FourthSubKey") ))
            {
                Sequence<sal_Int8> seqByte = xSubKey->getBinaryValue();
                TEST_ENSHURE(!strcmp(((const char*)seqByte.getArray()), "ich bin ein binary value"),
                            "test_SimpleRegistry error 13" );
            }

            seqKeys.getArray()[i]->closeKey();
        }

        xKey->closeKey();

        xRootKey->deleteKey(OUString( RTL_CONSTASCII_USTRINGPARAM("FirstKey") ));
        xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("SecondFirstKey" )));

        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("SecondKey") ));
        sal_Int32 pLongs[3] = {123, 456, 789};
        Sequence<sal_Int32> seqLongs(pLongs, 3);
        xKey->setLongListValue(seqLongs);

        Sequence<sal_Int32> seqLongs2;
        seqLongs2 = xKey->getLongListValue();
        TEST_ENSHURE( seqLongs.getLength() == 3, "test_SimpleRegistry error 14" );
        TEST_ENSHURE( seqLongs.getArray()[0] == 123, "test_SimpleRegistry error 15" );
        TEST_ENSHURE( seqLongs.getArray()[1] == 456, "test_SimpleRegistry error 16" );
        TEST_ENSHURE( seqLongs.getArray()[2] == 789, "test_SimpleRegistry error 17" );


        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("ThirdKey") ));
        OUString pAscii[3];
        pAscii[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("Hallo") );
        pAscii[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("jetzt komm") );
        pAscii[2] = OUString( RTL_CONSTASCII_USTRINGPARAM("ich") );

        Sequence<OUString> seqAscii(pAscii, 3);
        xKey->setAsciiListValue(seqAscii);

        Sequence<OUString> seqAscii2;
        seqAscii2 = xKey->getAsciiListValue();
        TEST_ENSHURE( seqAscii2.getLength() == 3, "test_SimpleRegistry error 18" );
        TEST_ENSHURE( seqAscii2.getArray()[0] == OUString( RTL_CONSTASCII_USTRINGPARAM("Hallo") ), "test_SimpleRegistry error 19");
        TEST_ENSHURE( seqAscii2.getArray()[1] == OUString( RTL_CONSTASCII_USTRINGPARAM("jetzt komm") ), "test_SimpleRegistry error 20");
        TEST_ENSHURE( seqAscii2.getArray()[2] == OUString( RTL_CONSTASCII_USTRINGPARAM("ich") ), "test_SimpleRegistry error 21");

        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("FourthKey") ));
        OUString pUnicode[3];
        pUnicode[0] = OUString( RTL_CONSTASCII_USTRINGPARAM("Hallo") );
        pUnicode[1] = OUString( RTL_CONSTASCII_USTRINGPARAM("jetzt komm") );
        pUnicode[2] = OUString( RTL_CONSTASCII_USTRINGPARAM("ich als unicode") );

        Sequence<OUString> seqUnicode(pUnicode, 3);
        xKey->setStringListValue(seqUnicode);

        Sequence<OUString> seqUnicode2;
        seqUnicode2 = xKey->getStringListValue();
        TEST_ENSHURE( seqUnicode2.getLength() == 3, "test_SimpleRegistry error 22" );
        TEST_ENSHURE( seqUnicode2.getArray()[0] == OUString( RTL_CONSTASCII_USTRINGPARAM("Hallo") ), "test_SimpleRegistry error 23");
        TEST_ENSHURE( seqUnicode2.getArray()[1] == OUString( RTL_CONSTASCII_USTRINGPARAM("jetzt komm") ), "test_SimpleRegistry error 24");
        TEST_ENSHURE( seqUnicode2.getArray()[2] == OUString( RTL_CONSTASCII_USTRINGPARAM("ich als unicode") ), "test_SimpleRegistry error 25");


        xReg->open(testreg2, sal_False, sal_True);
        TEST_ENSHURE( xReg->isValid() != sal_False, "test_SimpleRegistry error 25" );
        xRootKey = xReg->getRootKey();
        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("ThirdKey/FirstSubKey/WithSubSubKey") ));
        xKey->closeKey();
        TEST_ENSHURE(
            xRootKey->createLink(
                OUString( RTL_CONSTASCII_USTRINGPARAM("LinkTest") ),
                OUString( RTL_CONSTASCII_USTRINGPARAM("/ThirdKey/FirstSubKey/WithSubSubKey") )),
            "test_SimpleRegistry error 1212" );
        xRootKey->closeKey();
        xReg->close();

        xReg->open(testreg, sal_False, sal_False);
        TEST_ENSHURE( xReg->isValid() != sal_False, "test_SimpleRegistry error 26" );

        if (bMergeDifferently)
        {
            mergeKeys(
                xReg,
                OUString(),
                testreg2 );
        }
        else
        {
            xReg->mergeKey(OUString(), testreg2);
        }

        xRootKey = xReg->getRootKey();
        xKey = xRootKey->openKey( OUString( RTL_CONSTASCII_USTRINGPARAM("LinkTest") ) );
        TEST_ENSHURE(
            xKey.is() && xKey->isValid() &&
            xKey->getKeyName().equalsAsciiL( RTL_CONSTASCII_STRINGPARAM("/ThirdKey/FirstSubKey/WithSubSubKey") ),
            "test_SimpleRegistry error 1213" );
        xKey->closeKey();
        TEST_ENSHURE(
            xRootKey->getKeyType( OUString( RTL_CONSTASCII_USTRINGPARAM("LinkTest") ) ) ==
            registry::RegistryKeyType_LINK,
            "test_SimpleRegistry error 1214" );

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("FirstKey/SecondSubKey") ));
        TEST_ENSHURE( !xKey.is(), "test_SimpleRegistry error 27" );

        // Test Links
        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("FifthKey") ));
        xKey->createLink(OUString( RTL_CONSTASCII_USTRINGPARAM("MyFirstLink") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("/ThirdKey/FirstSubKey") ));

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/FifthKey/MyFirstLink") ));
        TEST_ENSHURE( xKey->isValid(), "test_SimpleRegistry error 27" );
        TEST_ENSHURE( xKey->getKeyName() == OUString( RTL_CONSTASCII_USTRINGPARAM("/ThirdKey/FirstSubKey") ), "test_SimpleRegistry error 28" );

        xKey->createLink(OUString( RTL_CONSTASCII_USTRINGPARAM("/WithSubSubKey/MyFourthLink") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("/FourthKey/MySecondLink") ));

        TEST_ENSHURE( xKey->getLinkTarget(OUString( RTL_CONSTASCII_USTRINGPARAM("/WithSubSubKey/MyFourthLink") ))
                     == OUString( RTL_CONSTASCII_USTRINGPARAM("/FourthKey/MySecondLink") ), "test_SimpleRegistry error 29" );

        try
        {
            TEST_ENSHURE( xKey->getResolvedName(OUString( RTL_CONSTASCII_USTRINGPARAM("/WithSubSubKey/MyFourthLink/BlaBlaBla") ))
                         == OUString( RTL_CONSTASCII_USTRINGPARAM("/FourthKey/MySecondLink/BlaBlaBla") ), "test_SimpleRegistry error 30" );
        }
        catch(InvalidRegistryException&)
        {
        }

        xRootKey->createLink(OUString( RTL_CONSTASCII_USTRINGPARAM("/FourthKey/MySecondLink") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("/SixthKey/MyThirdLink") ));
        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("SixthKey") ));
        xKey->createLink(OUString( RTL_CONSTASCII_USTRINGPARAM("MyThirdLink") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("/FourthKey/MySecondLink") ));

        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/SixthKey/SixthSubKey") ));

        try
        {
            xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink") ));
        }
        catch(InvalidRegistryException&)
        {
        }

        TEST_ENSHURE( xRootKey->getLinkTarget(OUString( RTL_CONSTASCII_USTRINGPARAM("/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink") ))
                     == OUString( RTL_CONSTASCII_USTRINGPARAM("/FourthKey/MySecondLink") ), "test_SimpleRegistry error 31" );

        xRootKey->deleteLink(OUString( RTL_CONSTASCII_USTRINGPARAM("/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink") ));

        xRootKey->createLink(OUString( RTL_CONSTASCII_USTRINGPARAM("/FourthKey/MySecondLink") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("/ThirdKey/FirstSubKey/WithSubSubKey") ));

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("SixthKey") ));
        seqNames = xKey->getKeyNames();
        seqKeys = xKey->openKeys();

        TEST_ENSHURE( seqNames.getArray()[0] == OUString( RTL_CONSTASCII_USTRINGPARAM("/SixthKey/SixthSubKey") ),
                      "test_SimpleRegistry error 32" );
        TEST_ENSHURE( seqNames.getArray()[1] == OUString( RTL_CONSTASCII_USTRINGPARAM("/SixthKey/MyThirdLink") ),
                      "test_SimpleRegistry error 33" );

        TEST_ENSHURE( seqKeys.getArray()[0]->getKeyName() == OUString( RTL_CONSTASCII_USTRINGPARAM("/SixthKey/SixthSubKey") ),
                      "test_SimpleRegistry error 34" );
        TEST_ENSHURE( seqKeys.getArray()[1]->getKeyName() == OUString( RTL_CONSTASCII_USTRINGPARAM("/ThirdKey/FirstSubKey/WithSubSubKey") ),
                      "test_SimpleRegistry error 35" );

        xRootKey->deleteLink(OUString( RTL_CONSTASCII_USTRINGPARAM("/FourthKey/MySecondLink") ));
        xRootKey->closeKey();
    }
    catch(InvalidRegistryException&)
    {
        TEST_ENSHURE(0, "exception InvalidRegistryExcption raised while doing test_SimpleRegistry");
    }
    catch(InvalidValueException&)
    {
        TEST_ENSHURE(0, "exception InvalidValueExcption raised while doing test_SimpleRegistry()");
    }

    xReg.clear();

    printf("Test SimpleRegistry, OK!\n");
}


void test_DefaultRegistry(
    OUString const & testreg,
    OUString const & testreg2,
    bool bMergeDifferently = false )
{
    // Test NestedRegistry
    OUString exePath( getExePath() );
    OUString userRdb(exePath);
    OUString applicatRdb(exePath);

    userRdb += OUString(RTL_CONSTASCII_USTRINGPARAM("user.rdb"));
    applicatRdb += OUString(RTL_CONSTASCII_USTRINGPARAM("stoctest.rdb"));

    Reference < XMultiServiceFactory > rSMgr  = ::cppu::createRegistryServiceFactory( userRdb, applicatRdb, sal_False, OUString());
                                                                                      //OUString(RTL_CONSTASCII_USTRINGPARAM("//./e:/src596/stoc/wntmsci3/bin")) );

    Reference< XPropertySet > xPropSet( rSMgr, UNO_QUERY);
    TEST_ENSHURE( xPropSet.is(), "test_DefaultRegistry error0");

    Any aPropertyAny( xPropSet->getPropertyValue( OUString(RTL_CONSTASCII_USTRINGPARAM("Registry")) ) );
    TEST_ENSHURE( aPropertyAny.hasValue(), "test_DefaultRegistry error1");

    Reference<XSimpleRegistry> xReg;
    aPropertyAny >>= xReg;
    TEST_ENSHURE( xReg.is(), "test_DefaultRegistry error1a");

    Reference<XServiceInfo> xServInfo( Reference<XServiceInfo>::query(xReg) );

    TEST_ENSHURE( xServInfo.is(), "test_DefaultRegistry error2");

    TEST_ENSHURE( xServInfo->getImplementationName() == OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.stoc.NestedRegistry") ), "test_DefualtRegistry error3");
    TEST_ENSHURE( xServInfo->supportsService(OUString( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.registry.NestedRegistry") )), "test_DefaultRegistry error4");
    TEST_ENSHURE( xServInfo->getSupportedServiceNames().getLength() == 1, "test_DefaultRegistry error5");
    xServInfo.clear();

    TEST_ENSHURE( xReg.is(), "test_DefaultRegistry error6");

    try
    {
        Reference<XRegistryKey> xRootKey(xReg->getRootKey());

        Reference<XRegistryKey> xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/UCR/com/sun/star/registry/XSimpleRegistry") ));

        TEST_ENSHURE( xKey->getKeyName() == OUString( RTL_CONSTASCII_USTRINGPARAM("/UCR/com/sun/star/registry/XSimpleRegistry") ),
                     "test_DefaultRegistry error 7" );

        if (bMergeDifferently)
        {
            mergeKeys(
                xReg,
                OUString( RTL_CONSTASCII_USTRINGPARAM("Test") ),
                testreg );
        }
        else
        {
            xReg->mergeKey(OUString( RTL_CONSTASCII_USTRINGPARAM("Test") ), testreg );
        }

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("Test/ThirdKey/FirstSubKey/WithSubSubKey") ));
        if (xKey.is())
            xKey->setLongValue(123456789);

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("Test/ThirdKey/FirstSubKey") ));
        if (xKey.is())
        {
            xKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("SecondSubSubKey") ));

            Sequence<OUString> seqNames = xKey->getKeyNames();

            TEST_ENSHURE( seqNames.getLength() == 2, "test_DefaultRegistry error 8" );
        }

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/ThirdKey") ));
        if (xKey.is())
        {
            RegistryValueType valueType = xKey->getValueType();
            TEST_ENSHURE( valueType == RegistryValueType_ASCIILIST, "test_DefaultRegistry error 9" );

            Sequence<OUString> seqValue = xKey->getAsciiListValue();

            TEST_ENSHURE( seqValue.getLength() == 3, "test_DefaultRegistry error 10" );
            TEST_ENSHURE( seqValue.getArray()[0] == OUString( RTL_CONSTASCII_USTRINGPARAM("Hallo") ),
                          "test_DefaultRegistry error 11" );
            TEST_ENSHURE( seqValue.getArray()[1] == OUString( RTL_CONSTASCII_USTRINGPARAM("jetzt komm") ),
                          "test_DefaultRegistry error 12" );
            TEST_ENSHURE( seqValue.getArray()[2] == OUString( RTL_CONSTASCII_USTRINGPARAM("ich") ),
                          "test_DefaultRegistry error 13" );

            Sequence<sal_Int32> seqLong(3);
            seqLong.getArray()[0] = 1234;
            seqLong.getArray()[1] = 4567;
            seqLong.getArray()[2] = 7890;

            xKey->setLongListValue(seqLong);

            Sequence<sal_Int32> seqLongValue = xKey->getLongListValue();

            TEST_ENSHURE( seqLongValue.getLength() == 3, "test_DefaultRegistry error 14" );
            TEST_ENSHURE( seqLongValue.getArray()[0] == 1234, "test_DefaultRegistry error 15" );
            TEST_ENSHURE( seqLongValue.getArray()[1] == 4567, "test_DefaultRegistry error 16" );
            TEST_ENSHURE( seqLongValue.getArray()[2] == 7890, "test_DefaultRegistry error 17" );
        }

        // Test Links
        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FifthKey") ));
        xKey->createLink(OUString( RTL_CONSTASCII_USTRINGPARAM("MyFirstLink") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/ThirdKey/FirstSubKey") ));

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FifthKey/MyFirstLink") ));
        TEST_ENSHURE( xKey->isValid(), "test_DefaultRegistry error 18" );
        TEST_ENSHURE( xKey->getKeyName() == OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/ThirdKey/FirstSubKey") ),
                      "test_DefaultRegistry error 19" );

        xKey->createLink(OUString( RTL_CONSTASCII_USTRINGPARAM("/WithSubSubKey/MyFourthLink") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FourthKey/MySecondLink") ));

        TEST_ENSHURE( xKey->getLinkTarget(OUString( RTL_CONSTASCII_USTRINGPARAM("/WithSubSubKey/MyFourthLink") ))
                     == OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FourthKey/MySecondLink") ),
                      "test_DefaultRegistry error 20" );

        try
        {
            TEST_ENSHURE( xKey->getResolvedName(OUString( RTL_CONSTASCII_USTRINGPARAM("/WithSubSubKey/MyFourthLink/BlaBlaBla") ))
                          == OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FourthKey/MySecondLink/BlaBlaBla") ),
                          "test_DefaultRegistry error 21" );
        }
        catch(InvalidRegistryException&)
        {
        }

        xRootKey->createLink(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FourthKey/MySecondLink") ),
                             OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/SixthKey/MyThirdLink") ));
        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/SixthKey") ));
        xKey->createLink(OUString( RTL_CONSTASCII_USTRINGPARAM("MyThirdLink") ),
                         OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FourthKey/MySecondLink") ));

        try
        {
            xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink") ));
        }
        catch(InvalidRegistryException&)
        {
            printf("test InvalidRegistryExcption OK!\n");
        }

        TEST_ENSHURE( xRootKey->getLinkTarget(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink") ))
                     == OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FourthKey/MySecondLink") ),
                      "test_DefaultRegistry error 22" );

        xRootKey->deleteLink(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/FifthKey/MyFirstLink/WithSubSubKey/MyFourthLink") ));

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/DefaultLink/SecondSubSubKey") ));
        if (xKey.is())
        {
            TEST_ENSHURE( xKey->getKeyName() == OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/ThirdKey/FirstSubKey/SecondSubSubKey") ), "test_DefaultRegistry error 23" );
        }
        xKey = xRootKey->createKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/DefaultLink/ThirdSubSubKey") ));
        if (xKey.is())
        {
            TEST_ENSHURE( xKey->getKeyName() == OUString( RTL_CONSTASCII_USTRINGPARAM("/Test/ThirdKey/FirstSubKey/ThirdSubSubKey") ),
                          "test_DefaultRegistry error 24" );
        }

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("Test") ));
        TEST_ENSHURE( xKey->isValid(), "test_DefaultRegistry error 25" );

        xRootKey->deleteKey(OUString( RTL_CONSTASCII_USTRINGPARAM("Test") ));

        if (bMergeDifferently)
        {
            mergeKeys(
                xReg,
                OUString( RTL_CONSTASCII_USTRINGPARAM("AllFromTestreg2") ),
                testreg2);
        }
        else
        {
            xReg->mergeKey(OUString( RTL_CONSTASCII_USTRINGPARAM("AllFromTestreg2") ),
                           testreg2);
        }

        xKey = xRootKey->openKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/AllFromTestreg2/ThirdKey/FirstSubKey") ));
        if (xKey.is())
        {
            xRootKey->deleteKey(OUString( RTL_CONSTASCII_USTRINGPARAM("/AllFromTestreg2") ));
        }

    }
    catch(InvalidRegistryException&)
    {
        TEST_ENSHURE(0, "exception InvalidRegistryExcption raised while doing test_DefaultRegistry");
    }
    catch(InvalidValueException&)
    {
        TEST_ENSHURE(0, "exception InvalidValueExcption raised while doing test_DefaultRegistry()");
    }
    try
    {
        xReg->close();
    }
    catch(InvalidRegistryException& e)
    {
        (void)e;
        TEST_ENSHURE(0, OUStringToOString(e.Message,RTL_TEXTENCODING_ASCII_US).getStr());
    }


    xReg.clear();

    // shutdown
    Reference< ::com::sun::star::lang::XComponent > xComp( rSMgr, UNO_QUERY );
    OSL_ENSURE( xComp.is(), "### serivce manager has to implement XComponent!" );
    xComp->dispose();

    printf("Test DefaultRegistry, OK!\n");
}


SAL_IMPLEMENT_MAIN()
{
//  setStarUserRegistry();
     setLinkInDefaultRegistry(OUString(RTL_CONSTASCII_USTRINGPARAM("/Test/DefaultLink")),
                              OUString(RTL_CONSTASCII_USTRINGPARAM("/Test/FifthKey/MyFirstLink")));

    OUString reg1( RTL_CONSTASCII_USTRINGPARAM("testreg1.rdb") );
    OUString reg2( RTL_CONSTASCII_USTRINGPARAM("testreg2.rdb") );
    OUString areg1( RTL_CONSTASCII_USTRINGPARAM("atestreg1.rdb") );
    OUString areg2( RTL_CONSTASCII_USTRINGPARAM("atestreg2.rdb") );

      test_SimpleRegistry( reg1, reg2 );
      test_DefaultRegistry( reg1, reg2 );
      test_SimpleRegistry( areg1, areg2, true ); // use different merge
      test_DefaultRegistry( areg1, areg2, true );

    Reference< XSimpleRegistry > xSimReg( ::cppu::createSimpleRegistry() );
    xSimReg->open( reg1, sal_False, sal_True );
    xSimReg->destroy();
    xSimReg->open( reg2, sal_False, sal_True );
    xSimReg->destroy();
    xSimReg->open( areg1, sal_False, sal_True );
    xSimReg->destroy();
    xSimReg->open( areg2, sal_False, sal_True );
    xSimReg->destroy();
    return(0);
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
