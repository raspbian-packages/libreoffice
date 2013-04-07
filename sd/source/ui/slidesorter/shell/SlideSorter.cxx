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


#include "SlideSorter.hxx"

#include "SlideSorterChildWindow.hrc"
#include "SlideSorterViewShell.hxx"
#include "controller/SlideSorterController.hxx"
#include "controller/SlsScrollBarManager.hxx"
#include "controller/SlsProperties.hxx"
#include "controller/SlsAnimator.hxx"
#include "view/SlideSorterView.hxx"
#include "view/SlsTheme.hxx"
#include "model/SlideSorterModel.hxx"

#include "glob.hrc"
#include "DrawController.hxx"
#include "ViewShellBase.hxx"
#include "ViewShellManager.hxx"
#include "Window.hxx"

#include <vcl/scrbar.hxx>
#include <vcl/svapp.hxx>

#include <sfx2/dispatch.hxx>
#include "sdresid.hxx"

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star;


namespace sd { namespace slidesorter {

namespace {
class ContentWindow : public ::sd::Window
{
public:
    ContentWindow(::Window& rParent, SlideSorter& rSlideSorter);
    ~ContentWindow (void);
    void SetCurrentFunction (const FunctionReference& rpFunction);
    virtual void Paint(const Rectangle& rRect);
    virtual void KeyInput (const KeyEvent& rEvent);
    virtual void MouseMove (const MouseEvent& rEvent);
    virtual void MouseButtonUp (const MouseEvent& rEvent);
    virtual void MouseButtonDown (const MouseEvent& rEvent);
    virtual void Command (const CommandEvent& rEvent);
    virtual long Notify (NotifyEvent& rEvent);

private:
    SlideSorter& mrSlideSorter;
    FunctionReference mpCurrentFunction;
};
}




//===== SlideSorter ===========================================================

::boost::shared_ptr<SlideSorter> SlideSorter::CreateSlideSorter(
    ViewShell& rViewShell,
    const ::boost::shared_ptr<sd::Window>& rpContentWindow,
    const ::boost::shared_ptr<ScrollBar>& rpHorizontalScrollBar,
    const ::boost::shared_ptr<ScrollBar>& rpVerticalScrollBar,
    const ::boost::shared_ptr<ScrollBarBox>& rpScrollBarBox)
{
    ::boost::shared_ptr<SlideSorter> pSlideSorter(
        new SlideSorter(
            rViewShell,
            rpContentWindow,
            rpHorizontalScrollBar,
            rpVerticalScrollBar,
            rpScrollBarBox));
    pSlideSorter->Init();
    return pSlideSorter;
}




::boost::shared_ptr<SlideSorter> SlideSorter::CreateSlideSorter (
    ViewShellBase& rBase,
    ViewShell* pViewShell,
    ::Window& rParentWindow)
{
    ::boost::shared_ptr<SlideSorter> pSlideSorter(
        new SlideSorter(
            rBase,
            pViewShell,
            rParentWindow));
    pSlideSorter->Init();
    return pSlideSorter;
}




SlideSorter::SlideSorter (
    ViewShell& rViewShell,
    const ::boost::shared_ptr<sd::Window>& rpContentWindow,
    const ::boost::shared_ptr<ScrollBar>& rpHorizontalScrollBar,
    const ::boost::shared_ptr<ScrollBar>& rpVerticalScrollBar,
    const ::boost::shared_ptr<ScrollBarBox>& rpScrollBarBox)
    : mbIsValid(false),
      mpSlideSorterController(),
      mpSlideSorterModel(),
      mpSlideSorterView(),
      mxControllerWeak(),
      mpViewShell(&rViewShell),
      mpViewShellBase(&rViewShell.GetViewShellBase()),
      mpContentWindow(rpContentWindow),
      mbOwnesContentWindow(false),
      mpHorizontalScrollBar(rpHorizontalScrollBar),
      mpVerticalScrollBar(rpVerticalScrollBar),
      mpScrollBarBox(rpScrollBarBox),
      mbLayoutPending(true),
      mpProperties(new controller::Properties()),
      mpTheme(new view::Theme(mpProperties))
{
}




SlideSorter::SlideSorter (
    ViewShellBase& rBase,
    ViewShell* pViewShell,
    ::Window& rParentWindow)
    : mbIsValid(false),
      mpSlideSorterController(),
      mpSlideSorterModel(),
      mpSlideSorterView(),
      mxControllerWeak(),
      mpViewShell(pViewShell),
      mpViewShellBase(&rBase),
      mpContentWindow(new ContentWindow(rParentWindow,*this )),
      mbOwnesContentWindow(true),
      mpHorizontalScrollBar(new ScrollBar(&rParentWindow,WinBits(WB_HSCROLL | WB_DRAG))),
      mpVerticalScrollBar(new ScrollBar(&rParentWindow,WinBits(WB_VSCROLL | WB_DRAG))),
      mpScrollBarBox(new ScrollBarBox(&rParentWindow)),
      mbLayoutPending(true),
      mpProperties(new controller::Properties()),
      mpTheme(new view::Theme(mpProperties))
{
}




void SlideSorter::Init (void)
{
    if (mpViewShellBase != NULL)
        mxControllerWeak = mpViewShellBase->GetController();

    // Reinitialize colors in Properties with window specific values.
    if (mpContentWindow)
    {
        mpProperties->SetBackgroundColor(
            mpContentWindow->GetSettings().GetStyleSettings().GetWindowColor());
        mpProperties->SetTextColor(
            mpContentWindow->GetSettings().GetStyleSettings().GetWindowTextColor());
        mpProperties->SetSelectionColor(
            mpContentWindow->GetSettings().GetStyleSettings().GetMenuHighlightColor());
        mpProperties->SetHighlightColor(
            mpContentWindow->GetSettings().GetStyleSettings().GetMenuHighlightColor());
    }

    CreateModelViewController ();

    SetupListeners ();

    // Initialize the window.
    SharedSdWindow pContentWindow (GetContentWindow());
    if (pContentWindow)
    {
        ::Window* pParentWindow = pContentWindow->GetParent();
        if (pParentWindow != NULL)
            pParentWindow->SetBackground(Wallpaper());
        pContentWindow->SetBackground(Wallpaper());
        pContentWindow->SetViewOrigin (Point(0,0));
        // We do our own scrolling while dragging a page selection.
        pContentWindow->SetUseDropScroll (false);
        // Change the winbits so that the active window accepts the focus.
        pContentWindow->SetStyle ((pContentWindow->GetStyle() & ~WB_DIALOGCONTROL) | WB_TABSTOP);
        pContentWindow->Hide();

        // Set view pointer of base class.
        SetupControls(pParentWindow);

        mbIsValid = true;
    }
}




SlideSorter::~SlideSorter (void)
{
    mbIsValid = false;

    ReleaseListeners();

    // Dispose model, view and controller to avoid calls between them when
    // they are being destructed and one or two of them are already gone.
    mpSlideSorterController->Dispose();
    mpSlideSorterView->Dispose();
    mpSlideSorterModel->Dispose();

    // Reset the auto pointers explicitly to control the order of destruction.
    mpSlideSorterController.reset();
    mpSlideSorterView.reset();
    mpSlideSorterModel.reset();

    mpHorizontalScrollBar.reset();
    mpVerticalScrollBar.reset();
    mpScrollBarBox.reset();

    if (mbOwnesContentWindow)
    {
        OSL_ASSERT(mpContentWindow.unique());
    }
    else
    {
        // Assume that outside this class only the owner holds a reference
        // to the content window.
        OSL_ASSERT(mpContentWindow.use_count()==2);
    }
    mpContentWindow.reset();
}




bool SlideSorter::IsValid (void) const
{
    return mbIsValid;
}




::boost::shared_ptr<ScrollBar> SlideSorter::GetVerticalScrollBar (void) const
{
    return mpVerticalScrollBar;
}





::boost::shared_ptr<ScrollBar> SlideSorter::GetHorizontalScrollBar (void) const
{
    return mpHorizontalScrollBar;
}




::boost::shared_ptr<ScrollBarBox> SlideSorter::GetScrollBarFiller (void) const
{
    return mpScrollBarBox;
}




model::SlideSorterModel& SlideSorter::GetModel (void) const
{
    OSL_ASSERT(mpSlideSorterModel.get()!=NULL);
    return *mpSlideSorterModel;
}




view::SlideSorterView& SlideSorter::GetView (void) const
{
    OSL_ASSERT(mpSlideSorterView.get()!=NULL);
    return *mpSlideSorterView;
}




controller::SlideSorterController& SlideSorter::GetController (void) const
{
    OSL_ASSERT(mpSlideSorterController.get()!=NULL);
    return *mpSlideSorterController;
}




Reference<frame::XController> SlideSorter::GetXController (void) const
{
    Reference<frame::XController> xController(mxControllerWeak);
    return xController;
}




void SlideSorter::Paint (const Rectangle& rRepaintArea)
{
    GetController().Paint(
        rRepaintArea,
        GetContentWindow().get());
}




::SharedSdWindow SlideSorter::GetContentWindow (void) const
{
    return mpContentWindow;
}




ViewShellBase* SlideSorter::GetViewShellBase (void) const
{
    return mpViewShellBase;
}




ViewShell* SlideSorter::GetViewShell (void) const
{
    return mpViewShell;
}




void SlideSorter::SetupControls (::Window* )
{
    GetVerticalScrollBar()->Show();
    mpSlideSorterController->GetScrollBarManager().LateInitialization();
}




void SlideSorter::SetupListeners (void)
{
    SharedSdWindow pWindow (GetContentWindow());
    if (pWindow)
    {
        ::Window* pParentWindow = pWindow->GetParent();
        if (pParentWindow != NULL)
            pParentWindow->AddEventListener(
                LINK(
                    mpSlideSorterController.get(),
                    controller::SlideSorterController,
                    WindowEventHandler));
        pWindow->AddEventListener(
            LINK(
                mpSlideSorterController.get(),
                controller::SlideSorterController,
                WindowEventHandler));
    }
    Application::AddEventListener(
        LINK(
            mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));

    mpSlideSorterController->GetScrollBarManager().Connect();
}




void SlideSorter::ReleaseListeners (void)
{
    mpSlideSorterController->GetScrollBarManager().Disconnect();

    SharedSdWindow pWindow (GetContentWindow());
    if (pWindow)
    {
        pWindow->RemoveEventListener(
            LINK(mpSlideSorterController.get(),
                controller::SlideSorterController,
                WindowEventHandler));

        ::Window* pParentWindow = pWindow->GetParent();
        if (pParentWindow != NULL)
            pParentWindow->RemoveEventListener(
                LINK(mpSlideSorterController.get(),
                    controller::SlideSorterController,
                    WindowEventHandler));
    }
    Application::RemoveEventListener(
        LINK(mpSlideSorterController.get(),
            controller::SlideSorterController,
            WindowEventHandler));
}




void SlideSorter::CreateModelViewController (void)
{
    mpSlideSorterModel.reset(CreateModel());
    DBG_ASSERT (mpSlideSorterModel.get()!=NULL,
        "Can not create model for slide browser");

    mpSlideSorterView.reset(CreateView());
    DBG_ASSERT (mpSlideSorterView.get()!=NULL,
        "Can not create view for slide browser");

    mpSlideSorterController.reset(CreateController());
    DBG_ASSERT (mpSlideSorterController.get()!=NULL,
        "Can not create controller for slide browser");

    // Now that model, view, and controller are constructed, do the
    // initialization that relies on all three being in place.
    mpSlideSorterModel->Init();
    mpSlideSorterController->Init();
    mpSlideSorterView->Init();
}




model::SlideSorterModel* SlideSorter::CreateModel (void)
{
    // Get pointers to the document.
    ViewShellBase* pViewShellBase = GetViewShellBase();
    if (pViewShellBase != NULL)
    {
        OSL_ASSERT (pViewShellBase->GetDocument() != NULL);

        return new model::SlideSorterModel(*this);
    }
    else
        return NULL;
}




view::SlideSorterView* SlideSorter::CreateView (void)
{
    return new view::SlideSorterView (*this);
}




controller::SlideSorterController* SlideSorter::CreateController (void)
{
    controller::SlideSorterController* pController
        = new controller::SlideSorterController (*this);
    return pController;
}




void SlideSorter::ArrangeGUIElements (
    const Point& rOffset,
    const Size& rSize)
{
    Point aOrigin (rOffset);

    if (rSize.Width()>0
        && rSize.Height()>0
        && GetContentWindow()
        && GetContentWindow()->IsVisible())
    {
        // Prevent untimely redraws while the view is not yet correctly
        // resized.
        view::SlideSorterView::DrawLock aLock (*this);
        GetContentWindow()->EnablePaint (sal_False);

        mpSlideSorterController->Resize (Rectangle(aOrigin, rSize));

        GetContentWindow()->EnablePaint (sal_True);

        mbLayoutPending = false;
    }
}




SvBorder SlideSorter::GetBorder (void)
{
    SvBorder aBorder;

    ::boost::shared_ptr<ScrollBar> pScrollBar = GetVerticalScrollBar();
    if (pScrollBar.get() != NULL && pScrollBar->IsVisible())
        aBorder.Right() = pScrollBar->GetOutputSizePixel().Width();

    pScrollBar = GetHorizontalScrollBar();
    if (pScrollBar.get() != NULL && pScrollBar->IsVisible())
        aBorder.Bottom() = pScrollBar->GetOutputSizePixel().Height();

    return aBorder;
}




bool SlideSorter::RelocateToWindow (::Window* pParentWindow)
{
   // Stop all animations for they have been started for the old window.
    mpSlideSorterController->GetAnimator()->RemoveAllAnimations();

    ReleaseListeners();

    if (mpViewShell != NULL)
        mpViewShell->ViewShell::RelocateToParentWindow(pParentWindow);

    SetupControls(mpViewShell->GetParentWindow());
    SetupListeners();

    // For accessibility we have to shortly hide the content window.  This
    // triggers the construction of a new accessibility object for the new
    // view shell.  (One is created earlier while the construtor of the base
    // class is executed.  But because at that time the correct
    // accessibility object can not be constructed we do that now.)
    if (mpContentWindow.get() !=NULL)
    {
        mpContentWindow->Hide();
        mpContentWindow->Show();
    }

    return true;
}




void SlideSorter::SetCurrentFunction (const FunctionReference& rpFunction)
{
    if (GetViewShell() != NULL)
    {
        GetViewShell()->SetCurrentFunction(rpFunction);
        GetViewShell()->SetOldFunction(rpFunction);
    }
    else
    {
        ContentWindow* pWindow = dynamic_cast<ContentWindow*>(GetContentWindow().get());
        if (pWindow != NULL)
            pWindow->SetCurrentFunction(rpFunction);
    }
}




::boost::shared_ptr<controller::Properties> SlideSorter::GetProperties (void) const
{
    OSL_ASSERT(mpProperties);
    return mpProperties;
}




::boost::shared_ptr<view::Theme> SlideSorter::GetTheme (void) const
{
    OSL_ASSERT(mpTheme);
    return mpTheme;
}




//===== ContentWindow =========================================================

namespace {

ContentWindow::ContentWindow(
    ::Window& rParent,
    SlideSorter& rSlideSorter)
    : ::sd::Window(&rParent),
    mrSlideSorter(rSlideSorter),
    mpCurrentFunction()
{
    SetDialogControlFlags(GetDialogControlFlags() & ~WINDOW_DLGCTRL_WANTFOCUS);
    SetStyle(GetStyle() | WB_NOPOINTERFOCUS);
}




ContentWindow::~ContentWindow (void)
{
}




void ContentWindow::SetCurrentFunction (const FunctionReference& rpFunction)
{
    mpCurrentFunction = rpFunction;
}




void ContentWindow::Paint (const Rectangle& rRect)
{
    mrSlideSorter.Paint(rRect);
}




void ContentWindow::KeyInput (const KeyEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->KeyInput(rEvent);
}




void ContentWindow::MouseMove (const MouseEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->MouseMove(rEvent);
}




void ContentWindow::MouseButtonUp(const MouseEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->MouseButtonUp(rEvent);
}




void ContentWindow::MouseButtonDown(const MouseEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->MouseButtonDown(rEvent);
}




void ContentWindow::Command(const CommandEvent& rEvent)
{
    if (mpCurrentFunction.is())
        mpCurrentFunction->Command(rEvent);
}




long ContentWindow::Notify (NotifyEvent& rEvent)
{
    (void)rEvent;
    return 0;
}



} // end of anonymous namespace





} } // end of namespace ::sd::slidesorter

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
