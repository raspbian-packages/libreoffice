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

#ifndef CSV_CSV_ENV_HXX
#define CSV_CSV_ENV_HXX



//*******       Include c-language-types        ************//
// size_t, wchar_t
#include <stdlib.h>



//*******       Builtin types of exact length        ************//

// Exact length builtin types
typedef signed char     INT8;
typedef unsigned char   UINT8;
typedef short           INT16;
typedef unsigned short  UINT16;
typedef long            INT32;
typedef unsigned long   UINT32;
typedef float           REAL32;
typedef double          REAL64;


// Additional builtin types
typedef INT32        intt;      // standard sized integer.
typedef UINT32       uintt;     // standard sized unsigned integer.
typedef REAL64       real;      // standard sized real.

//  Constants
//  ---------
// Zero-pointer for use in ellipsed (...) parameter lists which expect a
//   pointer which may have another size than an int.
//   Must be a define to be used in precompiled headers:
#define NIL   ((void*)0)
// char '\0'
#define NULCH '\0'



// Boolesche Operatoren
#define AND &&
#define OR  ||
#define NOT !

// Macro for distinguishing dynamic allocated pointers from
//   referencing pointers
#define DYN     // Exact specification: DYN has to be used if and only if:
                //  1. DYN specifies a class member pointer or reference variable and
                //     the class must free the referenced memory.
                //  2. DYN specifies a pointer or reference (return-) parameter of a function
                //     and for in-parameters the function or its class
                //     must free the referenced memory, the parameter is then called
                //     a let-parameter.
                //     For out- and inout-parameters
                //     or return values the caller of the function hast to
                //     free the referenced memory.
                //
                //     It is irrelevant who allocated the memory!
                //
                //     DYN - variables use the prefixes "dp" or "dr" instead of "p" or "r".


//******        Assertions          ******//

namespace csv
{
void                PerformAssertion(
                        const char *        condition,
                        const char *        file,
                        unsigned            line );
}

// Programming by contract
#ifndef CSV_NO_ASSERTIONS

#ifdef CSV_USE_CSV_ASSERTIONS
#define csv_assert(x)       ( (x) ? (void)(0) : ::csv::PerformAssertion( #x, __FILE__, __LINE__) )
#else

// Save NDEBUG state
#ifdef NDEBUG
#define CSV_CSV_ENV_HXX_HAD_NDEBUG
#undef NDEBUG
#endif

#if OSL_DEBUG_LEVEL == 0
#define NDEBUG
#endif
#include <assert.h>

#define csv_assert(x)       assert(x);

// Restore NDEBUG state
#ifdef CSV_CSV_ENV_HXX_HAD_NDEBUG
#define NDEBUG
#else
#undef NDEBUG
#endif

#endif

#else // #ifndef CSV_NO_ASSERTIONS else

#define csv_assert(x)

#endif  // end ifndef CSV_NO_ASSERTIONS else



/* Additional Programming Conventions

1. see above at "#define DYN"
2. function parameters get one of these prefixes:
    - i_     := Function uses only the value, but must not change a referenced variable.
    - o_     := Parameter is undefined until function has set it.
                Parametere must be set by the function.
    - io_    := Function may use and change the referenced variable.
    - pass_  := Funktion may use and change the referenced variable and HAS TO free the
                associated memory.
3. Global constants get the prefix 'C_', global variables the prefix 'G_'.
4. Static members end with an underscore '_'.

*/


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
