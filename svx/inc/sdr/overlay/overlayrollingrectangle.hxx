/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYROLLINGRECTANGLE_HXX
#define INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYROLLINGRECTANGLE_HXX

#include <svx/sdr/overlay/overlayobject.hxx>



namespace sdr
{
    namespace overlay
    {
        class OverlayRollingRectangleStriped : public OverlayObjectWithBasePosition
        {
        protected:
            // second position in pixel
            basegfx::B2DPoint                       maSecondPosition;

            // bitfield
            // Flag to switch on/off long lines to the OutputDevice bounds
            bool                                    mbExtendedLines : 1;

            // Flag to switch on/off the bounds itself
            bool                                    mbShowBounds : 1;

            // geometry creation for OverlayObject
            virtual drawinglayer::primitive2d::Primitive2DSequence createOverlayObjectPrimitive2DSequence() SAL_OVERRIDE;

        public:
            OverlayRollingRectangleStriped(
                const basegfx::B2DPoint& rBasePos,
                const basegfx::B2DPoint& rSecondPos,
                bool bExtendedLines = false,
                bool bShowBounds = true);
            virtual ~OverlayRollingRectangleStriped();

            // change second position
            const basegfx::B2DPoint& getSecondPosition() const { return maSecondPosition; }
            void setSecondPosition(const basegfx::B2DPoint& rNew);

            // change extended lines
            bool getExtendedLines() const { return mbExtendedLines; }

            // change show bounds
            bool getShowBounds() const { return mbShowBounds; }

            // react on stripe definition change
            virtual void stripeDefinitionHasChanged() SAL_OVERRIDE;
        };
    } // end of namespace overlay
} // end of namespace sdr



#endif // INCLUDED_SVX_INC_SDR_OVERLAY_OVERLAYROLLINGRECTANGLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
