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

#ifndef OOX_POWERPOINT_BACKGROUNDPROPERTIES_HXX
#define OOX_POWERPOINT_BACKGROUNDPROPERTIES_HXX

#include "oox/core/fragmenthandler2.hxx"
#include "oox/drawingml/fillproperties.hxx"

namespace oox { namespace ppt {

// ---------------------------------------------------------------------

class BackgroundPropertiesContext : public ::oox::core::FragmentHandler2
{
public:
    BackgroundPropertiesContext( ::oox::core::FragmentHandler2& rParent, ::oox::drawingml::FillProperties& rFillProperties );
    virtual ::oox::core::ContextHandlerRef onCreateContext( sal_Int32 aElementToken, const AttributeList& rAttribs );

protected:
    ::oox::drawingml::FillProperties& mrFillProperties;
};

} }

#endif // OOX_POWERPOINT_BACKGROUNDPROPERTIES_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
