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

#ifndef _BGFX_RANGE_B2IBOX_HXX
#define _BGFX_RANGE_B2IBOX_HXX

#include <basegfx/point/b2ipoint.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <basegfx/tuple/b2ituple.hxx>
#include <basegfx/tuple/b2i64tuple.hxx>
#include <basegfx/range/basicbox.hxx>
#include <vector>
#include <basegfx/basegfxdllapi.h>


namespace basegfx
{
    /** A two-dimensional interval over integers

        This is most easily depicted as a set of integers, bounded by
        a lower and an upper value - but excluding the upper
        value. All inbetween values are included in the set (see also
        http://en.wikipedia.org/wiki/Interval_%28mathematics%29).

        The set is half-open, i.e. the lower bound is included, the
        upper bound not (if you're used to the notation - we're
        talking about [a,b) here, compared to closed [a,b] or fully
        open intervals (a,b)).

        If you don't need a half-open interval, check B2IRange.

        That means, isInside(val) will return true also for values of
        val=a, but not for val=b.

        Alternatively, consider this a rectangle, where the rightmost
        pixel column and the bottommost pixel row are excluded - this
        is much like polygon filling. As a result, filling a given
        rectangle with basebmp::BitmapDevice::fillPolyPolygon(), will
        affect exactly the same set of pixel as isInside() would
        return true for.

        @see B2IRange
     */
    class B2IBox
    {
    public:
        typedef sal_Int32       ValueType;
        typedef Int32Traits     TraitsType;

        B2IBox() {}

        /// Create degenerate interval that's still empty
        explicit B2IBox(const B2ITuple& rTuple)
        :   maRangeX(rTuple.getX()),
            maRangeY(rTuple.getY())
        {
        }

        /// Create proper interval between the two given points
        B2IBox(sal_Int32 x1,
               sal_Int32 y1,
               sal_Int32 x2,
               sal_Int32 y2) :
            maRangeX(x1),
            maRangeY(y1)
        {
            maRangeX.expand(x2);
            maRangeY.expand(y2);
        }

        /// Create proper interval between the two given points
        B2IBox(const B2ITuple& rTuple1,
               const B2ITuple& rTuple2) :
            maRangeX(rTuple1.getX()),
            maRangeY(rTuple1.getY())
        {
            expand( rTuple2 );
        }

        /** Check if the interval set is empty

            @return false, if no value is in this set - having a
            single value included will still return false.
         */
        bool isEmpty() const
        {
            return maRangeX.isEmpty() || maRangeY.isEmpty();
        }

        /// reset the object to empty state again, clearing all values
        void reset()
        {
            maRangeX.reset();
            maRangeY.reset();
        }

        bool operator==( const B2IBox& rBox ) const
        {
            return (maRangeX == rBox.maRangeX
                && maRangeY == rBox.maRangeY);
        }

        bool operator!=( const B2IBox& rBox ) const
        {
            return (maRangeX != rBox.maRangeX
                || maRangeY != rBox.maRangeY);
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMinX() const
        {
            return maRangeX.getMinimum();
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMinY() const
        {
            return maRangeY.getMinimum();
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMaxX() const
        {
            return maRangeX.getMaximum();
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        sal_Int32 getMaxY() const
        {
            return maRangeY.getMaximum();
        }

        /// return difference between upper and lower X value. returns 0 for empty sets.
        sal_Int64 getWidth() const
        {
            return maRangeX.getRange();
        }

        /// return difference between upper and lower Y value. returns 0 for empty sets.
        sal_Int64 getHeight() const
        {
            return maRangeY.getRange();
        }

        /// get lower bound of the set. returns arbitrary values for empty sets.
        B2IPoint getMinimum() const
        {
            return B2IPoint(
                maRangeX.getMinimum(),
                maRangeY.getMinimum()
                );
        }

        /// get upper bound of the set. returns arbitrary values for empty sets.
        B2IPoint getMaximum() const
        {
            return B2IPoint(
                maRangeX.getMaximum(),
                maRangeY.getMaximum()
                );
        }

        /// return difference between upper and lower value. returns (0,0) for empty sets.
        B2I64Tuple getRange() const
        {
            return B2I64Tuple(
                maRangeX.getRange(),
                maRangeY.getRange()
                );
        }

        /// return center point of set. returns (0,0) for empty sets.
        B2DPoint getCenter() const
        {
            return B2DPoint(
                maRangeX.getCenter(),
                maRangeY.getCenter()
                );
        }

        /// yields true if point is contained in set
        bool isInside(const B2ITuple& rTuple) const
        {
            return (
                maRangeX.isInside(rTuple.getX())
                && maRangeY.isInside(rTuple.getY())
                );
        }

        /// yields true if rBox is inside, or equal to set
        bool isInside(const B2IBox& rBox) const
        {
            return (
                maRangeX.isInside(rBox.maRangeX)
                && maRangeY.isInside(rBox.maRangeY)
                );
        }

        /// yields true if rBox at least partly inside set
        bool overlaps(const B2IBox& rBox) const
        {
            return (
                maRangeX.overlaps(rBox.maRangeX)
                && maRangeY.overlaps(rBox.maRangeY)
                );
        }

        /// add point to the set, expanding as necessary
        void expand(const B2ITuple& rTuple)
        {
            maRangeX.expand(rTuple.getX());
            maRangeY.expand(rTuple.getY());
        }

        /// add rBox to the set, expanding as necessary
        void expand(const B2IBox& rBox)
        {
            maRangeX.expand(rBox.maRangeX);
            maRangeY.expand(rBox.maRangeY);
        }

        /// calc set intersection
        void intersect(const B2IBox& rBox)
        {
            maRangeX.intersect(rBox.maRangeX);
            maRangeY.intersect(rBox.maRangeY);
        }

        /// grow set by nValue on all sides
        void grow(sal_Int32 nValue)
        {
            maRangeX.grow(nValue);
            maRangeY.grow(nValue);
        }

    private:
        BasicBox        maRangeX;
        BasicBox        maRangeY;
    };

    /** Compute the set difference of the two given boxes

        This method calculates the symmetric difference (aka XOR)
        between the two given boxes, and returning the resulting
        boxes. Thus, the result will contain all areas where one, but
        not both boxes lie.

        @param o_rResult
        Result vector. The up to four difference boxes are returned
        within this vector

        @param rFirst
        The first box

        @param rSecond
        The second box

        @return the input vector
     */
     BASEGFX_DLLPUBLIC ::std::vector< B2IBox >& computeSetDifference( ::std::vector< B2IBox >& o_rResult,
                                                                      const B2IBox&            rFirst,
                                                                      const B2IBox&            rSecond );

} // end of namespace basegfx

#endif /* _BGFX_RANGE_B2IBOX_HXX */

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
