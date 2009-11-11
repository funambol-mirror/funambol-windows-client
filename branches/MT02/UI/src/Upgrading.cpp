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


BEGIN_MESSAGE_MAP(CUpgrading, CDialog)
END_MESSAGE_MAP()


// CUpgrading message handlers
