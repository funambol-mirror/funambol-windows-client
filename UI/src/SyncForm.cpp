/*
 * Funambol is a mobile platform developed by Funambol, Inc. 
 * Copyright (C) 2003 - 2007 Funambol, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License version 3 as published by
 * the Free Software Foundation with the addition of the following permission 
 * added to Section 15 as permitted in Section 7(a): FOR ANY PART OF THE COVERED
 * WORK IN WHICH THE COPYRIGHT IS OWNED BY FUNAMBOL, FUNAMBOL DISCLAIMS THE 
 * WARRANTY OF NON INFRINGEMENT  OF THIRD PARTY RIGHTS.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Affero General Public License 
 * along with this program; if not, see http://www.gnu.org/licenses or write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301 USA.
 * 
 * You can contact Funambol, Inc. headquarters at 643 Bair Island Road, Suite 
 * 305, Redwood City, CA 94063, USA, or at email address info@funambol.com.
 * 
 * The interactive user interfaces in modified source and object code versions
 * of this program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU Affero General Public License version 3.
 * 
 * In accordance with Section 7(b) of the GNU Affero General Public License
 * version 3, these Appropriate Legal Notices must retain the display of the
 * "Powered by Funambol" logo. If the display of the logo is not reasonably 
 * feasible for technical reasons, the Appropriate Legal Notices must display
 * the words "Powered by Funambol".
 */


#pragma once
#include "stdafx.h"
#include "OutlookPlugin.h"
#include "SyncForm.h"
#include "MainSyncFrm.h"
#include "winmaincpp.h"
#include "ClientUtil.h"
#include "UICustomization.h"
#include "utils.h"
#include "AnimatedIcon.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNCREATE(CSyncForm, CFormView)


static int getLastSourceState(const char* sourceName) {

    WindowsSyncSourceConfig* ssc = getConfig()->getSyncSourceConfig(sourceName);
    if (!ssc) return SYNCSOURCE_STATE_OK;

    int error = ssc->getCommonConfig()->getLastSourceError(); 
    int ret = manageWinErrors(error);
    return ret;
}

void startCheckMediaFolderThread();

CSyncForm::CSyncForm()
    : CFormView(CSyncForm::IDD)
{
    syncSourceContactState  = getLastSourceState(CONTACT_);
    syncSourceCalendarState = getLastSourceState(APPOINTMENT_);
    syncSourceTaskState     = getLastSourceState(TASK_);
    syncSourceNoteState     = getLastSourceState(NOTE_);
    syncSourcePictureState  = getLastSourceState(PICTURE_);
    syncSourceVideoState    = getLastSourceState(VIDEO_);
    syncSourceFileState     = getLastSourceState(FILES_);

    lockedUI = false;

    panesCount = countSourceVisible();
    
}

CSyncForm::~CSyncForm()
{
}

void CSyncForm::DoDataExchange(CDataExchange* pDX)
{
    CFormView::DoDataExchange(pDX);

    DDX_Control(pDX, IDC_MAIN_BUT_START,        butStart);
    
    DDX_Control(pDX, IDC_MAIN_ICON_CONTACTS, iconContacts);
    DDX_Control(pDX, IDC_MAIN_ICON_CALENDAR, iconCalendar);
    DDX_Control(pDX, IDC_MAIN_ICON_TASKS,    iconTasks);
    DDX_Control(pDX, IDC_MAIN_ICON_NOTES,    iconNotes);
    DDX_Control(pDX, IDC_MAIN_ICON_PICTURES, iconPictures);
    DDX_Control(pDX, IDC_MAIN_ICON_VIDEOS,   iconVideos);
    DDX_Control(pDX, IDC_MAIN_ICON_FILES,    iconFiles);

    DDX_Control(pDX, IDC_MAIN_ICON_STATUS_SYNC,     iconStatusSync);
    DDX_Control(pDX, IDC_MAIN_ICON_STATUS_CONTACTS, iconStatusContacts);
    DDX_Control(pDX, IDC_MAIN_ICON_STATUS_CALENDAR, iconStatusCalendar);
    DDX_Control(pDX, IDC_MAIN_ICON_STATUS_TASKS,    iconStatusTasks);
    DDX_Control(pDX, IDC_MAIN_ICON_STATUS_NOTES,    iconStatusNotes);
    DDX_Control(pDX, IDC_MAIN_ICON_STATUS_PICTURES, iconStatusPictures);
    DDX_Control(pDX, IDC_MAIN_ICON_STATUS_VIDEOS,   iconStatusVideos);
    DDX_Control(pDX, IDC_MAIN_ICON_STATUS_FILES,    iconStatusFiles);

    DDX_Control(pDX, IDC_MAIN_BK_SYNC,     paneSync);
    DDX_Control(pDX, IDC_MAIN_BK_CONTACTS, paneContacts);
    DDX_Control(pDX, IDC_MAIN_BK_CALENDAR, paneCalendar);
    DDX_Control(pDX, IDC_MAIN_BK_TASKS,    paneTasks);
    DDX_Control(pDX, IDC_MAIN_BK_NOTES,    paneNotes);
    DDX_Control(pDX, IDC_MAIN_BK_PICTURES, panePictures);
    DDX_Control(pDX, IDC_MAIN_BK_VIDEOS,   paneVideos);
    DDX_Control(pDX, IDC_MAIN_BK_FILES,    paneFiles);
}


BEGIN_MESSAGE_MAP(CSyncForm, CFormView)

    ON_MESSAGE( WM_INITDIALOG, OnInitForm ) 
    ON_WM_NCPAINT( )
    ON_WM_PAINT( )
    ON_WM_CTLCOLOR()
    ON_WM_ERASEBKGND()
    
    ON_STN_CLICKED(IDC_MAIN_BK_CONTACTS, &CSyncForm::OnStnClickedMainBkContacts)
    ON_STN_CLICKED(IDC_MAIN_BK_SYNC,     &CSyncForm::OnStnClickedMainBkSync)
    ON_STN_CLICKED(IDC_MAIN_BK_CALENDAR, &CSyncForm::OnStnClickedMainBkCalendar)
    ON_STN_CLICKED(IDC_MAIN_BK_TASKS,    &CSyncForm::OnStnClickedMainBkTasks)
    ON_STN_CLICKED(IDC_MAIN_BK_NOTES,    &CSyncForm::OnStnClickedMainBkNotes)
    ON_STN_CLICKED(IDC_MAIN_BK_PICTURES, &CSyncForm::OnStnClickedMainBkPictures)
    ON_STN_CLICKED(IDC_MAIN_BK_VIDEOS,   &CSyncForm::OnStnClickedMainBkVideos)
    ON_STN_CLICKED(IDC_MAIN_BK_FILES,    &CSyncForm::OnStnClickedMainBkFiles)

END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSyncForm diagnostics
#ifdef _DEBUG
void CSyncForm::AssertValid() const
{
	CFormView::AssertValid();
}
void CSyncForm::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif //_DEBUG


/////////////////////////////////////////////////////////////////////////////
// CSyncForm message handlers
LRESULT CSyncForm::OnInitForm(WPARAM, LPARAM) {
    CFormView::OnInitialUpdate(); //!!

    CString s1; 

    // IDS_<source> are used in Sync Settings pane, here we use IDS_MAIN_<source>
    //s1.LoadString(IDS_MAIN_PRESS_TO_SYNC); SetDlgItemText(IDC_MAIN_MSG_PRESS, s1);
    s1.LoadString(IDS_SYNCALL); SetDlgItemText(IDC_MAIN_MSG_PRESS, s1);

    // Load pane titles
    contactsLabel.LoadString(IDS_MAIN_CONTACTS);
    calendarLabel.LoadString(IDS_MAIN_CALENDAR);
    tasksLabel.LoadString   (IDS_MAIN_TASKS);
    notesLabel.LoadString   (IDS_MAIN_NOTES);
    picturesLabel.LoadString(IDS_MAIN_PICTURES);
    videosLabel.LoadString  (IDS_MAIN_VIDEOS);
    filesLabel.LoadString   (IDS_MAIN_FILES);

    // TODO: for now icon states not really used anywhere
    iconContacts.state = STATE_INVISIBLE;
    iconCalendar.state = STATE_INVISIBLE;
    iconTasks.state    = STATE_INVISIBLE;
    iconNotes.state    = STATE_INVISIBLE;
    iconPictures.state = STATE_INVISIBLE;
    iconVideos.state   = STATE_INVISIBLE;
    iconFiles.state    = STATE_INVISIBLE;

    butStart.SetIcon(::LoadIcon(AfxGetInstanceHandle(),MAKEINTRESOURCE(IDI_LOGO)));

    // set font to source labels
    LOGFONT lf;
    memset(&lf, 0, sizeof(LOGFONT));  
    lf.lfWeight = FW_BOLD; 
    lf.lfHeight =-14;
    wcscpy(lf.lfFaceName, _T("Tahoma"));
    VERIFY(fontBold.CreateFontIndirect(&lf));  

    memset(&lf, 0, sizeof(LOGFONT));  
    lf.lfHeight = -9;
    wcscpy(lf.lfFaceName, _T("Tahoma"));
    VERIFY(fontNormal.CreateFontIndirect(&lf));  

    GetDlgItem(IDC_MAIN_STATIC_CONTACTS)->SetFont(&fontBold);
    GetDlgItem(IDC_MAIN_STATIC_CALENDAR)->SetFont(&fontBold);
    GetDlgItem(IDC_MAIN_STATIC_TASKS)->SetFont(&fontBold);
    GetDlgItem(IDC_MAIN_STATIC_NOTES)->SetFont(&fontBold);
	GetDlgItem(IDC_MAIN_STATIC_PICTURES)->SetFont(&fontBold);
	GetDlgItem(IDC_MAIN_STATIC_VIDEOS)->SetFont(&fontBold);
    GetDlgItem(IDC_MAIN_STATIC_FILES)->SetFont(&fontBold);
    GetDlgItem(IDC_MAIN_MSG_PRESS)->SetFont(&fontBold);

    GetDlgItem(IDC_MAIN_STATIC_STATUS_CONTACTS)->SetFont(&fontNormal);
    GetDlgItem(IDC_MAIN_STATIC_STATUS_CALENDAR)->SetFont(&fontNormal);
    GetDlgItem(IDC_MAIN_STATIC_STATUS_TASKS)->SetFont(&fontNormal);
    GetDlgItem(IDC_MAIN_STATIC_STATUS_NOTES)->SetFont(&fontNormal);
    GetDlgItem(IDC_MAIN_STATIC_STATUS_PICTURES)->SetFont(&fontNormal);
    GetDlgItem(IDC_MAIN_STATIC_STATUS_VIDEOS)->SetFont(&fontNormal);
    GetDlgItem(IDC_MAIN_STATIC_STATUS_FILES)->SetFont(&fontNormal);

    iconStatusSync.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_SYNC_ALL_BLUE)));
    paneSync.type     = PANE_TYPE_SYNC; 
    paneContacts.type = PANE_TYPE_CONTACTS; 
    paneCalendar.type = PANE_TYPE_CALENDAR;
    paneTasks.type    = PANE_TYPE_TASKS; 
    paneNotes.type    = PANE_TYPE_NOTES;
    panePictures.type = PANE_TYPE_PICTURES;
    paneVideos.type   = PANE_TYPE_VIDEOS;
    paneFiles.type    = PANE_TYPE_FILES;

    paneContacts.state = STATE_NORMAL; 
    paneCalendar.state = STATE_NORMAL; 
    paneTasks.state    = STATE_NORMAL; 
    paneNotes.state    = STATE_NORMAL;
    panePictures.state = STATE_NORMAL;
    paneVideos.state   = STATE_NORMAL;
    paneFiles.state    = STATE_NORMAL;

    refreshSources();
    VERIFY(brushHollow.CreateStockObject(HOLLOW_BRUSH));

    // move icon controls depending on dpi
    HDC hdc = ::GetDC(0);
    int dpiX = ::GetDeviceCaps(hdc,LOGPIXELSX);
    int dpiY = ::GetDeviceCaps(hdc,LOGPIXELSY);
    ::ReleaseDC(0,hdc);

    double dx = FRAME_MAIN_X * ((double)dpiX/96);      // default DPI = 96
    if( (dpiX != 96) || (dpiY != 96) ) {
        CRect rectIcon;
   
        iconStatusSync.GetWindowRect(&rectIcon);
        ScreenToClient(&rectIcon);
        iconStatusSync.SetWindowPos(&CWnd::wndTop,
            (int)(dx - rectIcon.Width()- 70),
            rectIcon.TopLeft().y, rectIcon.Width(),
            rectIcon.Height(), SWP_SHOWWINDOW);

        iconStatusContacts.GetWindowRect(&rectIcon);
        ScreenToClient(&rectIcon);
        int posX = (int)(dx - rectIcon.Width()- 70);    // a fixed X position for all status icons...
        iconStatusContacts.SetWindowPos(&CWnd::wndTop,
            posX,
            rectIcon.TopLeft().y, rectIcon.Width(),
            rectIcon.Height(), SWP_SHOWWINDOW);

        iconStatusCalendar.GetWindowRect(&rectIcon);
        ScreenToClient(&rectIcon);
        iconStatusCalendar.SetWindowPos(&CWnd::wndTop, 
            posX,
            rectIcon.TopLeft().y, rectIcon.Width(),
            rectIcon.Height(), SWP_SHOWWINDOW);

        iconStatusTasks.GetWindowRect(&rectIcon);
        ScreenToClient(&rectIcon);
        iconStatusTasks.SetWindowPos(&CWnd::wndTop, 
            posX,
            rectIcon.TopLeft().y, rectIcon.Width(),
            rectIcon.Height(), SWP_SHOWWINDOW);

        iconStatusNotes.GetWindowRect(&rectIcon);
        ScreenToClient(&rectIcon);
        iconStatusNotes.SetWindowPos(&CWnd::wndTop, 
            posX,
            rectIcon.TopLeft().y, rectIcon.Width(),
            rectIcon.Height(), SWP_SHOWWINDOW);

        iconStatusPictures.GetWindowRect(&rectIcon);
        ScreenToClient(&rectIcon);
        iconStatusPictures.SetWindowPos(&CWnd::wndTop, 
            posX,
            rectIcon.TopLeft().y, rectIcon.Width(),
            rectIcon.Height(), SWP_SHOWWINDOW);

        iconStatusVideos.GetWindowRect(&rectIcon);
        ScreenToClient(&rectIcon);
        iconStatusVideos.SetWindowPos(&CWnd::wndTop, 
            posX,
            rectIcon.TopLeft().y, rectIcon.Width(),
            rectIcon.Height(), SWP_SHOWWINDOW);

        iconStatusFiles.GetWindowRect(&rectIcon);
        ScreenToClient(&rectIcon);
        iconStatusFiles.SetWindowPos(&CWnd::wndTop, 
            posX,
            rectIcon.TopLeft().y, rectIcon.Width(),
            rectIcon.Height(), SWP_SHOWWINDOW);
    }



    if (isSourceVisible(PICTURE) || isSourceVisible(VIDEO) || isSourceVisible(FILES)) {
        startCheckMediaFolderThread();        
    }
    return 0;
}

static DWORD WINAPI checkMediaFolderThread(LPVOID lpv) {
    
    for (int i = 0; i < 20; i++) {
        Sleep(300);    
        HWND dd = HwndFunctions::getWindowHandle();
        if (dd == NULL) {
            continue;
        }
        ::SendMessage(dd, ID_MYMSG_CHECK_MEDIA_HUB_FOLDER, 0, 0);    
        break;
    }
    return 0;
}

void startCheckMediaFolderThread() {      
    static bool internalCheckMediaHubFolder = false;
    if (internalCheckMediaHubFolder == false) {
        if ( !CreateThread(NULL, 0, checkMediaFolderThread, (LPVOID)NULL, 0, NULL) ) {
            LOG.error("startCheckMediaFolderThread Error creating thread.");           
        }
    }
    internalCheckMediaHubFolder = true;
}



void CSyncForm::showSyncControls( BOOL show )
{
    if (!show) {
        iconContacts.ShowWindow(SW_HIDE); 
        iconCalendar.ShowWindow(SW_HIDE);
        iconTasks.ShowWindow(SW_HIDE); 
        iconNotes.ShowWindow(SW_HIDE);
        iconPictures.ShowWindow(SW_HIDE);
        iconVideos.ShowWindow(SW_HIDE);
        iconFiles.ShowWindow(SW_HIDE);

        GetDlgItem(IDC_MAIN_STATIC_CONTACTS)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_CALENDAR)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_TASKS)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_NOTES)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_PICTURES)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_VIDEOS)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_FILES)->ShowWindow(SW_HIDE);

        GetDlgItem(IDC_MAIN_STATIC_STATUS_CONTACTS)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_STATUS_CALENDAR)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_STATUS_TASKS)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_STATUS_NOTES)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_STATUS_PICTURES)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_STATUS_VIDEOS)->ShowWindow(SW_HIDE);
        GetDlgItem(IDC_MAIN_STATIC_STATUS_FILES)->ShowWindow(SW_HIDE);
    }
}

void CSyncForm::OnNcPaint(){

    CFormView::OnNcPaint();

    CScrollView::SetScrollSizes(MM_TEXT, CSize(0,0));
    //CScrollView::SetScrollSizes(MM_TEXT, CSize(0,0));         <---- double ???
}
bool done = false;
void CSyncForm::OnPaint(){
    
    // If num sources changed, resize the dialog
    if (!arePIMSourcesVisible() && done == false) {   
        // Resize main window
        CPoint size = getMainWindowSize();          
        //GetDlgItem(IDD_SYNC_FORM)->SetWindowPos(&CWnd::wndTop, NULL, NULL, size.x, size.y, SWP_SHOWWINDOW | SWP_NOMOVE);
        AfxGetMainWnd()->SetWindowPos(&CWnd::wndTop, NULL, NULL, size.x, size.y, SWP_SHOWWINDOW | SWP_NOMOVE);
        
        moveMediaSources();
        done = true;      
    }
        
    CFormView::OnPaint();
   
}

void CSyncForm::moveElement(int resourceFrom, int resourceTo) {

    CRect rect;    
    GetDlgItem(resourceFrom)->GetWindowRect(&rect);
    ScreenToClient(&rect);
    GetDlgItem(resourceTo)->SetWindowPos(&CWnd::wndTop,(int)(rect.TopLeft().x), rect.TopLeft().y, rect.Width(), rect.Height(), SWP_SHOWWINDOW);
    
}

void CSyncForm::moveMediaSources() {
    // CAnimatedIcon& icon, int iconres, int resource1, int resource2, CCustomPane& pane
    hideSource(iconContacts, IDI_CONTACTS, 
                   IDC_MAIN_STATIC_CONTACTS, IDC_MAIN_STATIC_STATUS_CONTACTS,
                   paneContacts);
    hideSource(iconCalendar, IDI_CALENDAR, 
                   IDC_MAIN_STATIC_CALENDAR, IDC_MAIN_STATIC_STATUS_CALENDAR,
                   paneCalendar);
    hideSource(iconTasks, IDI_TASKS, 
                   IDC_MAIN_STATIC_TASKS, IDC_MAIN_STATIC_STATUS_TASKS,
                   paneTasks);
    hideSource(iconNotes, IDI_NOTES, 
                   IDC_MAIN_STATIC_NOTES, IDC_MAIN_STATIC_STATUS_NOTES,
                   paneNotes);        
    GetDlgItem(IDC_MAIN_BK_CONTACTS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_MAIN_BK_CALENDAR)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_MAIN_BK_TASKS)->ShowWindow(SW_HIDE);
    GetDlgItem(IDC_MAIN_BK_NOTES)->ShowWindow(SW_HIDE);
       
    // move pictures to contact
    moveElement(IDC_MAIN_BK_CONTACTS,            IDC_MAIN_BK_PICTURES);
    moveElement(IDC_MAIN_ICON_CONTACTS,          IDC_MAIN_ICON_PICTURES);
    moveElement(IDC_MAIN_ICON_STATUS_CONTACTS,   IDC_MAIN_ICON_STATUS_PICTURES);
    moveElement(IDC_MAIN_STATIC_CONTACTS,        IDC_MAIN_STATIC_PICTURES);
    moveElement(IDC_MAIN_STATIC_STATUS_CONTACTS, IDC_MAIN_STATIC_STATUS_PICTURES);

    // move video to calendar
    moveElement(IDC_MAIN_BK_CALENDAR,            IDC_MAIN_BK_VIDEOS);
    moveElement(IDC_MAIN_ICON_CALENDAR,          IDC_MAIN_ICON_VIDEOS);
    moveElement(IDC_MAIN_ICON_STATUS_CALENDAR,   IDC_MAIN_ICON_STATUS_VIDEOS);
    moveElement(IDC_MAIN_STATIC_CALENDAR,        IDC_MAIN_STATIC_VIDEOS);
    moveElement(IDC_MAIN_STATIC_STATUS_CALENDAR, IDC_MAIN_STATIC_STATUS_VIDEOS);

    // move files to tasks
    moveElement(IDC_MAIN_BK_TASKS,               IDC_MAIN_BK_FILES);
    moveElement(IDC_MAIN_ICON_TASKS,             IDC_MAIN_ICON_FILES);
    moveElement(IDC_MAIN_ICON_STATUS_TASKS,      IDC_MAIN_ICON_STATUS_FILES);
    moveElement(IDC_MAIN_STATIC_TASKS,           IDC_MAIN_STATIC_FILES);
    moveElement(IDC_MAIN_STATIC_STATUS_TASKS,    IDC_MAIN_STATIC_STATUS_FILES);

}


/**
 * Used to change the status text for a source.
 * Using a buffer to avoid re-paintings.
 */
void CSyncForm::changeContactsStatus(CString& status){
    contactsStatusLabel = status;
}
void CSyncForm::changeCalendarStatus(CString& status){
    calendarStatusLabel = status;
}
void CSyncForm::changeTasksStatus(CString& status){
    tasksStatusLabel = status;
}
void CSyncForm::changeNotesStatus(CString& status){
    notesStatusLabel = status;
}
void CSyncForm::changePicturesStatus(CString& status){
    picturesStatusLabel = status;
}
void CSyncForm::changeVideosStatus(CString& status){
    videosStatusLabel = status;
}
void CSyncForm::changeFilesStatus(CString& status){
    filesStatusLabel = status;
}

HBRUSH CSyncForm::OnCtlColor( CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
    HBRUSH hbr = CFormView::OnCtlColor(pDC, pWnd, nCtlColor);

    pDC->SetBkMode(TRANSPARENT);
    if(pWnd->GetDlgCtrlID() == IDC_MAIN_MSG_PRESS){
        //set text color to 'Sync All' text
        int r = UICustomization::syncAllTextRed;
        int g = UICustomization::syncAllTextGreen;
        int b = UICustomization::syncAllTextBlue;
        pDC->SetTextColor(RGB(r,g,b));
    }

	// Colouring in gray if source is STATE_PANE_DISABLED @#@#
	int idCtrl = pWnd->GetDlgCtrlID();
    
    // grayed color
    int rgrayed = 180;
	int ggrayed = 180;
	int bgrayed = 195; // gray but a little blue ...

    int ctrlID = pWnd->GetDlgCtrlID();
    // PANE pictures
	if( ( ctrlID == IDC_MAIN_STATIC_PICTURES )          || 
		( ctrlID == IDC_MAIN_STATIC_STATUS_PICTURES )   
        
        ){
		// set grayed color if pane picture is not allowed....
		if (panePictures.state == STATE_PANE_DISABLED) {
			pDC->SetTextColor(RGB(rgrayed,ggrayed,bgrayed));
		} else {
			pDC->SetTextColor(RGB(0,0,0));
		}
	}
	// PANE videos
	if( ( ctrlID == IDC_MAIN_STATIC_VIDEOS )            ||
        ( ctrlID == IDC_MAIN_STATIC_STATUS_VIDEOS )     
        ){
		// set grayed color if pane picture is not allowed....
		if (paneVideos.state == STATE_PANE_DISABLED) {
			pDC->SetTextColor(RGB(rgrayed,ggrayed,bgrayed));
		} else {
			pDC->SetTextColor(RGB(0,0,0));
		}
	} 
    // PANE files
	if( (  ctrlID == IDC_MAIN_STATIC_FILES )             ||
        ( ctrlID == IDC_MAIN_STATIC_STATUS_FILES )      
       ){
		// set grayed color if pane picture is not allowed....
		if (paneFiles.state == STATE_PANE_DISABLED) {
			pDC->SetTextColor(RGB(rgrayed,ggrayed,bgrayed));
		} else {
			pDC->SetTextColor(RGB(0,0,0));
		}
	} 

    if(pWnd->GetRuntimeClass() == RUNTIME_CLASS(CAnimatedIcon) ){
        return HBRUSH(brushHollow);
    }
    
    switch(nCtlColor) {
            case CTLCOLOR_STATIC:
            case CTLCOLOR_BTN:
                // let static controls shine through
                pDC->SetBkMode(TRANSPARENT);
                return HBRUSH(brushHollow);

            default:
                break;
    }

    return hbr;
}

void CSyncForm::refreshSources() {

    // refresh all sources
    refreshSource(SYNCSOURCE_CONTACTS);
    refreshSource(SYNCSOURCE_CALENDAR);
    refreshSource(SYNCSOURCE_TASKS);
    refreshSource(SYNCSOURCE_NOTES);
    refreshSource(SYNCSOURCE_PICTURES);
    refreshSource(SYNCSOURCE_VIDEOS);
    refreshSource(SYNCSOURCE_FILES);


    // TODO: this is needed
    if(AfxGetMainWnd() != NULL){
        paneContacts.SetBitmap(((CMainSyncFrame*)AfxGetMainWnd())->hBmpLight);
        paneCalendar.SetBitmap(((CMainSyncFrame*)AfxGetMainWnd())->hBmpLight);
        paneTasks.SetBitmap   (((CMainSyncFrame*)AfxGetMainWnd())->hBmpLight);
        paneNotes.SetBitmap   (((CMainSyncFrame*)AfxGetMainWnd())->hBmpLight);
        panePictures.SetBitmap(((CMainSyncFrame*)AfxGetMainWnd())->hBmpLight);
        paneVideos.SetBitmap  (((CMainSyncFrame*)AfxGetMainWnd())->hBmpLight);
        paneFiles.SetBitmap   (((CMainSyncFrame*)AfxGetMainWnd())->hBmpLight);
    }

    // This is done to correctly refresh also disabled sources!
    Invalidate();
}

BOOL CSyncForm::OnEraseBkgnd(CDC* pDC)
{
    return TRUE;
}



void CSyncForm::OnDraw(CDC* pDC){
    CRect rect;
    GetClientRect(&rect);
    CDC dc;
    dc.CreateCompatibleDC(pDC);

    CRect rect1;
    GetWindowRect(&rect1);
    ScreenToClient(&rect1);

    pDC->FillSolidRect(rect, COLOR_EXT_PANE);

    // no need for color, already has backgrounds
    dc.DeleteDC();
}

void CSyncForm::repaintPaneControls(int paneType) {

    if(paneType == PANE_TYPE_SYNC){
        iconStatusSync.Invalidate();
        butStart.Invalidate();
        GetDlgItem(IDC_MAIN_MSG_PRESS)->Invalidate();
    }
    else if (paneType == PANE_TYPE_CONTACTS) {
        iconStatusContacts.Invalidate();
        iconContacts.Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_CONTACTS, contactsLabel);                    // Always fixed
        GetDlgItem(IDC_MAIN_STATIC_CONTACTS)->Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_CONTACTS, contactsStatusLabel);       // Use the buffer set by 'changeContactsStatus'
        GetDlgItem(IDC_MAIN_STATIC_STATUS_CONTACTS)->Invalidate();
    }
    else if (paneType == PANE_TYPE_CALENDAR) {
        iconStatusCalendar.Invalidate();
        iconCalendar.Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_CALENDAR, calendarLabel);                    // Always fixed
        GetDlgItem(IDC_MAIN_STATIC_CALENDAR)->Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_CALENDAR, calendarStatusLabel);       // Use the buffer set by 'changeCalendarStatus'
        GetDlgItem(IDC_MAIN_STATIC_STATUS_CALENDAR)->Invalidate();
    }
    else if (paneType == PANE_TYPE_TASKS) {
        iconStatusTasks.Invalidate();
        iconTasks.Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_TASKS, tasksLabel);                          // Always fixed
        GetDlgItem(IDC_MAIN_STATIC_TASKS)->Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_TASKS, tasksStatusLabel);             // Use the buffer set by 'changeTasksStatus'
        GetDlgItem(IDC_MAIN_STATIC_STATUS_TASKS)->Invalidate();
    }
    else if (paneType == PANE_TYPE_NOTES) {
        iconStatusNotes.Invalidate();
        iconNotes.Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_NOTES, notesLabel);                          // Always fixed
        GetDlgItem(IDC_MAIN_STATIC_NOTES)->Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_NOTES, notesStatusLabel);             // Use the buffer set by 'changeNotesStatus'
        GetDlgItem(IDC_MAIN_STATIC_STATUS_NOTES)->Invalidate();
    }
    else if (paneType == PANE_TYPE_PICTURES) {
        iconStatusPictures.Invalidate();
        iconPictures.Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_PICTURES, picturesLabel);                     // Always fixed
        GetDlgItem(IDC_MAIN_STATIC_PICTURES)->Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_PICTURES, picturesStatusLabel);        // Use the buffer set by 'changePicturesStatus'
        GetDlgItem(IDC_MAIN_STATIC_STATUS_PICTURES)->Invalidate();
    }
    else if (paneType == PANE_TYPE_VIDEOS) {
        iconStatusVideos.Invalidate();
        iconVideos.Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_VIDEOS, videosLabel);                     // Always fixed
        GetDlgItem(IDC_MAIN_STATIC_VIDEOS)->Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_VIDEOS, videosStatusLabel);        // Use the buffer set by 'changeVideosStatus'
        GetDlgItem(IDC_MAIN_STATIC_STATUS_VIDEOS)->Invalidate();
    }
    else if (paneType == PANE_TYPE_FILES) {
        iconStatusFiles.Invalidate();
        iconFiles.Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_FILES, filesLabel);                     // Always fixed
        GetDlgItem(IDC_MAIN_STATIC_FILES)->Invalidate();
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_FILES, filesStatusLabel);        // Use the buffer set by 'changeFilesStatus'
        GetDlgItem(IDC_MAIN_STATIC_STATUS_FILES)->Invalidate();
    }
}



void CSyncForm::OnStnClickedMainBkSync()
{
    if (lockedUI) {
        return;
    }

    CString s1;
    CMainSyncFrame *pFrame=(CMainSyncFrame*)AfxGetMainWnd();
    if(  (!checkSyncInProgress()) ){
        // No sync in progress -> StartSync.
        pFrame->StartSync();
    }
    else{
        if (getConfig()->getScheduledSync()) {
            // It's running a scheduled sync -> error msg.
            s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
            wsafeMessageBox(s1);
        }
        else {
            // It's running a normal sync -> CancelSync.
            pFrame->CancelSync();
        }
    }
}

void CSyncForm::OnStnClickedMainBkContacts()
{
    if (lockedUI) {
        return;
    }

    if ( (paneContacts.state == STATE_PANE_DISABLED) || (paneContacts.state == STATE_SYNC) )
        return;

    if (checkSyncInProgress()) {
        // It's running a sync -> error msg.
        CString s1;
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
    }
    else {
        // Start Sync of a single source
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(TASK_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(NOTE_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(PICTURE_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(VIDEO_      )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(FILES_      )->setIsEnabled(false);

        ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();
    }
}

void CSyncForm::OnStnClickedMainBkCalendar()
{
    if (lockedUI) {
        return;
    }

    if ( (paneCalendar.state == STATE_PANE_DISABLED) || (paneCalendar.state == STATE_SYNC) )
        return;

    if (checkSyncInProgress()) {
        // It's running a sync -> error msg.
        CString s1;
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
    }
    else {
        // Start Sync of a single source
        getConfig()->getSyncSourceConfig(CONTACT_)->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(TASK_   )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(NOTE_   )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(PICTURE_)->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(VIDEO_  )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(FILES_  )->setIsEnabled(false);

        ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();
    }
}

void CSyncForm::OnStnClickedMainBkTasks()
{
    if (lockedUI) {
        return;
    }

    if ( (paneTasks.state == STATE_PANE_DISABLED) || (paneTasks.state == STATE_SYNC) )
        return;

    if (checkSyncInProgress()) {
        // It's running a sync -> error msg.
        CString s1;
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
    }
    else {
        // Start Sync of a single source
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(CONTACT_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(NOTE_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(PICTURE_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(VIDEO_      )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(FILES_      )->setIsEnabled(false);

        ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();
    }
}

void CSyncForm::OnStnClickedMainBkNotes()
{
    if (lockedUI) {
        return;
    }

    if ( ( paneNotes.state == STATE_SYNC ) || (paneNotes.state == STATE_PANE_DISABLED ) ) {
        return;
    }
  

    if (checkSyncInProgress()) {
        // It's running a sync -> error msg.
        CString s1;
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
    }
    else {
        // Start Sync of a single source
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(CONTACT_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(TASK_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(PICTURE_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(VIDEO_      )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(FILES_      )->setIsEnabled(false);

        ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();
    }
}


void CSyncForm::OnStnClickedMainBkPictures()
{
    if (lockedUI) {
        return;
    }

    if ( panePictures.state == STATE_SYNC )
        return;

    if ( panePictures.state == STATE_PANE_DISABLED ) {
        // 
        CString str;
        CString sourceName;
        sourceName.LoadString(IDS_MAIN_PICTURES);
        str.FormatMessage(IDS_WARNING_PAYMENT_REQUIRED_FOR_SOURCE, sourceName, _T(DATA_PLAN_WEB_PAGE));

        MessageBox((LPCTSTR)str, TEXT(PROGRAM_NAME), MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_SETFOREGROUND);
        return;
    }

    if (checkSyncInProgress()) {
        // It's running a sync -> error msg.
        CString s1;
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
    }
    else {
        // Start Sync of a single source
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(CONTACT_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(TASK_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(NOTE_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(VIDEO_      )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(FILES_      )->setIsEnabled(false);

        ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();
    }
}


void CSyncForm::OnStnClickedMainBkVideos()
{
    if (lockedUI) {
        return;
    }

    if ( paneVideos.state == STATE_SYNC )
        return;

    if ( paneVideos.state == STATE_PANE_DISABLED ) {
        CString str;
        CString sourceName;
        sourceName.LoadString(IDS_MAIN_VIDEOS);
        str.FormatMessage(IDS_WARNING_PAYMENT_REQUIRED_FOR_SOURCE, sourceName, _T(DATA_PLAN_WEB_PAGE));
        MessageBox((LPCTSTR)str, TEXT(PROGRAM_NAME), MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_SETFOREGROUND);
        return;
    }


    if (checkSyncInProgress()) {
        // It's running a sync -> error msg.
        CString s1;
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
    }
    else {
        // Start Sync of a single source
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(CONTACT_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(TASK_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(NOTE_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(PICTURE_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(FILES_      )->setIsEnabled(false);

        ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();
    }
}


void CSyncForm::OnStnClickedMainBkFiles()
{
    if (lockedUI) {
        return;
    }

    if ( paneFiles.state == STATE_SYNC )
        return;

    if ( paneFiles.state == STATE_PANE_DISABLED ) {
        CString str;
        CString sourceName;
        sourceName.LoadString(IDS_MAIN_FILES);
        str.FormatMessage(IDS_WARNING_PAYMENT_REQUIRED_FOR_SOURCE, sourceName, _T(DATA_PLAN_WEB_PAGE));
        MessageBox((LPCTSTR)str, TEXT(PROGRAM_NAME), MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL | MB_SETFOREGROUND);
        return;
    }


    if (checkSyncInProgress()) {
        // It's running a sync -> error msg.
        CString s1;
        s1.LoadString(IDS_TEXT_SYNC_ALREADY_RUNNING);
        wsafeMessageBox(s1);
    }
    else {
        // Start Sync of a single source
        getConfig()->getSyncSourceConfig(APPOINTMENT_)->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(CONTACT_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(TASK_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(NOTE_       )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(PICTURE_    )->setIsEnabled(false);
        getConfig()->getSyncSourceConfig(VIDEO_      )->setIsEnabled(false);

        ((CMainSyncFrame*)AfxGetMainWnd())->StartSync();
    }
}


void CSyncForm::hideSource(CAnimatedIcon& icon, int iconres, int resource1, 
                           int resource2, CCustomPane& pane) {
    
    icon.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(iconres)));
    icon.ShowWindow(SW_HIDE); 
    GetDlgItem(resource1)->ShowWindow(SW_HIDE);
    GetDlgItem(resource2)->ShowWindow(SW_HIDE);
    pane.ShowWindow(SW_HIDE);
    disableSource(icon, iconres, resource1, resource2, pane);

}
void CSyncForm::disableSource(CAnimatedIcon& icon, int iconres, int resource1, 
                           int resource2, CCustomPane& pane) {
    
    icon.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(iconres)));
    icon.EnableWindow(FALSE); 
    GetDlgItem(resource1)->EnableWindow(FALSE);
    GetDlgItem(resource2)->EnableWindow(FALSE);
    pane.EnableWindow(FALSE);

}



void CSyncForm::refreshSource( int sourceId )
{
    CString s1;

    WindowsSyncSourceConfig * config;

    if (sourceId == SYNCSOURCE_CONTACTS) {
        unsigned long lastSyncContacts=0;
        iconStatusContacts.StopAnim();
        
        if (isSourceVisible(CONTACT)) {
            // source visible
            bool enabled = getConfig()->getSyncSourceConfig(CONTACT_)->isEnabled();
			bool allowed = getConfig()->getSyncSourceConfig(CONTACT_)->isAllowed();
			enabled = enabled & allowed; // allowed management

            GetDlgItem(IDC_MAIN_STATIC_CONTACTS)->EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_CONTACTS)->EnableWindow(enabled);
            if(enabled) {
                paneContacts.ShowWindow(SW_NORMAL);
                paneContacts.state = STATE_NORMAL;
                iconContacts.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CONTACTS)));
            }
            else {
                paneContacts.ShowWindow(SW_HIDE);
                iconContacts.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CONTACTS_GREY)));
            }
            iconContacts.EnableWindow(enabled);
        }
        else {
            // source not visible: we hide the controls
            disableSource(iconContacts, IDI_CONTACTS_GREY, 
                       IDC_MAIN_STATIC_CONTACTS, IDC_MAIN_STATIC_STATUS_CONTACTS,
                       paneContacts);
            /*
            iconContacts.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CONTACTS)));
            iconContacts.ShowWindow(SW_HIDE); 
            GetDlgItem(IDC_MAIN_STATIC_CONTACTS)->ShowWindow(SW_HIDE);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_CONTACTS)->ShowWindow(SW_HIDE);
            paneContacts.ShowWindow(SW_HIDE);
            */
        }

        lastSyncContacts = getConfig()->getSyncSourceConfig(CONTACT_)->getEndTimestamp();


        // check if the last sync failed
        if( syncSourceContactState == SYNCSOURCE_STATE_FAILED){
            s1.LoadString(IDS_MAIN_LAST_SYNC_FAILED);
            iconStatusContacts.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneContacts.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        }
        // check if the last sync canceled
        else if( syncSourceContactState == SYNCSOURCE_STATE_CANCELED){
            s1.LoadString(IDS_MAIN_LAST_SYNC_CANCELED);
            iconStatusContacts.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneContacts.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        }
        else if(lastSyncContacts == 0) {
            s1.LoadString(IDS_NOT_SYNCHRONIZED); 
        }
        else
        {
            CTime timeSyncContacts(lastSyncContacts);
            s1.LoadString(IDS_SYNCHRONIZED); s1+= " ";
            s1 += timeSyncContacts.Format(LAST_SYNC_TIME_FORMAT);
        }
        changeContactsStatus(s1);
        SetDlgItemText(IDC_MAIN_STATIC_CONTACTS, contactsLabel);    // Set directly here, pane could be disabled
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_CONTACTS, s1);
        paneContacts.Invalidate();
    }


    else if (sourceId == SYNCSOURCE_CALENDAR) {
        unsigned long lastSyncCalendar=0;
        iconStatusCalendar.StopAnim();
        
        if (isSourceVisible(APPOINTMENT)) {
            // source visible
            bool enabled = getConfig()->getSyncSourceConfig(APPOINTMENT_)->isEnabled();
   			bool allowed = getConfig()->getSyncSourceConfig(APPOINTMENT_)->isAllowed();
			enabled = enabled & allowed; // allowed management

			iconStatusCalendar.EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_CALENDAR)->EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_CALENDAR)->EnableWindow(enabled);
            if(enabled) {
                paneCalendar.ShowWindow(SW_NORMAL);
                paneCalendar.state = STATE_NORMAL;
                iconCalendar.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CALENDAR)));
            }
            else {
                paneCalendar.ShowWindow(SW_HIDE);
                iconCalendar.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CALENDAR_GREY)));
            }
            iconCalendar.EnableWindow(enabled);
        }
        else {
            // source not visible: we hide the controls
            disableSource(iconCalendar, IDI_CALENDAR_GREY, 
                       IDC_MAIN_STATIC_CALENDAR, IDC_MAIN_STATIC_STATUS_CALENDAR,
                       paneCalendar);
            /*
            iconCalendar.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_CALENDAR)));
            iconCalendar.ShowWindow(SW_HIDE); 
            GetDlgItem(IDC_MAIN_STATIC_CALENDAR)->ShowWindow(SW_HIDE);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_CALENDAR)->ShowWindow(SW_HIDE);
            paneCalendar.ShowWindow(SW_HIDE);
            */
        }


        lastSyncCalendar = 0;
        if (config = getConfig()->getSyncSourceConfig(APPOINTMENT_)) {
            lastSyncCalendar = config->getEndTimestamp();
        }

        // check if the last sync failed
        if(syncSourceCalendarState == SYNCSOURCE_STATE_FAILED){
            s1.LoadString(IDS_MAIN_LAST_SYNC_FAILED);
            iconStatusCalendar.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneCalendar.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        }
        // check if the last sync failed
        else if( syncSourceCalendarState == SYNCSOURCE_STATE_CANCELED){
            s1.LoadString(IDS_MAIN_LAST_SYNC_CANCELED);
            iconStatusCalendar.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneCalendar.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        }
        else if(lastSyncCalendar == 0) {
            s1.LoadString(IDS_NOT_SYNCHRONIZED); 
        }
        else
        {
            CTime timeSyncCalendar(lastSyncCalendar);
            s1.LoadString(IDS_SYNCHRONIZED);s1+= " ";
            s1 += timeSyncCalendar.Format(LAST_SYNC_TIME_FORMAT);
        }
        changeCalendarStatus(s1);
        SetDlgItemText(IDC_MAIN_STATIC_CALENDAR, calendarLabel);    // Set directly here, pane could be disabled
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_CALENDAR, s1);
        paneCalendar.Invalidate();
    }

    else if (sourceId == SYNCSOURCE_TASKS) {
        unsigned long lastSyncTasks=0;
        iconStatusTasks.StopAnim();
        
        if (isSourceVisible(TASK)) {
            // source visible
            bool enabled = getConfig()->getSyncSourceConfig(TASK_)->isEnabled();
			bool allowed = getConfig()->getSyncSourceConfig(TASK_)->isAllowed();
			enabled = enabled & allowed; // allowed management

            GetDlgItem(IDC_MAIN_STATIC_TASKS)->EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_TASKS)->EnableWindow(enabled);
            if(enabled) {
                paneTasks.ShowWindow(SW_NORMAL);
                paneTasks.state = STATE_NORMAL;
                iconTasks.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_TASKS)));
            }
            else {
                paneTasks.ShowWindow(SW_HIDE);
                iconTasks.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_TASKS_GREY)));
            }
            iconTasks.EnableWindow(enabled);
        }
        else {
            // source not visible: we hide the controls
            disableSource(iconTasks, IDI_TASKS_GREY, 
                       IDC_MAIN_STATIC_TASKS, IDC_MAIN_STATIC_STATUS_TASKS,
                       paneTasks);
            /*
            iconTasks.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_TASKS)));
            iconTasks.ShowWindow(SW_HIDE); 
            GetDlgItem(IDC_MAIN_STATIC_TASKS)->ShowWindow(SW_HIDE);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_TASKS)->ShowWindow(SW_HIDE);
            paneTasks.ShowWindow(SW_HIDE);
            */
        }


        lastSyncTasks = 0;
        if (config = getConfig()->getSyncSourceConfig(TASK_)) {
            lastSyncTasks = config->getEndTimestamp();
        }

        // check if the last sync failed
        if(syncSourceTaskState == SYNCSOURCE_STATE_FAILED){
            s1.LoadString(IDS_MAIN_LAST_SYNC_FAILED);
            iconStatusTasks.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneTasks.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        }
        // check if the last sync failed
        else if( syncSourceTaskState == SYNCSOURCE_STATE_CANCELED){
            s1.LoadString(IDS_MAIN_LAST_SYNC_CANCELED);
            iconStatusTasks.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneTasks.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        }
        else if(lastSyncTasks == 0) {
            s1.LoadString(IDS_NOT_SYNCHRONIZED); 
        }
        else
        {
            CTime timeSyncTasks(lastSyncTasks);
            s1.LoadString(IDS_SYNCHRONIZED); s1+= " ";
            s1 += timeSyncTasks.Format(LAST_SYNC_TIME_FORMAT);
        }
        changeTasksStatus(s1);
        SetDlgItemText(IDC_MAIN_STATIC_TASKS, tasksLabel);    // Set directly here, pane could be disabled
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_TASKS, s1);
        paneTasks.Invalidate();
    }


    else if (sourceId == SYNCSOURCE_NOTES) {
        unsigned long lastSyncNotes=0;
        iconStatusNotes.StopAnim();

        if (isSourceVisible(NOTE)) {
            // source visible
            bool enabled = getConfig()->getSyncSourceConfig(NOTE_)->isEnabled();
			bool allowed = getConfig()->getSyncSourceConfig(TASK_)->isAllowed();
			enabled = enabled & allowed; // allowed management

            GetDlgItem(IDC_MAIN_STATIC_NOTES)->EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_NOTES)->EnableWindow(enabled);
            if(enabled) {
                paneNotes.ShowWindow(SW_NORMAL);
                paneNotes.state = STATE_NORMAL;
                iconNotes.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOTES)));
            }
            else {
                paneNotes.ShowWindow(SW_HIDE);
                iconNotes.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOTES_GREY)));
            }
            iconNotes.EnableWindow(enabled);
        }
        else {
            // source not visible: we hide the controls
            disableSource(iconNotes, IDI_NOTES_GREY, 
                       IDC_MAIN_STATIC_NOTES, IDC_MAIN_STATIC_STATUS_NOTES,
                       paneNotes);
            /*
            iconNotes.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_NOTES)));
            iconNotes.ShowWindow(SW_HIDE); 
            GetDlgItem(IDC_MAIN_STATIC_NOTES)->ShowWindow(SW_HIDE);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_NOTES)->ShowWindow(SW_HIDE);
            paneNotes.ShowWindow(SW_HIDE);
            */
        }

        lastSyncNotes = getConfig()->getSyncSourceConfig(NOTE_)->getEndTimestamp();

        // check if the last sync failed
        if(syncSourceNoteState == SYNCSOURCE_STATE_FAILED){
            s1.LoadString(IDS_MAIN_LAST_SYNC_FAILED);
            iconStatusNotes.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneNotes.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        }
        // check if the last sync failed
        else if( syncSourceNoteState == SYNCSOURCE_STATE_CANCELED){
            s1.LoadString(IDS_MAIN_LAST_SYNC_CANCELED);
            iconStatusNotes.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneNotes.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        }
        else if(lastSyncNotes == 0) {
            s1.LoadString(IDS_NOT_SYNCHRONIZED); 
        }
        else
        {
            CTime timeSyncNotes(lastSyncNotes);
            s1.LoadString(IDS_SYNCHRONIZED); s1+= " ";
            s1 += timeSyncNotes.Format(LAST_SYNC_TIME_FORMAT);
        }
        changeNotesStatus(s1);
        SetDlgItemText(IDC_MAIN_STATIC_NOTES, notesLabel);    // Set directly here, pane could be disabled
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_NOTES, s1);
        paneNotes.Invalidate();
    }


    else if (sourceId == SYNCSOURCE_PICTURES) {
        
        unsigned long lastSyncPictures = 0;
        iconStatusPictures.StopAnim();

        if (isSourceVisible(PICTURE)) {
            // source visible
            GetDlgItem(IDC_MAIN_STATIC_PICTURES)->ShowWindow(SW_NORMAL);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_PICTURES)->ShowWindow(SW_NORMAL);
            iconPictures.ShowWindow(SW_NORMAL); 

            WindowsSyncSourceConfig* ssc = getConfig()->getSyncSourceConfig(PICTURE_);
            if (!ssc) {
                printLog("configuration not found for source picture", "ERROR");
                // TODO: use string resources
                wsafeMessageBox(L"Configuration error: please reinstall the application.");
                exit(1);
                return;
            }

            bool enabled = getConfig()->getSyncSourceConfig(PICTURE_)->isEnabled() || getConfig()->getSyncSourceConfig(PICTURE_)->getIsRefreshMode();
			bool allowed = getConfig()->getSyncSourceConfig(PICTURE_)->isAllowed(); // @#@#
            if(enabled) {
                panePictures.ShowWindow(SW_NORMAL);
				if ( allowed ) {
					panePictures.state = STATE_NORMAL;
					panePictures.SetBitmap(::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_LIGHT)));
					iconPictures.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_PICTURES)));
				} else {
					panePictures.state = STATE_PANE_DISABLED; //@#@#
					panePictures.SetBitmap(::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_DISABLED))); // @#@#@# load ghost background
					iconPictures.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_PICTURES_GREY)));
				}   
            } else {
                // not enabled, behaviour is like before
                panePictures.ShowWindow(SW_HIDE);
                panePictures.state = STATE_PANE_DISABLED; //@#@#
                iconPictures.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_PICTURES_GREY)));
            }

			
            iconPictures.EnableWindow(allowed);
            GetDlgItem(IDC_MAIN_STATIC_PICTURES)->EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_PICTURES)->EnableWindow(enabled);

        }
        else {
            // source not visible: hide the controls
            disableSource(iconPictures, IDI_PICTURES_GREY, 
                       IDC_MAIN_STATIC_PICTURES, IDC_MAIN_STATIC_STATUS_PICTURES,
                       panePictures);
            /*
            iconPictures.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_PICTURES)));
            iconPictures.ShowWindow(SW_HIDE); 
            GetDlgItem(IDC_MAIN_STATIC_PICTURES)->ShowWindow(SW_HIDE);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_PICTURES)->ShowWindow(SW_HIDE);
            panePictures.ShowWindow(SW_HIDE);
            */
        }

        lastSyncPictures = 0;
        if (config = getConfig()->getSyncSourceConfig(PICTURE_)) {
            lastSyncPictures = config->getEndTimestamp();
        }

        // set the warning icon
        if (syncSourcePictureState != SYNCSOURCE_STATE_OK) {
            iconStatusPictures.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            panePictures.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        } else {
            panePictures.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_OK));
        }

        // update the UI source status
        switch (syncSourcePictureState) {
            case SYNCSOURCE_STATE_OK:
                if (lastSyncPictures == 0) {
                    s1.LoadString(IDS_NOT_SYNCHRONIZED); 
                } else {
                    CTime timeSyncPictures(lastSyncPictures);
                    s1.LoadString(IDS_SYNCHRONIZED); s1+= " ";
                    s1 += timeSyncPictures.Format(LAST_SYNC_TIME_FORMAT);
                }
                break;
            case SYNCSOURCE_STATE_FAILED:
                s1.LoadString(IDS_MAIN_LAST_SYNC_FAILED);
                break;
            case SYNCSOURCE_STATE_CANCELED:
                s1.LoadString(IDS_MAIN_LAST_SYNC_CANCELED);
                break;
            case SYNCSOURCE_STATE_QUOTA_EXCEEDED:
                s1.LoadString(IDS_STATUS_QUOTA_EXCEEDED);
                break;
            case SYNCSOURCE_STATE_STORAGE_FULL:
                s1.LoadString(IDS_STATUS_STORAGE_FULL);
                break;
            case SYNCSOURCE_STATE_NOT_SUPPORTED:
                s1.LoadString(IDS_SOURCE_NOT_SUPPORTED_BY_SERVER);
                break;                
            default:
                s1 = "";
                break;
        }
        changePicturesStatus(s1);

        SetDlgItemText(IDC_MAIN_STATIC_PICTURES, picturesLabel);    // Set directly here, pane could be disabled
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_PICTURES, s1);
        panePictures.Invalidate();
    }


    else if (sourceId == SYNCSOURCE_VIDEOS) {
        
        unsigned long lastSyncVideos = 0;
        iconStatusVideos.StopAnim();

        if (isSourceVisible(VIDEO)) {
            // source visible
            GetDlgItem(IDC_MAIN_STATIC_VIDEOS)->ShowWindow(SW_NORMAL);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_VIDEOS)->ShowWindow(SW_NORMAL);
            iconVideos.ShowWindow(SW_NORMAL); 

            WindowsSyncSourceConfig* ssc = getConfig()->getSyncSourceConfig(VIDEO_);
            if (!ssc) {
                printLog("configuration not found for source video", "ERROR");
                // TODO: use string resources
                wsafeMessageBox(L"Configuration error: please reinstall the application.");
                exit(1);
                return;
            }

            bool enabled = getConfig()->getSyncSourceConfig(VIDEO_)->isEnabled() || getConfig()->getSyncSourceConfig(VIDEO_)->getIsRefreshMode();
			bool allowed = getConfig()->getSyncSourceConfig(VIDEO_)->isAllowed();
			// enabled = enabled & allowed; // allowed management

			if( enabled ) {
                paneVideos.ShowWindow(SW_NORMAL);
                if ( allowed ) {
                    paneVideos.state = STATE_NORMAL;
                    paneVideos.SetBitmap(::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_LIGHT))); // @#@#
                    iconVideos.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_VIDEOS)));
                } else {
                    paneVideos.state = STATE_PANE_DISABLED; // @#@#
                    paneVideos.SetBitmap(::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_DISABLED)));  // @#@#
                    iconVideos.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_VIDEOS_GREY)));
                }
            } else {
                // standard mode, only not enabled
                paneVideos.ShowWindow(SW_HIDE);
    			paneVideos.state = STATE_PANE_DISABLED; // @#@#
                iconVideos.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_VIDEOS_GREY)));
            }
            iconVideos.EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_VIDEOS)->EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_VIDEOS)->EnableWindow(enabled);
        }
        else {
            // source not visible: hide the controls
            disableSource(iconVideos, IDI_VIDEOS_GREY, 
                       IDC_MAIN_STATIC_VIDEOS, IDC_MAIN_STATIC_STATUS_VIDEOS,
                       paneVideos);
        }

        lastSyncVideos = 0;
        if (config = getConfig()->getSyncSourceConfig(VIDEO_)) {
            lastSyncVideos = config->getEndTimestamp();
        }

        // set the warning icon
        if (syncSourceVideoState != SYNCSOURCE_STATE_OK) {
            iconStatusVideos.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneVideos.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        } else {
            paneVideos.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_OK));
        }

        // update the UI source status
        switch (syncSourceVideoState) {
            case SYNCSOURCE_STATE_OK:
                if (lastSyncVideos == 0) {
                    s1.LoadString(IDS_NOT_SYNCHRONIZED); 
                } else {
                    CTime timeSyncVideos(lastSyncVideos);
                    s1.LoadString(IDS_SYNCHRONIZED); s1+= " ";
                    s1 += timeSyncVideos.Format(LAST_SYNC_TIME_FORMAT);
                }
                break;
            case SYNCSOURCE_STATE_FAILED:
                s1.LoadString(IDS_MAIN_LAST_SYNC_FAILED);
                break;
            case SYNCSOURCE_STATE_CANCELED:
                s1.LoadString(IDS_MAIN_LAST_SYNC_CANCELED);
                break;
            case SYNCSOURCE_STATE_QUOTA_EXCEEDED:
                s1.LoadString(IDS_STATUS_QUOTA_EXCEEDED);
                break;
            case SYNCSOURCE_STATE_STORAGE_FULL:
                s1.LoadString(IDS_STATUS_STORAGE_FULL);
                break;
            case SYNCSOURCE_STATE_NOT_SUPPORTED:
                s1.LoadString(IDS_SOURCE_NOT_SUPPORTED_BY_SERVER);
                break;
            default:
                s1 = "";
                break;
        }
        changeVideosStatus(s1);
        SetDlgItemText(IDC_MAIN_STATIC_VIDEOS, videosLabel);    // Set directly here, pane could be disabled
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_VIDEOS, s1);
        paneVideos.Invalidate();
    }


    else if (sourceId == SYNCSOURCE_FILES) {
        
        unsigned long lastSyncFiles = 0;
        iconStatusFiles.StopAnim();

        if (isSourceVisible(FILES)) {
            // source visible
            GetDlgItem(IDC_MAIN_STATIC_FILES)->ShowWindow(SW_NORMAL);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_FILES)->ShowWindow(SW_NORMAL);
            iconFiles.ShowWindow(SW_NORMAL); 

            WindowsSyncSourceConfig* ssc = getConfig()->getSyncSourceConfig(FILES_);
            if (!ssc) {
                printLog("configuration not found for source files", "ERROR");
                // TODO: use string resources
                wsafeMessageBox(L"Configuration error: please reinstall the application.");
                exit(1);
                return;
            }

            bool enabled = getConfig()->getSyncSourceConfig(FILES_)->isEnabled() || getConfig()->getSyncSourceConfig(FILES_)->getIsRefreshMode();
			bool allowed = getConfig()->getSyncSourceConfig(FILES_)->isAllowed();
            if(enabled) {
                paneFiles.ShowWindow(SW_NORMAL);
                if (allowed) {
                    paneFiles.state = STATE_NORMAL;
                    iconFiles.SetBitmap(::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_LIGHT))); // @#@#
                    iconFiles.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_FILES)));
                } else {
                    paneFiles.state = STATE_PANE_DISABLED;
                    iconFiles.SetBitmap(::LoadBitmap(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_BK_DISABLED))); // @#@#
                    iconFiles.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_FILES_GREY)));
                }
            } else {
                paneFiles.ShowWindow(SW_HIDE);
				paneFiles.state = STATE_PANE_DISABLED;
                iconFiles.SetIcon(::LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_FILES_GREY)));
            }
            iconFiles.EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_FILES)->EnableWindow(enabled);
            GetDlgItem(IDC_MAIN_STATIC_STATUS_FILES)->EnableWindow(enabled);
        }
        else {
            // source not visible: hide the controls
            disableSource(iconFiles, IDI_FILES_GREY, IDC_MAIN_STATIC_FILES, IDC_MAIN_STATIC_STATUS_FILES, paneFiles);
        }

        lastSyncFiles = 0;
        if (config = getConfig()->getSyncSourceConfig(FILES_)) {
            lastSyncFiles = config->getEndTimestamp();
        }

        // set the warning icon
        if (syncSourceFileState != SYNCSOURCE_STATE_OK) {
            iconStatusFiles.SetIcon(LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT)));
            paneFiles.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_ALERT));
        } else {
            paneFiles.hPrevStatusIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDI_OK));
        }

        // update the UI source status
        switch (syncSourceFileState) {
            case SYNCSOURCE_STATE_OK:
                if (lastSyncFiles == 0) {
                    s1.LoadString(IDS_NOT_SYNCHRONIZED); 
                } else {
                    CTime timeSyncFiles(lastSyncFiles);
                    s1.LoadString(IDS_SYNCHRONIZED); s1+= " ";
                    s1 += timeSyncFiles.Format(LAST_SYNC_TIME_FORMAT);
                }
                break;
            case SYNCSOURCE_STATE_FAILED:
                s1.LoadString(IDS_MAIN_LAST_SYNC_FAILED);
                break;
            case SYNCSOURCE_STATE_CANCELED:
                s1.LoadString(IDS_MAIN_LAST_SYNC_CANCELED);
                break;
            case SYNCSOURCE_STATE_QUOTA_EXCEEDED:
                s1.LoadString(IDS_STATUS_QUOTA_EXCEEDED);
                break;
            case SYNCSOURCE_STATE_STORAGE_FULL:
                s1.LoadString(IDS_STATUS_STORAGE_FULL);
                break;
            case SYNCSOURCE_STATE_NOT_SUPPORTED:
                s1.LoadString(IDS_SOURCE_NOT_SUPPORTED_BY_SERVER);
                break;            
            default:
                s1 = "";
                break;
        }
        changeFilesStatus(s1);
        SetDlgItemText(IDC_MAIN_STATIC_FILES, filesLabel);    // Set directly here, pane could be disabled
        SetDlgItemText(IDC_MAIN_STATIC_STATUS_FILES, s1);
        paneFiles.Invalidate();
    }
}



/**
 * Lock UI buttons of main window.
 * Buttons are locked when starting sync, to avoid errors clicking
 * quickly on buttons, and avoid displaying the cancel msg together with
 * the full-sync msg.
 * Buttons are locked when canceling sync.
 */
void CSyncForm::lockButtons() {
    lockedUI = true;
}

/**
 * Unlock UI buttons of main window.
 * Buttons are unlocked after the 'ContinueAfterPrepareSync()' method.
 * Buttons are unlocked when the sync process has finished.
 */
void CSyncForm::unlockButtons() {
    lockedUI = false;
}
