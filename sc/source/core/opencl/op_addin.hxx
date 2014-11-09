/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_SC_SOURCE_CORE_OPENCL_OP_ADDIN_HXX
#define INCLUDED_SC_SOURCE_CORE_OPENCL_OP_ADDIN_HXX

#include "opbase.hxx"

namespace sc { namespace opencl {

class OpBesselj: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Besselj"; }
};
class OpGestep: public Normal
{
public:
    virtual void GenSlidingWindowFunction(std::stringstream &ss,
        const std::string &sSymName, SubArguments &vSubArguments) SAL_OVERRIDE;;
    virtual std::string BinFuncName(void) const SAL_OVERRIDE { return "Gestep"; }
};

}}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
