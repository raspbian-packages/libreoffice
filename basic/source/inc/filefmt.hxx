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

#ifndef _SB_FILEFMT_HXX
#define _SB_FILEFMT_HXX

#include <tools/solar.h>

class SvStream;

// Version  2: data type of the return value for publics
// Version  3: new opcodes
// Version  4: new opcodes
// Version  5: bug (entry of STATIC-variables in the init code)
// Version  6: new opcodes and bug (construct globals, without ending the BASIC)
// Version  7: correction concerning the WITH-Parsing
// Version  8: correction concerning the IF-Parsing
// Version  9: end init code with LEAVE, too, if no SUB/FUNCTION follows
// Version  A: #36374 at DIM AS NEW... construct variable too
// Version  B: #40689 reorganized static
// Version  C: #41606 bug at static
// Version  D: #42678 bug at RTL-function spc
// Version  E: #56204 DCREATE, to also construct arrays at DIM AS NEW
// Version  F: #57844 introduction of SvNumberformat::StringToDouble
// Version 10: #29955 generate for-loop-level in Statement-PCodes
// Version 11: #29955 force anew compilation because of build-inconsistences

#define B_LEGACYVERSION 0x00000011L
#define B_CURVERSION 0x00000012L
#define B_EXT_IMG_VERSION 0x00000012L

// The file contains either a module- or a library-record.
// Those records contain further records. Every record's got
// the following header:

//  sal_uInt16 identifier
//  sal_uInt32 the record's length without the header
//  sal_uInt16 number of sub-elements

// all the file-offsets in records are relative to the module's start!

#define B_LIBRARY       0x4C42      // BL Library Record
#define B_MODULE        0x4D42      // BM Module Record
#define B_NAME          0x4E4D      // MN module name
#define B_COMMENT       0x434D      // MC comment
#define B_SOURCE        0x4353      // SC source code
#define B_PCODE         0x4350      // PC p-code
#define B_OLDPUBLICS    0x7550      // Pu publics
#define B_PUBLICS       0x5550      // PU publics
#define B_POOLDIR       0x4450      // PD symbol pool directory
#define B_SYMPOOL       0x5953      // SY symbol pool
#define B_STRINGPOOL    0x5453      // ST symbol pool
#define B_LINERANGES    0x524C      // LR line ranges for publics
#define B_MODEND        0x454D      // ME module end
#define B_SBXOBJECTS    0x5853      // SX SBX objects

#define EXTENDED_BINARY_MODULES
#ifdef  EXTENDED_BINARY_MODULES
#define B_EXTSOURCE     0x5345      // ES extended source
#endif

// A library record contains only module records
//  sal_uInt16 identifier BL
//  sal_uInt32 the record's length
//  sal_uInt16 number of modules

// A module-record contains all the other record types
//  sal_uInt16 identifier BM
//  sal_uInt32 the record's length
//  sal_uInt16 1
// Data:
//  sal_uInt32 version number
//  sal_uInt32 character set
//  sal_uInt32 starting address initialisation code
//  sal_uInt32 starting address sub main
//  sal_uInt32 reserved
//  sal_uInt32 reserved

// module name, comment and source code:
//  sal_uInt16 identifier MN, MC or SC
//  sal_uInt32 the record's length
//  sal_uInt16 1
// Data:
//  string instance

// P-Code:
//  sal_uInt16 identifier PC
//  sal_uInt32 the record's length
//  sal_uInt16 1
// Data:
//  the P-Code as bytesack

// All symbols and strings are kept in a string-pool.
// References to these strings are in this pool in the form of an index.

// List of all publics:
//  sal_uInt16 identifier PU or Pu
//  sal_uInt32 the record's length
//  sal_uInt16 number of publics
// Data for every public-entry:
//  sal_uInt16 string index
//  sal_uInt32 starting address in the p-code-image (sal_uInt16 for old publics)
//  sal_uInt16 data type of the return value (from version 2)

// Register of the symbol tables:
//  sal_uInt16 identifier SP
//  sal_uInt32 the record's length
//  sal_uInt16 number of symbol tables
// Data for every symbol table:
//  sal_uInt16 stringindex of the name
//  sal_uInt16 number of symbols
//  sal_uInt16 scope identifier

// symbol table:
//  sal_uInt16 identifier SY
//  sal_uInt32 the record's length
//  sal_uInt16 number of symbols
// Data:
//  sal_uInt16 stringindex of the name
//  sal_uInt16 number of symbols
// Data for every symbol:
//  sal_uInt16 stringindex of the name
//  sal_uInt16 data type
//  sal_uInt16 length for STRING*n-symbols (0x8000: STATIC variable)

// Stringpool:
//  sal_uInt16 identifier ST
//  sal_uInt32 the record's length
//  sal_uInt16 number of strings
// Data for every string:
//  sal_uInt32 Offset in the block of all strings
// the block of all strings (ASCIIZ) follows then

// line ranges:
//  sal_uInt16 identifier LR
//  sal_uInt32 the record's length
//  sal_uInt16 number of strings
// Data for every public:
//  sal_uInt16 1. Zeile (Sub XXX)
//  sal_uInt16 2. Zeile (End Sub)

// SBX-objects:
// sal_uInt16 number of objects
// ....   object data


// service routines (in IMAGE.CXX)

sal_Bool  SbGood( SvStream& r );
sal_uIntPtr SbOpenRecord( SvStream&, sal_uInt16 nSignature, sal_uInt16 nElem );
void  SbCloseRecord( SvStream&, sal_uIntPtr );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
