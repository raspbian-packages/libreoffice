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

#ifndef ADC_TKP2_HXX
#define ADC_TKP2_HXX

// USED SERVICES
    // BASE CLASSES
    // COMPONENTS
class CharacterSource;
class TkpContext;
    // PARAMETRS



/** This is the interface for parser classes, which get a sequence of Token s from
    a text.

    Start() starts to parse the text from the given i_rSource.
    GetNextToken() returns a Token on the heap as long as there are
    still characters in the text left. The last time GetNextToken()
    returns NULL.

    The algorithms for parsing tokens from the text are an issue of
    the derived classes.
*/
class TokenParse2
{
  public:
    // LIFECYCLE
                        TokenParse2();
    virtual             ~TokenParse2() {}

    // OPERATIONS
    virtual void        Start(
                            CharacterSource &
                                            i_rSource );

    /** @short  Gets the next identifiable token out of the
        source code.
        @return true, if there was passed a valid token.
                false, if the parsed stream is finished or
                       an error occurred.
    */
    bool                GetNextToken();

  private:
    virtual void        SetStartContext() = 0;
    virtual void        SetCurrentContext(
                            TkpContext &        io_rContext ) = 0;
    virtual TkpContext &
                        CurrentContext() = 0;
    // DATA
    CharacterSource *   pChars;
};


#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
