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

#include "sal/config.h"
#include "sal/precppunit.hxx"

// autogenerated file with codegen.pl

#include "cppunit/TestAssert.h"
#include "cppunit/TestFixture.h"
#include "cppunit/extensions/HelperMacros.h"

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/curve/b2dcubicbezier.hxx>
#include <basegfx/curve/b2dbeziertools.hxx>
#include <basegfx/range/b2dpolyrange.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygontools.hxx>
#include <basegfx/polygon/b2dpolypolygoncutter.hxx>
#include <basegfx/polygon/b2dpolygonclipper.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/numeric/ftools.hxx>

#include <boost/bind.hpp>

using namespace ::basegfx;


namespace basegfx2d
{

class genericclipper : public CppUnit::TestFixture
{
private:
    B2DPolygon aSelfIntersecting;
    B2DPolygon aShiftedRectangle;

public:
    // initialise your test code values here.
    void setUp()
    {
        aSelfIntersecting.append(B2DPoint(0,  0));
        aSelfIntersecting.append(B2DPoint(0,  100));
        aSelfIntersecting.append(B2DPoint(75, 100));
        aSelfIntersecting.append(B2DPoint(75, 50));
        aSelfIntersecting.append(B2DPoint(25, 50));
        aSelfIntersecting.append(B2DPoint(25, 150));
        aSelfIntersecting.append(B2DPoint(100,150));
        aSelfIntersecting.append(B2DPoint(100,0));
        aSelfIntersecting.setClosed(true);

        aShiftedRectangle = tools::createPolygonFromRect(
            B2DRange(0,90,20,150));
    }

    void tearDown()
    {}

    void validate(const char* pName,
                  const char* pValidSvgD,
                  B2DPolyPolygon (*pFunc)(const B2DPolyPolygon&, const B2DPolyPolygon&))
    {
        const B2DPolyPolygon aSelfIntersect(
            tools::prepareForPolygonOperation(aSelfIntersecting));
        const B2DPolyPolygon aRect(
            tools::prepareForPolygonOperation(aShiftedRectangle));
#if defined(VERBOSE)
        fprintf(stderr, "%s input LHS - svg:d=\"%s\"\n",
                pName, rtl::OUStringToOString(
                    basegfx::tools::exportToSvgD(
                        aSelfIntersect),
                    RTL_TEXTENCODING_UTF8).getStr() );
        fprintf(stderr, "%s input RHS - svg:d=\"%s\"\n",
                pName, rtl::OUStringToOString(
                    basegfx::tools::exportToSvgD(
                        aRect),
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        const B2DPolyPolygon aRes=
            pFunc(aSelfIntersect, aRect);

#if defined(VERBOSE)
        fprintf(stderr, "%s - svg:d=\"%s\"\n",
                pName, rtl::OUStringToOString(
                    basegfx::tools::exportToSvgD(aRes),
                    RTL_TEXTENCODING_UTF8).getStr() );
#endif

        rtl::OUString aValid=rtl::OUString::createFromAscii(pValidSvgD);

        CPPUNIT_ASSERT_MESSAGE(pName,
                               basegfx::tools::exportToSvgD(aRes) == aValid);
    }

    void validateOr()
    {
        const char* pValid="m0 0h100v150h-75v-50h-5v50h-20v-50-10zm75 100v-50h-50v50z"; 
        validate("validateOr", pValid, &tools::solvePolygonOperationOr);
    }

    void validateXor()
    {
        const char* pValid="m0 0h100v150h-75v-50h-5v50h-20v-50-10zm0 100h20v-10h-20zm75 0v-50h-50v50z";
        validate("validateXor", pValid, &tools::solvePolygonOperationXor);
    }

    void validateAnd()
    {
        const char* pValid="m0 100v-10h20v10z";
        validate("validateAnd", pValid, &tools::solvePolygonOperationAnd);
    }

    void validateDiff()
    {
        const char* pValid="m0 90v-90h100v150h-75v-50h-5v-10zm75 10v-50h-50v50z";
        validate("validateDiff", pValid, &tools::solvePolygonOperationDiff);
    }

    void checkCrossoverSolver()
    {
        B2DPolyPolygon aPoly;
        tools::importFromSvgD(
            aPoly,
            ::rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM(
                    "m0 0 v 5  h 3 h 1 h 1 h 1 v -2 v -3 z"
                    "m3 7 v -2 h 1 h 1 h 1 v -2 h 1 v 3 z")));

        tools::solveCrossovers(aPoly);
    }

    // Change the following lines only, if you add, remove or rename
    // member functions of the current class,
    // because these macros are need by auto register mechanism.

    CPPUNIT_TEST_SUITE(genericclipper);
    CPPUNIT_TEST(validateOr);
    CPPUNIT_TEST(validateXor);
    CPPUNIT_TEST(validateAnd);
    CPPUNIT_TEST(validateDiff);
    CPPUNIT_TEST(checkCrossoverSolver);
    CPPUNIT_TEST_SUITE_END();
};

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_REGISTRATION(basegfx2d::genericclipper);
} // namespace basegfx2d

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
