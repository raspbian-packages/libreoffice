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

#ifndef INCLUDED_SDR_PRIMITIVE2D_SDRRECTANGLEPRIMITIVE2D_HXX
#define INCLUDED_SDR_PRIMITIVE2D_SDRRECTANGLEPRIMITIVE2D_HXX

#include <drawinglayer/primitive2d/baseprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <svx/sdr/attribute/sdrlinefillshadowtextattribute.hxx>
#include <svx/sdr/attribute/sdrlinefillshadowtextattribute.hxx>

//////////////////////////////////////////////////////////////////////////////
// predefines

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        class SdrRectanglePrimitive2D : public BufferedDecompositionPrimitive2D
        {
        private:
            basegfx::B2DHomMatrix                       maTransform;
            attribute::SdrLineFillShadowTextAttribute   maSdrLFSTAttribute;
            double                                      mfCornerRadiusX;    // [0.0..1.0] relative to 1/2 width
            double                                      mfCornerRadiusY;    // [0.0..1.0] relative to 1/2 height

            // bitfield
            // flag which decides if the HitArea should be the filled geometry
            bool                                        mbForceFillForHitTest : 1;

        protected:
            // local decomposition.
            virtual Primitive2DSequence create2DDecomposition(const geometry::ViewInformation2D& aViewInformation) const;

        public:
            SdrRectanglePrimitive2D(
                const basegfx::B2DHomMatrix& rTransform,
                const attribute::SdrLineFillShadowTextAttribute& rSdrLFSTAttribute,
                double fCornerRadiusX,
                double fCornerRadiusY,
                bool bForceFillForHitTest);

            // data access
            const basegfx::B2DHomMatrix& getTransform() const { return maTransform; }
            const attribute::SdrLineFillShadowTextAttribute& getSdrLFSTAttribute() const { return maSdrLFSTAttribute; }
            double getCornerRadiusX() const { return mfCornerRadiusX; }
            double getCornerRadiusY() const { return mfCornerRadiusY; }
            bool isCornerRadiusUsed() const { return (0.0 != mfCornerRadiusX || 0.0 != mfCornerRadiusY); }
            bool getForceFillForHitTest() const { return mbForceFillForHitTest; }

            // compare operator
            virtual bool operator==(const BasePrimitive2D& rPrimitive) const;

            // provide unique ID
            DeclPrimitrive2DIDBlock()
        };
    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

#endif //INCLUDED_SDR_PRIMITIVE2D_SDRRECTANGLEPRIMITIVE2D_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
