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

// autogenerated file with codegen.pl

#include "sal/config.h"
#include "sal/precppunit.hxx"

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include <basegfx/vector/b2isize.hxx>
#include <basegfx/range/b2ibox.hxx>
#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>

#include <basebmp/color.hxx>
#include <basebmp/scanlineformats.hxx>
#include <basebmp/bitmapdevice.hxx>
#include "tools.hxx"

using namespace ::basebmp;

namespace
{
class MaskTest : public CppUnit::TestFixture
{
private:
    BitmapDeviceSharedPtr mpDevice1bpp;
    BitmapDeviceSharedPtr mpDevice32bpp;
    BitmapDeviceSharedPtr mpMask;

    void implTestMaskBasics(const BitmapDeviceSharedPtr& rDevice,
                            const BitmapDeviceSharedPtr& rBmp)
    {
        const Color aCol(0);
        const Color aCol2(0xF0F0F0F0);

        const basegfx::B2IBox aSourceRect(0,0,10,10);
        const basegfx::B2IPoint aDestLeftTop(0,0);
        const basegfx::B2IPoint aDestRightTop(5,0);
        const basegfx::B2IPoint aDestLeftBottom(0,5);
        const basegfx::B2IPoint aDestRightBottom(5,5);

        rDevice->clear(aCol);
        rDevice->setPixel(
            basegfx::B2IPoint(1,1),
            aCol2,
            DrawMode_PAINT);
        rDevice->drawMaskedColor(
            aCol2,
            rBmp,
            aSourceRect,
            aDestLeftTop );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 50",
                               countPixel( rDevice, aCol ) == 100-50);

        rDevice->clear(aCol);
        rDevice->drawMaskedColor(
            aCol2,
            rBmp,
            aSourceRect,
            aDestRightTop );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 25",
                               countPixel( rDevice, aCol ) == 100-25);

        rDevice->clear(aCol);
        rDevice->drawMaskedColor(
            aCol2,
            rBmp,
            aSourceRect,
            aDestLeftBottom );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 25(b)",
                               countPixel( rDevice, aCol ) == 100-25);

        rDevice->clear(aCol);
        rDevice->drawMaskedColor(
            aCol2,
            rBmp,
            aSourceRect,
            aDestRightBottom );
        CPPUNIT_ASSERT_MESSAGE("number of rendered pixel is not 25(c)",
                               countPixel( rDevice, aCol ) == 100-25);
    }

public:
    void setUp()
    {
        const basegfx::B2ISize aSize(10,10);
        mpDevice1bpp = createBitmapDevice( aSize,
                                           true,
                                           Format::ONE_BIT_MSB_PAL );
        mpDevice32bpp = createBitmapDevice( aSize,
                                            true,
                                            Format::THIRTYTWO_BIT_TC_MASK );

        mpMask = createBitmapDevice( aSize,
                                     true,
                                     Format::EIGHT_BIT_GREY );

        ::rtl::OUString aSvg( RTL_CONSTASCII_USTRINGPARAM( "m 0 0h5v10h5v-5h-10z" ));

        basegfx::B2DPolyPolygon aPoly;
        basegfx::tools::importFromSvgD( aPoly, aSvg );
        const Color aCol(0xFF);
        mpMask->fillPolyPolygon(
            aPoly,
            aCol,
            DrawMode_PAINT );
    }

    void testMaskBasics()
    {
        implTestMaskBasics( mpDevice32bpp, mpMask );
        implTestMaskBasics( mpDevice1bpp, mpMask );
    }

    void testMaskClip()
    {
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(MaskTest);
    CPPUNIT_TEST(testMaskBasics);
    CPPUNIT_TEST(testMaskClip);
    CPPUNIT_TEST_SUITE_END();
};

CPPUNIT_TEST_SUITE_REGISTRATION(MaskTest);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
