#include "../DockingFeature/resource.h"
#include "../PluginDefinition.h"
#include "../Core/FDAApplication.h"
#include "../Utils/StringConvert.h"
#include "OptionDialog.h"
#include <string>
#include <windows.h>
#include <commctrl.h>

INT_PTR CALLBACK OptionEditDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static HBRUSH            hDarkBrush = nullptr;
    static OptionEditParams* pParams = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        pParams = reinterpret_cast<OptionEditParams*>(lParam);

        HICON hIconSmall = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
        HICON hIconBig = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
        if (hIconSmall) SendMessage(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
        if (hIconBig)   SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconBig));

        RECT pr, dr;
        GetWindowRect(GetParent(hDlg), &pr);
        GetWindowRect(hDlg, &dr);
        int x = pr.left + (pr.right - pr.left - (dr.right - dr.left)) / 2;
        int y = pr.top + (pr.bottom - pr.top - (dr.bottom - dr.top)) / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        if (pParams)
        {
            SetWindowText(GetDlgItem(hDlg, IDC_OPTION_VALUE),
                NarrowToWide(pParams->option.value).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_OPTION_LABEL),
                NarrowToWide(pParams->option.label).c_str());

            SetWindowText(hDlg, pParams->isEditMode ? TEXT("Edit Option") : TEXT("Add Option"));
        }

        SetFocus(GetDlgItem(hDlg, IDC_OPTION_LABEL));

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
            TCHAR valueBuf[128] = {};
            TCHAR labelBuf[128] = {};

            GetWindowText(GetDlgItem(hDlg, IDC_OPTION_VALUE), valueBuf, 128);
            GetWindowText(GetDlgItem(hDlg, IDC_OPTION_LABEL), labelBuf, 128);

            std::string value = WideToNarrow(valueBuf);
            std::string label = WideToNarrow(labelBuf);

            auto trim = [](std::string& s) {
                size_t f = s.find_first_not_of(" \t");
                size_t l = s.find_last_not_of(" \t");
                s = (f == std::string::npos) ? "" : s.substr(f, l - f + 1);
                };
            trim(value);
            trim(label);

            if (value.empty())
            {
                MessageBox(hDlg, TEXT("Value cannot be empty."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_OPTION_VALUE));
                return TRUE;
            }

            if (label.empty())
            {
                MessageBox(hDlg, TEXT("Label cannot be empty."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_OPTION_LABEL));
                return TRUE;
            }

            if (pParams)
            {
                pParams->option.value = value;
                pParams->option.label = label;
            }

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