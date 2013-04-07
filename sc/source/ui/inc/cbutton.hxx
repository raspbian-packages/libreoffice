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

//------------------------------------------------------------------

#ifndef SC_CBUTTON_HXX
#define SC_CBUTTON_HXX

#include <tools/gen.hxx>
#include <tools/color.hxx>

class OutputDevice;


//==================================================================

class ScDDComboBoxButton
{
public:
            ScDDComboBoxButton( OutputDevice* pOutputDevice );
            ~ScDDComboBoxButton();

    void    SetOutputDevice( OutputDevice* pOutputDevice );

    void    Draw( const Point&  rAt,
                  const Size&   rSize,
                  sal_Bool          bState,
                  sal_Bool          bBtnIn = false );

    void    Draw( const Point&  rAt,
                  sal_Bool          bState,
                  sal_Bool          bBtnIn = false )
                { Draw( rAt, aBtnSize, bState, bBtnIn ); }

    void    Draw( sal_Bool          bState,
                  sal_Bool          bBtnIn = false )
                { Draw( aBtnPos, aBtnSize, bState, bBtnIn ); }

    void    SetOptSizePixel();

    void    SetPosPixel( const Point& rNewPos )  { aBtnPos = rNewPos; }
    Point   GetPosPixel() const                  { return aBtnPos; }

    void    SetSizePixel( const Size& rNewSize ) { aBtnSize = rNewSize; }
    Size    GetSizePixel() const                 { return aBtnSize; }

private:
    void    ImpDrawArrow( const Rectangle&  rRect,
                          sal_Bool              bState );

protected:
    OutputDevice* pOut;
    Point   aBtnPos;
    Size    aBtnSize;
};


#endif // SC_CBUTTON_HXX


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
