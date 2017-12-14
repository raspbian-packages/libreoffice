/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <documentlinkmgr.hxx>
#include <ddelink.hxx>
#include <sc.hrc>
#include <scresid.hxx>

#include <svx/svdoole2.hxx>

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <vcl/msgbox.hxx>

using namespace rtl;

namespace sc {

struct DocumentLinkManagerImpl : boost::noncopyable
{
    SfxObjectShell* mpShell;
    std::unique_ptr<sfx2::LinkManager> mpLinkManager;

    DocumentLinkManagerImpl() {}
};

DocumentLinkManager::DocumentLinkManager() : mpImpl(new DocumentLinkManagerImpl) {}

DocumentLinkManager::~DocumentLinkManager()
{
    delete mpImpl;
}

sfx2::LinkManager* DocumentLinkManager::getLinkManager( bool bCreate )
{
    if (!mpImpl->mpLinkManager && bCreate && mpImpl->mpShell)
        mpImpl->mpLinkManager.reset(new sfx2::LinkManager(mpImpl->mpShell));
    return mpImpl->mpLinkManager.get();
}

const sfx2::LinkManager* DocumentLinkManager::getExistingLinkManager() const
{
    return mpImpl->mpLinkManager.get();
}

bool DocumentLinkManager::idleCheckLinks()
{
    if (!mpImpl->mpLinkManager)
        return false;

    bool bAnyLeft = false;
    const sfx2::SvBaseLinks& rLinks = mpImpl->mpLinkManager->GetLinks();
    for (size_t i = 0, n = rLinks.Count(); i < n; ++i)
    {
        sfx2::SvBaseLink* pBase = *rLinks[i];
        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (!pDdeLink || !pDdeLink->NeedsUpdate())
            continue;

        pDdeLink->TryUpdate();
        if (pDdeLink->NeedsUpdate()) // Was not successful?
            bAnyLeft = true;
    }

    return bAnyLeft;
}

bool DocumentLinkManager::hasDdeLinks() const
{
    return hasDdeOrOleLinks(true, false);
}

bool DocumentLinkManager::hasDdeOrOleLinks() const
{
    return hasDdeOrOleLinks(true, true);
}

bool DocumentLinkManager::hasDdeOrOleLinks(bool bDde, bool bOle) const
{
    if (!mpImpl->mpLinkManager)
        return false;

    const sfx2::SvBaseLinks& rLinks = mpImpl->mpLinkManager->GetLinks();
    for (size_t i = 0, n = rLinks.Count(); i < n; ++i)
    {
        sfx2::SvBaseLink* pBase = *rLinks[i];
        if (bDde && dynamic_cast<ScDdeLink*>(pBase))
            return true;
        if (bOle && dynamic_cast<SdrEmbedObjectLink*>(pBase))
            return true;
    }

    return false;
}

bool DocumentLinkManager::updateDdeOrOleLinks( Window* pWin )
{
    if (!mpImpl->mpLinkManager)
        return false;

    sfx2::LinkManager* pMgr = mpImpl->mpLinkManager.get();
    const sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();

    // If the update takes longer, reset all values so that nothing
    // old (wrong) is left behind
    bool bAny = false;
    for (size_t i = 0, n = rLinks.Count(); i < n; ++i)
    {
        sfx2::SvBaseLink* pBase = *rLinks[i];

        SdrEmbedObjectLink* pOleLink = dynamic_cast<SdrEmbedObjectLink*>(pBase);
        if (pOleLink)
        {
            pOleLink->Update();
            continue;
        }

        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (!pDdeLink)
            continue;

        if (pDdeLink->Update())
            bAny = true;
        else
        {
            // Update failed.  Notify the user.
            OUString aFile = pDdeLink->GetTopic();
            OUString aElem = pDdeLink->GetItem();
            OUString aType = pDdeLink->GetAppl();

            OUStringBuffer aBuf;
            aBuf.append(OUString( ResId::toString( ScResId(SCSTR_DDEDOC_NOT_LOADED))));
            aBuf.append(OUString::createFromAscii("\n\n"));
            aBuf.append(OUString::createFromAscii("Source : "));
            aBuf.append(aFile);
            aBuf.append(OUString::createFromAscii("\nElement : "));
            aBuf.append(aElem);
            aBuf.append(OUString::createFromAscii("\nType : "));
            aBuf.append(aType);
            ErrorBox aBox(pWin, WB_OK | RET_OK, aBuf.makeStringAndClear());
            aBox.Execute();
        }
    }

    pMgr->CloseCachedComps();

    return bAny;
}

bool DocumentLinkManager::updateDdeLink( const rtl::OUString& rAppl, const rtl::OUString& rTopic, const rtl::OUString& rItem )
{
    if (!mpImpl->mpLinkManager)
        return false;

    sfx2::LinkManager* pMgr = mpImpl->mpLinkManager.get();
    const sfx2::SvBaseLinks& rLinks = pMgr->GetLinks();

    bool bFound = false;
    for (size_t i = 0, n = rLinks.Count(); i < n; ++i)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (!pDdeLink)
            continue;

        if ( OUString(pDdeLink->GetAppl()) == rAppl &&
             OUString(pDdeLink->GetTopic()) == rTopic &&
             OUString(pDdeLink->GetItem()) == rItem )
        {
            pDdeLink->TryUpdate();
            bFound = true; // Could be multiple (Mode), so continue searching
        }
    }

    return bFound;
}

size_t DocumentLinkManager::getDdeLinkCount() const
{
    if (!mpImpl->mpLinkManager)
        return 0;

    size_t nDdeCount = 0;
    const sfx2::SvBaseLinks& rLinks = mpImpl->mpLinkManager->GetLinks();
    for (size_t i = 0, n = rLinks.Count(); i < n; ++i)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (!pDdeLink)
            continue;

        ++nDdeCount;
    }

    return nDdeCount;
}

void DocumentLinkManager::disconnectDdeLinks()
{
    if (!mpImpl->mpLinkManager)
        return;

    const sfx2::SvBaseLinks& rLinks = mpImpl->mpLinkManager->GetLinks();
    for (size_t i = 0, n = rLinks.Count(); i < n; ++i)
    {
        ::sfx2::SvBaseLink* pBase = *rLinks[i];
        ScDdeLink* pDdeLink = dynamic_cast<ScDdeLink*>(pBase);
        if (pDdeLink)
            pDdeLink->Disconnect();
    }
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
