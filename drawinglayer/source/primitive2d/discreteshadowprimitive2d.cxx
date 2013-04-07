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

#include <drawinglayer/primitive2d/discreteshadowprimitive2d.hxx>
#include <drawinglayer/primitive2d/drawinglayer_primitivetypes2d.hxx>
#include <drawinglayer/primitive2d/bitmapprimitive2d.hxx>
#include <basegfx/matrix/b2dhommatrixtools.hxx>
#include <drawinglayer/primitive2d/transformprimitive2d.hxx>
#include <drawinglayer/geometry/viewinformation2d.hxx>

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        DiscreteShadow::DiscreteShadow(const BitmapEx& rBitmapEx)
        :   maBitmapEx(rBitmapEx),
            maTopLeft(),
            maTop(),
            maTopRight(),
            maRight(),
            maBottomRight(),
            maBottom(),
            maBottomLeft(),
            maLeft()
        {
            const Size& rBitmapSize = getBitmapEx().GetSizePixel();

            if(rBitmapSize.Width() != rBitmapSize.Height() || rBitmapSize.Width() < 7)
            {
                OSL_ENSURE(false, "DiscreteShadowPrimitive2D: wrong bitmap format (!)");
                maBitmapEx = BitmapEx();
            }
        }

        const BitmapEx& DiscreteShadow::getTopLeft() const
        {
            if(maTopLeft.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maTopLeft = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maTopLeft.Crop(
                    Rectangle(Point(0,0),Size(nQuarter*2+1,nQuarter*2+1)));
            }

            return maTopLeft;
        }

        const BitmapEx& DiscreteShadow::getTop() const
        {
            if(maTop.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maTop = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maTop.Crop(
                    Rectangle(Point(nQuarter*2+1,0),Size(1,nQuarter+1)));
            }

            return maTop;
        }

        const BitmapEx& DiscreteShadow::getTopRight() const
        {
            if(maTopRight.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maTopRight = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maTopRight.Crop(
                    Rectangle(Point(nQuarter*2+2,0),Size(nQuarter*2+1,nQuarter*2+1)));
            }

            return maTopRight;
        }

        const BitmapEx& DiscreteShadow::getRight() const
        {
            if(maRight.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maRight = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maRight.Crop(
                    Rectangle(Point(nQuarter*3+2,nQuarter*2+1),Size(nQuarter+1,1)));
            }

            return maRight;
        }

        const BitmapEx& DiscreteShadow::getBottomRight() const
        {
            if(maBottomRight.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maBottomRight = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maBottomRight.Crop(
                    Rectangle(Point(nQuarter*2+2,nQuarter*2+2),Size(nQuarter*2+1,nQuarter*2+1)));
            }

            return maBottomRight;
        }

        const BitmapEx& DiscreteShadow::getBottom() const
        {
            if(maBottom.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maBottom = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maBottom.Crop(
                    Rectangle(Point(nQuarter*2+1,nQuarter*3+2),Size(1,nQuarter+1)));
            }

            return maBottom;
        }

        const BitmapEx& DiscreteShadow::getBottomLeft() const
        {
            if(maBottomLeft.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maBottomLeft = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maBottomLeft.Crop(
                    Rectangle(Point(0,nQuarter*2+2),Size(nQuarter*2+1,nQuarter*2+1)));
            }

            return maBottomLeft;
        }

        const BitmapEx& DiscreteShadow::getLeft() const
        {
            if(maLeft.IsEmpty())
            {
                const sal_Int32 nQuarter((getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const_cast< DiscreteShadow* >(this)->maLeft = getBitmapEx();
                const_cast< DiscreteShadow* >(this)->maLeft.Crop(
                    Rectangle(Point(0,nQuarter*2+1),Size(nQuarter+1,1)));
            }

            return maLeft;
        }

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////

namespace drawinglayer
{
    namespace primitive2d
    {
        Primitive2DSequence DiscreteShadowPrimitive2D::create2DDecomposition(const geometry::ViewInformation2D& /*rViewInformation*/) const
        {
            Primitive2DSequence xRetval;

            if(!getDiscreteShadow().getBitmapEx().IsEmpty())
            {
                const sal_Int32 nQuarter((getDiscreteShadow().getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const basegfx::B2DVector aScale(getTransform() * basegfx::B2DVector(1.0, 1.0));
                const double fSingleX(getDiscreteUnit() / aScale.getX());
                const double fSingleY(getDiscreteUnit() / aScale.getY());
                const double fBorderX(fSingleX * nQuarter);
                const double fBorderY(fSingleY * nQuarter);
                const double fBigLenX((fBorderX * 2.0) + fSingleX);
                const double fBigLenY((fBorderY * 2.0) + fSingleY);

                xRetval.realloc(8);

                // TopLeft
                xRetval[0] = Primitive2DReference(
                    new BitmapPrimitive2D(
                        getDiscreteShadow().getTopLeft(),
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            fBigLenX,
                            fBigLenY,
                            -fBorderX,
                            -fBorderY)));

                // Top
                xRetval[1] = Primitive2DReference(
                    new BitmapPrimitive2D(
                        getDiscreteShadow().getTop(),
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            1.0 - (2.0 * fBorderX) - fSingleX,
                            fBorderY + fSingleY,
                            fBorderX + fSingleX,
                            -fBorderY)));

                // TopRight
                xRetval[2] = Primitive2DReference(
                    new BitmapPrimitive2D(
                        getDiscreteShadow().getTopRight(),
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            fBigLenX,
                            fBigLenY,
                            1.0 - fBorderX,
                            -fBorderY)));

                // Right
                xRetval[3] = Primitive2DReference(
                    new BitmapPrimitive2D(
                        getDiscreteShadow().getRight(),
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            fBorderX + fSingleX,
                            1.0 - (2.0 * fBorderY) - fSingleY,
                            1.0,
                            fBorderY + fSingleY)));

                // BottomRight
                xRetval[4] = Primitive2DReference(
                    new BitmapPrimitive2D(
                        getDiscreteShadow().getBottomRight(),
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            fBigLenX,
                            fBigLenY,
                            1.0 - fBorderX,
                            1.0 - fBorderY)));

                // Bottom
                xRetval[5] = Primitive2DReference(
                    new BitmapPrimitive2D(
                        getDiscreteShadow().getBottom(),
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            1.0 - (2.0 * fBorderX) - fSingleX,
                            fBorderY + fSingleY,
                            fBorderX + fSingleX,
                            1.0)));

                // BottomLeft
                xRetval[6] = Primitive2DReference(
                    new BitmapPrimitive2D(
                        getDiscreteShadow().getBottomLeft(),
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            fBigLenX,
                            fBigLenY,
                            -fBorderX,
                            1.0 - fBorderY)));

                // Left
                xRetval[7] = Primitive2DReference(
                    new BitmapPrimitive2D(
                        getDiscreteShadow().getLeft(),
                        basegfx::tools::createScaleTranslateB2DHomMatrix(
                            fBorderX + fSingleX,
                            1.0 - (2.0 * fBorderY) - fSingleY,
                            -fBorderX,
                            fBorderY + fSingleY)));

                // put all in object transformation to get to target positions
                const Primitive2DReference xTransformed(
                    new TransformPrimitive2D(
                        getTransform(),
                        xRetval));

                xRetval = Primitive2DSequence(&xTransformed, 1);
            }

            return xRetval;
        }

        DiscreteShadowPrimitive2D::DiscreteShadowPrimitive2D(
            const basegfx::B2DHomMatrix& rTransform,
            const DiscreteShadow& rDiscreteShadow)
        :   DiscreteMetricDependentPrimitive2D(),
            maTransform(rTransform),
            maDiscreteShadow(rDiscreteShadow)
        {
        }

        bool DiscreteShadowPrimitive2D::operator==(const BasePrimitive2D& rPrimitive) const
        {
            if(DiscreteMetricDependentPrimitive2D::operator==(rPrimitive))
            {
                const DiscreteShadowPrimitive2D& rCompare = (DiscreteShadowPrimitive2D&)rPrimitive;

                return (getTransform() == rCompare.getTransform()
                    && getDiscreteShadow() == rCompare.getDiscreteShadow());
            }

            return false;
        }

        basegfx::B2DRange DiscreteShadowPrimitive2D::getB2DRange(const geometry::ViewInformation2D& rViewInformation) const
        {
            if(getDiscreteShadow().getBitmapEx().IsEmpty())
            {
                // no graphics without valid bitmap definition
                return basegfx::B2DRange();
            }
            else
            {
                // prepare normal objectrange
                basegfx::B2DRange aRetval(0.0, 0.0, 1.0, 1.0);
                aRetval.transform(getTransform());

                // extract discrete shadow size and grow
                const basegfx::B2DVector aScale(rViewInformation.getViewTransformation() * basegfx::B2DVector(1.0, 1.0));
                const sal_Int32 nQuarter((getDiscreteShadow().getBitmapEx().GetSizePixel().Width() - 3) >> 2);
                const double fGrowX((1.0 / aScale.getX()) * nQuarter);
                const double fGrowY((1.0 / aScale.getY()) * nQuarter);
                aRetval.grow(std::max(fGrowX, fGrowY));

                return aRetval;
            }
        }

        // provide unique ID
        ImplPrimitrive2DIDBlock(DiscreteShadowPrimitive2D, PRIMITIVE2D_ID_DISCRETESHADOWPRIMITIVE2D)

    } // end of namespace primitive2d
} // end of namespace drawinglayer

//////////////////////////////////////////////////////////////////////////////
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
