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


#include "aqua/salinst.h"
#include "aqua11ywrappergroup.h"

// Wrapper for AXGroup role

@implementation AquaA11yWrapperGroup : AquaA11yWrapper

-(id)titleUIElementAttribute {
    return self; // TODO
}

-(NSArray *)accessibilityAttributeNames {
    // Default Attributes
    NSMutableArray * attributeNames = [ NSMutableArray arrayWithArray: [ super accessibilityAttributeNames ] ];
    // Special Attributes and removing unwanted attributes depending on role
    [ attributeNames removeObjectsInArray: [ NSArray arrayWithObjects:
            NSAccessibilityTitleAttribute, 
            NSAccessibilityEnabledAttribute, 
            NSAccessibilitySelectedChildrenAttribute, 
            nil ]
    ];
    [ attributeNames addObject: NSAccessibilityContentsAttribute ];
    [ attributeNames addObject: NSAccessibilityTitleUIElementAttribute ];
    return attributeNames;
}

@end

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
