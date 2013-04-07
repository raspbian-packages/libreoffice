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
 *   Copyright (C) 2011 Michael Meeks <michael.meeks@suse.com>
 *   Caolán McNamara <caolanm@redhat.com>
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
#include <test/bootstrapfixture.hxx>
#include <tools/errinf.hxx>
#include <rtl/strbuf.hxx>
#include <rtl/bootstrap.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <comphelper/processfactory.hxx>
#include <i18npool/mslangid.hxx>

#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>

#include <vcl/svapp.hxx>
#include <tools/resmgr.hxx>
#include <unotools/syslocaleoptions.hxx>

using namespace ::com::sun::star;

static void aBasicErrorFunc( const String &rErr, const String &rAction )
{
    rtl::OStringBuffer aErr( "Unexpected dialog: " );
    aErr.append( rtl::OUStringToOString( rAction, RTL_TEXTENCODING_ASCII_US ) );
    aErr.append( " Error: " );
    aErr.append( rtl::OUStringToOString( rErr, RTL_TEXTENCODING_ASCII_US ) );
    CPPUNIT_ASSERT_MESSAGE( aErr.getStr(), false);
}

// NB. this constructor is called before any tests are run, once for each
// test function in a rather non-intuitive way. This is why all the 'real'
// heavy lifting is deferred until setUp. setUp and tearDown are interleaved
// between the tests as you might expect.
test::BootstrapFixture::BootstrapFixture( bool bAssertOnDialog, bool bNeedUCB )
    : m_bNeedUCB( bNeedUCB )
    , m_bAssertOnDialog( bAssertOnDialog )
{
    // force locale (and resource files loaded) to en-US
    const LanguageType eLang=LANGUAGE_ENGLISH_US;

    rtl::OUString aLang, aCountry;
    MsLangId::convertLanguageToIsoNames(eLang, aLang, aCountry);
    lang::Locale aLocale(aLang, aCountry, rtl::OUString());
    ResMgr::SetDefaultLocale( aLocale );
}

void test::BootstrapFixture::setUp()
{
    test::BootstrapFixtureBase::setUp();
    if (m_bNeedUCB)
    {
        // initialise UCB-Broker
        uno::Sequence<uno::Any> aUcbInitSequence(2);
        aUcbInitSequence[0] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Local"));
        aUcbInitSequence[1] <<= rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Office"));
        bool bInitUcb = ucbhelper::ContentBroker::initialize(m_xSFactory, aUcbInitSequence);
        CPPUNIT_ASSERT_MESSAGE("Should be able to initialize UCB", bInitUcb);

        uno::Reference<ucb::XContentProviderManager> xUcb =
            ucbhelper::ContentBroker::get()->getContentProviderManagerInterface();
        uno::Reference<ucb::XContentProvider> xFileProvider(m_xSFactory->createInstance(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.ucb.FileContentProvider"))), uno::UNO_QUERY);
        xUcb->registerContentProvider(xFileProvider, rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("file")), sal_True);
    }

    SvtSysLocaleOptions aLocalOptions;
    rtl::OUString aLangISO = MsLangId::convertLanguageToIsoString( LANGUAGE_ENGLISH_US );
    aLocalOptions.SetLocaleConfigString( aLangISO );
    aLocalOptions.SetUILocaleConfigString( aLangISO );

    InitVCL(m_xSFactory);
    if (Application::IsHeadlessModeRequested()) {
        Application::EnableHeadlessMode(true);
    }

    if( m_bAssertOnDialog )
        ErrorHandler::RegisterDisplay( aBasicErrorFunc );
}

void test::BootstrapFixture::tearDown()
{
    ucbhelper::ContentBroker::deinitialize();
    test::BootstrapFixtureBase::tearDown();
}

test::BootstrapFixture::~BootstrapFixture()
{
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
