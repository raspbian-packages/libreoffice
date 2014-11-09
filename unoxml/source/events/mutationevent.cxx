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

#include <mutationevent.hxx>

namespace DOM { namespace events
{
    CMutationEvent::CMutationEvent()
        : CMutationEvent_Base()
        , m_attrChangeType(AttrChangeType_MODIFICATION)
    {
    }

    CMutationEvent::~CMutationEvent()
    {
    }

    Reference< XNode > SAL_CALL CMutationEvent::getRelatedNode() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_relatedNode;
    }

    OUString SAL_CALL CMutationEvent::getPrevValue() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_prevValue;
    }

    OUString SAL_CALL CMutationEvent::getNewValue() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_newValue;
    }

    OUString SAL_CALL CMutationEvent::getAttrName() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_attrName;
    }

    AttrChangeType SAL_CALL CMutationEvent::getAttrChange() throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);
        return m_attrChangeType;
    }

    void SAL_CALL CMutationEvent::initMutationEvent(const OUString& typeArg,
        sal_Bool canBubbleArg, sal_Bool cancelableArg,
        const Reference< XNode >& relatedNodeArg, const OUString& prevValueArg,
        const OUString& newValueArg, const OUString& attrNameArg,
        AttrChangeType attrChangeArg) throw (RuntimeException, std::exception)
    {
        ::osl::MutexGuard const g(m_Mutex);

        CEvent::initEvent(typeArg, canBubbleArg, cancelableArg);
        m_relatedNode = relatedNodeArg;
        m_prevValue = prevValueArg;
        m_newValue = newValueArg;
        m_attrName = attrNameArg;
        m_attrChangeType = attrChangeArg;
    }

    // delegate to CEvent, since we are inheriting from CEvent and XEvent
    OUString SAL_CALL CMutationEvent::getType() throw (RuntimeException, std::exception)
    {
        return CEvent::getType();
    }

    Reference< XEventTarget > SAL_CALL CMutationEvent::getTarget() throw (RuntimeException, std::exception)
    {
        return CEvent::getTarget();
    }

    Reference< XEventTarget > SAL_CALL CMutationEvent::getCurrentTarget() throw (RuntimeException, std::exception)
    {
        return CEvent::getCurrentTarget();
    }

    PhaseType SAL_CALL CMutationEvent::getEventPhase() throw (RuntimeException, std::exception)
    {
        return CEvent::getEventPhase();
    }

    sal_Bool SAL_CALL CMutationEvent::getBubbles() throw (RuntimeException, std::exception)
    {
        return CEvent::getBubbles();
    }

    sal_Bool SAL_CALL CMutationEvent::getCancelable() throw (RuntimeException, std::exception)
    {
        return CEvent::getCancelable();
    }

    com::sun::star::util::Time SAL_CALL CMutationEvent::getTimeStamp() throw (RuntimeException, std::exception)
    {
        return CEvent::getTimeStamp();
    }

    void SAL_CALL CMutationEvent::stopPropagation() throw (RuntimeException, std::exception)
    {
        CEvent::stopPropagation();
    }
    void SAL_CALL CMutationEvent::preventDefault() throw (RuntimeException, std::exception)
    {
        CEvent::preventDefault();
    }

    void SAL_CALL CMutationEvent::initEvent(const OUString& eventTypeArg, sal_Bool canBubbleArg,
        sal_Bool cancelableArg) throw (RuntimeException, std::exception)
    {
        // base initializer
        CEvent::initEvent(eventTypeArg, canBubbleArg, cancelableArg);
    }
}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
