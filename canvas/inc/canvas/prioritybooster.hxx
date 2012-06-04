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

#ifndef INCLUDED_CANVAS_PRIORITYBOOSTER_HXX
#define INCLUDED_CANVAS_PRIORITYBOOSTER_HXX

#include <sal/types.h>

#include <memory>
#include <canvas/canvastoolsdllapi.h>


namespace canvas
{
    namespace tools
    {
        struct PriorityBooster_Impl;

        /** Simplistic RAII object, to temporarily boost execution
            priority for the current scope.
         */
        class PriorityBooster
        {
        public:
            /** Create booster, with given priority delta

                @param nDelta
                Difference in execution priority. Positive values
                increase prio, negative values decrease prio.
             */
             CANVASTOOLS_DLLPUBLIC explicit PriorityBooster( sal_Int32 nDelta );
             CANVASTOOLS_DLLPUBLIC ~PriorityBooster();

        private:
            // also disables copy constructor and assignment operator
            const ::std::auto_ptr< PriorityBooster_Impl > mpImpl;
        };
    }
}

#endif /* INCLUDED_CANVAS_PRIORITYBOOSTER_HXX */
// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
