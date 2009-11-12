// CWelcome message handlers
// Welcome.cpp : implementation file
//

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "Welcome.h"

#include "winmaincpp.h"
#include "utils.h"

// CWelcome dialog

IMPLEMENT_DYNAMIC(CWelcome, CDialog)

CWelcome::CWelcome(CWnd* pParent /*=NULL*/)
	: CDialog(CWelcome::IDD, pParent)
{
}

CWelcome::~CWelcome()
{
}

void CWelcome::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BOOL CWelcome::OnInitDialog(){
    CString s1;

    s1.LoadString(IDS_WELCOME); SetWindowText(s1);

    OutlookConfig * config = OutlookConfig::getInstance();
    s1.LoadString(IDS_WELCOME_MESSAGE);
    SetDlgItemText(IDC_WELCOME_MESSAGE, s1);
    CDialog::OnInitDialog();

    return TRUE;
}

BEGIN_MESSAGE_MAP(CWelcome, CDialog)
END_MESSAGE_MAP()


// CWelcome message handlers
