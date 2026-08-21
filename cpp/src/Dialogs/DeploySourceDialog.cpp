#include "DeploySourceDialog.h"
#include "../DockingFeature/resource.h"
#include "../PluginDefinition.h"
#include "../Core/FDAApplication.h"
#include "../Utils/StringConvert.h"
#include <string>
#include <vector>
#include <commctrl.h>
#include <shobjidl.h>
#pragma comment(lib, "ole32.lib")

INT_PTR CALLBACK DeployEnvSelectDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static HBRUSH                hDarkBrush = nullptr;
    static DeployEnvSelectParams* pParams = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        pParams = reinterpret_cast<DeployEnvSelectParams*>(lParam);

        HICON hIconSmall = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
        HICON hIconBig = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
        if (hIconSmall) SendMessage(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
        if (hIconBig)   SendMessage(hDlg, WM_SETICON, ICON_BIG,   reinterpret_cast<LPARAM>(hIconBig));

        RECT pr, dr;
        GetWindowRect(nppData._nppHandle, &pr);
        GetWindowRect(hDlg, &dr);
        int x = pr.left + (pr.right  - pr.left  - (dr.right  - dr.left))  / 2;
        int y = pr.top  + (pr.bottom - pr.top   - (dr.bottom - dr.top))   / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        HWND hCombo = GetDlgItem(hDlg, IDC_DEPLOY_ENV_LIST);
        if (pParams)
        {
            for (const std::string& name : pParams->environmentNames)
            {
                std::wstring label = L"c24." + NarrowToWide(name);
                SendMessage(hCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
            }
            if (!pParams->environmentNames.empty())
                SendMessage(hCombo, CB_SETCURSEL, 0, 0);
        }

        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));
        ::SendMessage(nppData._nppHandle,
            NPPM_DARKMODESUBCLASSANDTHEME,
            static_cast<WPARAM>(NppDarkMode::dmfInit),
            reinterpret_cast<LPARAM>(hDlg));

        return TRUE;
    }

    case WM_SETTINGCHANGE:
    {
        if (hDarkBrush) { DeleteObject(hDarkBrush); hDarkBrush = nullptr; }
        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));
        ::SendMessage(nppData._nppHandle,
            NPPM_DARKMODESUBCLASSANDTHEME,
            static_cast<WPARAM>(NppDarkMode::dmfHandleChange),
            reinterpret_cast<LPARAM>(hDlg));
        InvalidateRect(hDlg, nullptr, TRUE);
        break;
    }

    case WM_CTLCOLORDLG:
    case WM_CTLCOLORSTATIC:
    {
        if (hDarkBrush)
        {
            HDC hdc = reinterpret_cast<HDC>(wParam);
            SetTextColor(hdc, RGB(220, 220, 220));
            SetBkColor(hdc, RGB(37, 37, 38));
            return reinterpret_cast<INT_PTR>(hDarkBrush);
        }
        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            if (!pParams) { EndDialog(hDlg, IDCANCEL); return TRUE; }

            HWND hCombo = GetDlgItem(hDlg, IDC_DEPLOY_ENV_LIST);
            int sel = static_cast<int>(SendMessage(hCombo, CB_GETCURSEL, 0, 0));
            if (sel == CB_ERR)
            {
                MessageBox(hDlg, TEXT("Please select an environment."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (sel >= 0 && sel < static_cast<int>(pParams->environmentNames.size()))
                pParams->selectedEnvironment = pParams->environmentNames[sel];

            EndDialog(hDlg, IDOK);
            return TRUE;
        }

        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    case WM_DESTROY:
    {
        if (hDarkBrush) { DeleteObject(hDarkBrush); hDarkBrush = nullptr; }
        break;
    }
    }

    return FALSE;
}

std::string browserForFolder(HWND hParent)
{
    std::string result;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool needsUninit = (hr == S_OK || hr == S_FALSE);

    IFileOpenDialog* pDialog = nullptr;
    hr = CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
        IID_PPV_ARGS(&pDialog));

    if (SUCCEEDED(hr))
    {
        DWORD options = 0;
        pDialog->GetOptions(&options);
        pDialog->SetOptions(options | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        pDialog->SetTitle(TEXT("Select Deployment Source Folder"));

        hr = pDialog->Show(hParent);
        if (SUCCEEDED(hr))
        {
            IShellItem* pItem = nullptr;
            hr = pDialog->GetResult(&pItem);
            if (SUCCEEDED(hr))
            {
                PWSTR pszPath = nullptr;
                hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszPath);
                if (SUCCEEDED(hr))
                {
                    result = WideToNarrow(pszPath);
                    CoTaskMemFree(pszPath);
                }
                pItem->Release();
            }
        }
        // hr == HRESULT_FROM_WIN32(ERROR_CANCELLED) here just means the
        // user cancelled the dialog — result stays empty, no error to show.

        pDialog->Release();
    }

    if (needsUninit)
        CoUninitialize();

    return result;
}