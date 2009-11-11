#pragma once


// CUpgrading dialog

class CUpgrading : public CDialog
{
    DECLARE_DYNAMIC(CUpgrading)

public:
    CUpgrading(CWnd* pParent = NULL);   // standard constructor
    virtual ~CUpgrading();

// Dialog Data
    enum { IDD = IDD_UPGRADING };

protected:
    virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

    DECLARE_MESSAGE_MAP()
};
