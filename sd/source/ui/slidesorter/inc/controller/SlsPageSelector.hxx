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

#ifndef SD_SLIDESORTER_PAGE_SELECTOR_HXX
#define SD_SLIDESORTER_PAGE_SELECTOR_HXX

#include "model/SlsSharedPageDescriptor.hxx"

#include <com/sun/star/drawing/XDrawPage.hpp>
#include <vector>
#include <memory>
#include <boost/noncopyable.hpp>


class SdPage;

namespace sd { namespace slidesorter {
class SlideSorter;
} }

namespace sd { namespace slidesorter { namespace model {
class SlideSorterModel;
} } }

namespace sd { namespace slidesorter { namespace view {
class SlideSorterView;
} } }

namespace sd { namespace slidesorter { namespace controller {

class SlideSorterController;


/** A sub-controller that handles page selection of the slide browser.
    Selecting a page does not make it the current page (of the main view)
    automatically as this would not be desired in a multi selection.  This
    has to be done explicitly by calling the
    CurrentSlideManager::SetCurrentSlide() method.

    Indices of pages relate allways to the number of all pages in the model
    (as returned by GetPageCount()) not just the selected pages.
*/
class PageSelector : private ::boost::noncopyable
{
public:
    PageSelector (SlideSorter& rSlideSorter);

    void SelectAllPages (void);
    void DeselectAllPages (void);

    /** Update the selection state of all page descriptors to be the same as
        that of the corresponding pages of the SdPage objects and issue
        redraw requests where necessary.
    */
    void GetCoreSelection (void);

    /** Update the selection state of the SdPage objects to be the same as
        that of the correspinding page descriptors.
    */
    void SetCoreSelection (void);

    /** Select the specified descriptor.  The selection state of the other
        descriptors is not affected.
    */
    void SelectPage (int nPageIndex);
    /** Select the descriptor that is associated with the given page.  The
        selection state of the other descriptors is not affected.
    */
    void SelectPage (const SdPage* pPage);
    /** Select the specified descriptor.  The selection state of the other
        descriptors is not affected.
    */
    void SelectPage (const model::SharedPageDescriptor& rpDescriptor);

    /** Return whether the specified page is selected.  This convenience
        method is a subsitute for
        SlideSorterModel::GetPageDescriptor(i)->IsSelected() is included
        here to make this class more self contained.
    */
    bool IsPageSelected (int nPageIndex);

    /** Deselect the descriptor that is associated with the given page.
        @param bUpdateCurrentPage
            When <TRUE/> then the current page is updated to the first slide
            of the remaining selection.
    */
    void DeselectPage (
        int nPageIndex,
        const bool bUpdateCurrentPage = true);
    void DeselectPage (
        const model::SharedPageDescriptor& rpDescriptor,
        const bool bUpdateCurrentPage = true);

    /** This convenience method returns the same number of pages that
        SlideSorterModel.GetPageCount() returns.  It is included here so
        that it is self contained for iterating over all pages to select or
        deselect them.
    */
    int GetPageCount (void) const;
    int GetSelectedPageCount (void) const;

    /** Return the anchor for a range selection.  This usually is the first
        selected page after all pages have been deselected.
        @return
            The returned anchor may be NULL.
    */
    model::SharedPageDescriptor GetSelectionAnchor (void) const;


    typedef ::std::vector<SdPage*> PageSelection;

    /** Return an object that describes the current selection.  The caller
        can use that object to later restore the selection.
        @return
            The object returned describes the selection via indices.  So
            even if pages are exchanged a later call to SetPageSelection()
            is valid.
    */
    ::boost::shared_ptr<PageSelection> GetPageSelection (void) const;

    /** Restore a page selection according to the given selection object.
        @param rSelection
            Typically obtained by calling GetPageSelection() this object
            is used to restore the selection.  If pages were exchanged since
            the last call to GetPageSelection() it is still valid to call
            this method with the selection.  When pages have been inserted
            or removed the result may be unexpected.
        @param bUpdateCurrentPage
            When <TRUE/> (the default value) then after setting the
            selection update the current page to the first page of the
            selection.
            When called from withing UpdateCurrentPage() then this flag is
            used to prevent a recursion loop.
    */
    void SetPageSelection (
        const ::boost::shared_ptr<PageSelection>& rSelection,
        const bool bUpdateCurrentPage = true);

    /** Call this method after the the model has changed to set the number
        of selected pages.
    */
    void CountSelectedPages (void);

    /** Use the UpdateLock whenever you do a complex selection, i.e. call
        more than one method in a row.  An active lock prevents intermediate
        changes of the current slide.
    */
    class UpdateLock
    {
    public:
        UpdateLock (SlideSorter& rSlideSorter);
        UpdateLock (PageSelector& rPageSelector);
        ~UpdateLock (void);
        void Release (void);
    private:
        PageSelector* mpSelector;
    };

    class BroadcastLock
    {
    public:
        BroadcastLock (SlideSorter& rSlideSorter);
        BroadcastLock (PageSelector& rPageSelector);
        ~BroadcastLock (void);
    private:
        PageSelector& mrSelector;
    };

private:
    model::SlideSorterModel& mrModel;
    SlideSorter& mrSlideSorter;
    SlideSorterController& mrController;
    int mnSelectedPageCount;
    int mnBroadcastDisableLevel;
    bool mbSelectionChangeBroadcastPending;
    model::SharedPageDescriptor mpMostRecentlySelectedPage;
    /// Anchor for a range selection.
    model::SharedPageDescriptor mpSelectionAnchor;
    model::SharedPageDescriptor mpCurrentPage;
    sal_Int32 mnUpdateLockCount;
    bool mbIsUpdateCurrentPagePending;

    /** Enable the broadcasting of selection change events.  This calls the
        SlideSorterController::SelectionHasChanged() method to do the actual
        work.  When EnableBroadcasting has been called as many times as
        DisableBroadcasting() was called before and the selection has been
        changed in the mean time, this change will be broadcasted.
    */
    void EnableBroadcasting (void);

    /** Disable the broadcasting of selection change events.  Subsequent
        changes of the selection will set a flag that triggers the sending
        of events when EnableBroadcasting() is called.
    */
    void DisableBroadcasting (void);

    void UpdateCurrentPage (const bool bUpdateOnlyWhenPending = false);

    void CheckConsistency (void) const;
};

} } } // end of namespace ::sd::slidesorter::controller

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
