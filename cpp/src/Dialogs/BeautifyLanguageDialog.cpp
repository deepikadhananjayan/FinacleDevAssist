#include "BeautifyLanguageDialog.h"
#include "../PluginDefinition.h"
#include "../Core/FDAApplication.h"
#include "../DockingFeature/resource.h"

INT_PTR CALLBACK BeautifyLanguageDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM)
{
    static HBRUSH hDarkBrush = nullptr;
    static HICON  hIconSmall = nullptr;
    static HICON  hIconBig = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        hIconSmall = static_cast<HICON>(
            LoadImage(
                FDAApplication::getModuleHandle(),
                MAKEINTRESOURCE(IDI_FDA_SMALL),
                IMAGE_ICON,
                16, 16,
                LR_DEFAULTCOLOR
            )
            );

        hIconBig = static_cast<HICON>(
            LoadImage(
                FDAApplication::getModuleHandle(),
                MAKEINTRESOURCE(IDI_FDA_SMALL),
                IMAGE_ICON,
                32, 32,
                LR_DEFAULTCOLOR
            )
            );

        if (hIconSmall)
            SendMessage(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));

        if (hIconBig)
            SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconBig));

        // Center dialog over Notepad++
        HWND parent = nppData._nppHandle;
        RECT parentRect, dialogRect;

        GetWindowRect(parent, &parentRect);
        GetWindowRect(hDlg, &dialogRect);

        int dialogWidth = dialogRect.right - dialogRect.left;
        int dialogHeight = dialogRect.bottom - dialogRect.top;
        int parentWidth = parentRect.right - parentRect.left;
        int parentHeight = parentRect.bottom - parentRect.top;

        int x = parentRect.left + (parentWidth - dialogWidth) / 2;
        int y = parentRect.top + (parentHeight - dialogHeight) / 2;

        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // Populate combo box
        HWND comboBox = GetDlgItem(hDlg, IDC_BEAUTIFY_LANGUAGE);

        SendMessage(comboBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Finacle Script")));
        SendMessage(comboBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("JavaScript")));
        SendMessage(comboBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("XML")));
        SendMessage(comboBox, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Java")));

        SendMessage(comboBox, CB_SETCURSEL, 0, 0);

        // Create dark brush if dark mode is active
        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));

        // Apply Notepad++ dark mode theming
        ::SendMessage(
            nppData._nppHandle,
            NPPM_DARKMODESUBCLASSANDTHEME,
            static_cast<WPARAM>(NppDarkMode::dmfInit),
            reinterpret_cast<LPARAM>(hDlg)
        );

        return TRUE;
    }

    case WM_SETTINGCHANGE:
    {
        // Recreate brush on theme switch
        if (hDarkBrush)
        {
            DeleteObject(hDarkBrush);
            hDarkBrush = nullptr;
        }

        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));

        // Follow theme changes at runtime
        ::SendMessage(
            nppData._nppHandle,
            NPPM_DARKMODESUBCLASSANDTHEME,
            static_cast<WPARAM>(NppDarkMode::dmfHandleChange),
            reinterpret_cast<LPARAM>(hDlg)
        );
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
            HWND comboBox = GetDlgItem(hDlg, IDC_BEAUTIFY_LANGUAGE);

            int selectedIndex = static_cast<int>(
                SendMessage(comboBox, CB_GETCURSEL, 0, 0)
                );

            if (selectedIndex == CB_ERR)
            {
                MessageBox(
                    hDlg,
                    TEXT("Please select a language."),
                    TEXT("Finacle Dev Assist"),
                    MB_OK | MB_ICONWARNING
                );
                return TRUE;
            }

            EndDialog(hDlg, selectedIndex + 1);
            return TRUE;
        }

        case IDCANCEL:
        {
            EndDialog(hDlg, 0);
            return TRUE;
        }
        }
        break;
    }

    case WM_DESTROY:
    {
        if (hDarkBrush) { DeleteObject(hDarkBrush); hDarkBrush = nullptr; }
        if (hIconSmall) { DestroyIcon(hIconSmall);  hIconSmall = nullptr; }
        if (hIconBig) { DestroyIcon(hIconBig);    hIconBig = nullptr; }
        break;
    }
    }

    return FALSE;
}