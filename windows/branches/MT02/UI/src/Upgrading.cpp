// ..\src\Upgrading.cpp : implementation file
//

#include "stdafx.h"
#include "OutlookPlugin.h"
#include "Upgrading.h"


// CUpgrading dialog

IMPLEMENT_DYNAMIC(CUpgrading, CDialog)

CUpgrading::CUpgrading(CWnd* pParent /*=NULL*/)
    : CDialog(CUpgrading::IDD, pParent)
{
}

CUpgrading::~CUpgrading()
{
}

void CUpgrading::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BOOL CUpgrading::OnInitDialog(){
    CString s1;
    s1.LoadString(IDS_UPGRADING); SetWindowText(s1);

    return FALSE;
}

BEGIN_MESSAGE_MAP(CUpgrading, CDialog)
END_MESSAGE_MAP()


// CUpgrading message handlers
