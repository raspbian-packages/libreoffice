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
#if defined(_MSC_VER) && (_MSC_VER > 1310)
#pragma warning(disable : 4917 4555)
#endif

#include "docholder.hxx"
#include "syswinwrapper.hxx"

/*
 * CWindow::CWindow
 * CWindow::~CWindow
 *
 * Constructor Parameters:
 *  hInst           HINSTANCE of the task owning us.
 */


using namespace winwrap;


#define HWWL_STRUCTURE                  0

//Notification codes for WM_COMMAND messages
#define HWN_BORDERDOUBLECLICKED         1
#define CBHATCHWNDEXTRA                 (sizeof(LONG))
#define SZCLASSHATCHWIN                 TEXT("hatchwin")
#define SendCommand(hWnd, wID, wCode, hControl)                     \
            SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(wID, wCode)    \
                        , (LPARAM)hControl)


typedef CHatchWin *PCHatchWin;


void DrawShading(LPRECT prc, HDC hDC, UINT cWidth);



winwrap::CWindow::CWindow(HINSTANCE hInst)
{
    m_hInst=hInst;
    m_hWnd=NULL;
    return;
}

winwrap::CWindow::~CWindow(void)
{
    if (IsWindow(m_hWnd))
        DestroyWindow(m_hWnd);

    return;
}



/*
 * CWindow::Window
 *
 * Purpose:
 *  Returns the window handle associated with this object.
 *
 * Return Value:
 *  HWND            Window handle for this object
 */

HWND winwrap::CWindow::Window(void)
{
    return m_hWnd;
}



/*
 * CWindow::Instance
 *
 * Purpose:
 *  Returns the instance handle associated with this object.
 *
 * Return Value:
 *  HINSTANCE       Instance handle of the module stored here.
 */

HINSTANCE winwrap::CWindow::Instance(void)
{
    return m_hInst;
}





//Hatch pattern brush bits
static WORD g_wHatchBmp[]={0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88};

// void DrawShading(LPRECT, HDC, UINT);


/*
 * HatchWindowRegister
 *
 * Purpose:
 *  Registers the hatch window class for use with CHatchWin.
 *
 * Parameters:
 *  hInst           HINSTANCE under which to register.
 *
 * Return Value:
 *  BOOL            TRUE if successful, FALSE otherwise.
 */

BOOL winwrap::HatchWindowRegister(HINSTANCE hInst)
{
    WNDCLASS    wc;

    //Must have CS_DBLCLKS for border!
    wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    wc.hInstance     = hInst;
    wc.cbClsExtra    = 0;
    wc.lpfnWndProc   = HatchWndProc;
    wc.cbWndExtra    = CBHATCHWNDEXTRA;
    wc.hIcon         = NULL;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = SZCLASSHATCHWIN;

    return RegisterClass(&wc);
}




/*
 * CHatchWin:CHatchWin
 * CHatchWin::~CHatchWin
 *
 * Constructor Parameters:
 *  hInst           HINSTANCE of the application we're in.
 */

CHatchWin::CHatchWin(HINSTANCE hInst,const DocumentHolder* pDocHolder)
    : CWindow(hInst),
      m_aTracker()
{
    m_hWnd=NULL;
    m_hWndKid=NULL;
    m_hWndAssociate=NULL;
    m_uID=0;

    m_dBorderOrg=GetProfileInt(TEXT("windows")
                               , TEXT("OleInPlaceBorderWidth")
                               , HATCHWIN_BORDERWIDTHDEFAULT);

    m_dBorder=m_dBorderOrg;
    SetRect(&m_rcPos, 0, 0, 0, 0);
    SetRect(&m_rcClip, 0, 0, 0, 0);

    m_pDocHolder = pDocHolder;
    return;
}


CHatchWin::~CHatchWin(void)
{
    /*
     * Chances are this was already destroyed when a document
     * was destroyed.
     */
    if (NULL!=m_hWnd && IsWindow(m_hWnd))
        DestroyWindow(m_hWnd);

    return;
}



/*
 * CHatchWin::Init
 *
 * Purpose:
 *  Instantiates a hatch window within a given parent with a
 *  default rectangle.  This is not initially visible.
 *
 * Parameters:
 *  hWndParent      HWND of the parent of this window
 *  uID             UINT identifier for this window (send in
 *                  notifications to associate window).
 *  hWndAssoc       HWND of the initial associate.
 *
 * Return Value:
 *  BOOL            TRUE if the function succeeded, FALSE otherwise.
 */

BOOL CHatchWin::Init(HWND hWndParent, UINT uID, HWND hWndAssoc)
{
    m_hWndParent = hWndParent;
    m_hWnd=CreateWindowEx(
        WS_EX_NOPARENTNOTIFY, SZCLASSHATCHWIN
        , SZCLASSHATCHWIN, WS_CHILD | WS_CLIPSIBLINGS
        | WS_CLIPCHILDREN, 0, 0, 100, 100, hWndParent, (HMENU)uID
        , m_hInst, this);

    m_uID=uID;
    m_hWndAssociate=hWndAssoc;

    return (NULL!=m_hWnd);
}


void CHatchWin::SetTrans()
{
    HRGN hrgn = CreateRectRgn(0,0,0,0);
    SetWindowRgn(m_hWnd,hrgn,true);
}

/*
 * CHatchWin::HwndAssociateSet
 * CHatchWin::HwndAssociateGet
 *
 * Purpose:
 *  Sets (Set) or retrieves (Get) the associate window of the
 *  hatch window.
 *
 * Parameters: (Set only)
 *  hWndAssoc       HWND to set as the associate.
 *
 * Return Value:
 *  HWND            Previous (Set) or current (Get) associate
 *                  window.
 */

HWND CHatchWin::HwndAssociateSet(HWND hWndAssoc)
{
    HWND    hWndT=m_hWndAssociate;

    m_hWndAssociate=hWndAssoc;
    return hWndT;
}


HWND CHatchWin::HwndAssociateGet(void)
{
    return m_hWndAssociate;
}


/*
 * CHatchWin::RectsSet
 *
 * Purpose:
 *  Changes the size and position of the hatch window and the child
 *  window within it using a position rectangle for the child and
 *  a clipping rectangle for the hatch window and child.  The hatch
 *  window occupies prcPos expanded by the hatch border and clipped
 *  by prcClip.  The child window is fit to prcPos to give the
 *  proper scaling, but it clipped to the hatch window which
 *  therefore clips it to prcClip without affecting the scaling.
 *
 * Parameters:
 *  prcPos          LPRECT providing the position rectangle.
 *  prcClip         LPRECT providing the clipping rectangle.
 *
 * Return Value:
 *  None
 */

void CHatchWin::RectsSet(LPRECT prcPos, LPRECT prcClip)
{
    RECT    rc;
    RECT    rcPos;

    m_rcPos=*prcPos;
    m_rcClip=*prcClip;

    //Calculate the rectangle for the hatch window, then clip it.
    rcPos=*prcPos;
    InflateRect(&rcPos, m_dBorder, m_dBorder);
    IntersectRect(&rc, &rcPos, prcClip);

    SetWindowPos(m_hWnd, NULL, rc.left, rc.top, rc.right-rc.left
                 , rc.bottom-rc.top, SWP_NOZORDER | SWP_NOACTIVATE);

    /*
     * Set the rectangle of the child window to be at m_dBorder
     * from the top and left but with the same size as prcPos
     * contains.  The hatch window will clip it.
     */
//     SetWindowPos(m_hWndKid, NULL, rcPos.left-rc.left+m_dBorder
//                  , rcPos.top-rc.top+m_dBorder, prcPos->right-prcPos->left
//                  , prcPos->bottom-prcPos->top, SWP_NOZORDER | SWP_NOACTIVATE);

    RECT newRC;
    GetClientRect(m_hWnd,&newRC);
    m_aTracker = Tracker(
        &newRC,
        Tracker::hatchInside |
        Tracker::hatchedBorder |
        Tracker::resizeInside
    );

    return;
}



/*
 * CHatchWin::ChildSet
 *
 * Purpose:
 *  Assigns a child window to this hatch window.
 *
 * Parameters:
 *  hWndKid         HWND of the child window.
 *
 * Return Value:
 *  None
 */

void CHatchWin::ChildSet(HWND hWndKid)
{
    m_hWndKid=hWndKid;

    if (NULL!=hWndKid)
    {
        SetParent(hWndKid, m_hWnd);

        //Insure this is visible when the hatch window becomes visible.
        ShowWindow(hWndKid, SW_SHOW);
    }

    return;
}



/*
 * CHatchWin::ShowHatch
 *
 * Purpose:
 *  Turns hatching on and off; turning the hatching off changes
 *  the size of the window to be exactly that of the child, leaving
 *  everything else the same.  The result is that we don't have
 *  to turn off drawing because our own WM_PAINT will never be
 *  called.
 *
 * Parameters:
 *  fHatch          BOOL indicating to show (TRUE) or hide (FALSE)
                    the hatching.
 *
 * Return Value:
 *  None
 */

void CHatchWin::ShowHatch(BOOL fHatch)
{
    /*
     * All we have to do is set the border to zero and
     * call SetRects again with the last rectangles the
     * child sent to us.
     */
    m_dBorder=fHatch ? m_dBorderOrg : 0;
    RectsSet(&m_rcPos, &m_rcClip);
    return;
}



/*
 * HatchWndProc
 *
 * Purpose:
 *  Standard window procedure for the Hatch Window
 */

LRESULT APIENTRY winwrap::HatchWndProc(
    HWND hWnd, UINT iMsg
    , WPARAM wParam, LPARAM lParam)
{
    PCHatchWin  phw;
    HDC         hDC;
    PAINTSTRUCT ps;

    phw=(PCHatchWin)GetWindowLong(hWnd, HWWL_STRUCTURE);
    POINT ptMouse;

    switch (iMsg)
    {
        case WM_CREATE:
            phw=(PCHatchWin)((LPCREATESTRUCT)lParam)->lpCreateParams;
            SetWindowLong(hWnd, HWWL_STRUCTURE, (LONG)phw);
            break;
        case WM_PAINT:
            hDC=BeginPaint(hWnd,&ps);
            //Always draw the hatching.
            phw->m_aTracker.Draw(hDC);
            EndPaint(hWnd,&ps);
            break;
        case WM_LBUTTONDOWN:
            GetCursorPos(&ptMouse);
            ScreenToClient(hWnd,&ptMouse);

            // track in case we have to
            if(phw->m_aTracker.Track(hWnd,ptMouse,FALSE,GetParent(hWnd)))
            {
                RECT aRect = phw->m_aTracker.m_rect;
                TransformRect(&aRect,hWnd,GetParent(hWnd));
                phw->m_pDocHolder->OnPosRectChanged(&aRect);
            }
            break;
        case WM_LBUTTONUP:
        case WM_MOUSEMOVE:
            GetCursorPos(&ptMouse);
            ScreenToClient(hWnd,&ptMouse);
            phw->m_aTracker.SetCursor(hWnd,HTCLIENT);
            break;
        case WM_SETFOCUS:
            //We need this since the container will SetFocus to us.
            if (NULL!=phw->m_hWndKid)
                SetFocus(phw->m_hWndKid);

            break;
        case WM_LBUTTONDBLCLK:
            /*
             * If the double click was within m_dBorder of an
             * edge, send the HWN_BORDERDOUBLECLICKED notification.
             *
             * Because we're always sized just larger than our child
             * window by the border width, we can only *get* this
             * message when the mouse is on the border.  So we can
             * just send the notification.
             */
            if (NULL!=phw->m_hWndAssociate)
            {
                SendCommand(phw->m_hWndAssociate, phw->m_uID
                            , HWN_BORDERDOUBLECLICKED, hWnd);
            }

            break;
        default:
            return DefWindowProc(hWnd, iMsg, wParam, lParam);
    }

    return 0L;
}

// Fix strange warnings about some
// ATL::CAxHostWindow::QueryInterface|AddRef|Releae functions.
// warning C4505: 'xxx' : unreferenced local function has been removed
#if defined(_MSC_VER)
#pragma warning(disable: 4505)
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
