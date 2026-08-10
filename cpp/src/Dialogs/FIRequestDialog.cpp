#include <string>
#include "FIRequestDialog.h"
#include "../PluginDefinition.h"
#include "../DockingFeature/resource.h"
#include "../Configuration/FDAConfig.h"
#include "../Core/FDAApplication.h"
#include "../Models/FIRequestData.h"

static std::string WideToNarrow(const TCHAR* wide)
{
    if (!wide || wide[0] == L'\0') return {};

    int size = WideCharToMultiByte(
        CP_UTF8, 0,
        wide, -1,
        nullptr, 0,
        nullptr, nullptr
    );

    std::string result(size - 1, '\0');

    WideCharToMultiByte(
        CP_UTF8, 0,
        wide, -1,
        &result[0], size,
        nullptr, nullptr
    );

    return result;
}

INT_PTR CALLBACK FIRequestDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static HBRUSH hDarkBrush = nullptr;
    static HICON  hIconSmall = nullptr;
    static HICON  hIconBig = nullptr;
    static FIRequestData* pReqData = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        pReqData = reinterpret_cast<FIRequestData*>(lParam);

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

        // Center dialog relative to Notepad++
        HWND parent = nppData._nppHandle;
        RECT parentRect, dialogRect;

        GetWindowRect(parent, &parentRect);
        GetWindowRect(hDlg, &dialogRect);

        int dialogWidth  = dialogRect.right  - dialogRect.left;
        int dialogHeight = dialogRect.bottom - dialogRect.top;
        int parentWidth  = parentRect.right  - parentRect.left;
        int parentHeight = parentRect.bottom - parentRect.top;

        int x = parentRect.left + (parentWidth  - dialogWidth)  / 2;
        int y = parentRect.top  + (parentHeight - dialogHeight) / 2;

        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        // --------------------------------------------------
        // Environment
        // --------------------------------------------------
        HWND environmentCombo = GetDlgItem(hDlg, IDC_FI_ENVIRONMENT);

        const std::vector<std::string>& environments = FDAConfig::getFIEnvironments();

        for (const std::string& environment : environments)
        {
            std::string displayName = environment;
            if (displayName.rfind("fi.", 0) == 0)
                displayName = displayName.substr(3);

            // Convert narrow to wide before sending
            int size = MultiByteToWideChar(CP_UTF8, 0, displayName.c_str(), -1, nullptr, 0);
            std::wstring wide(size - 1, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, displayName.c_str(), -1, &wide[0], size);

            SendMessageW(environmentCombo, CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(wide.c_str()));
        }

        if (!environments.empty())
            SendMessage(environmentCombo, CB_SETCURSEL, 0, 0);

        // --------------------------------------------------
        // Method
        // --------------------------------------------------
        HWND methodCombo = GetDlgItem(hDlg, IDC_FI_METHOD);

        SendMessage(methodCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("GET")));
        SendMessage(methodCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("POST")));
        SendMessage(methodCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("PUT")));
        SendMessage(methodCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("DELETE")));
        SendMessage(methodCombo, CB_SETCURSEL, 1, 0); // POST

        // --------------------------------------------------
        // Content-Type
        // --------------------------------------------------
        HWND contentTypeCombo = GetDlgItem(hDlg, IDC_FI_CONTENT_TYPE);

        SendMessage(contentTypeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("text/xml")));
        SendMessage(contentTypeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("application/xml")));
        SendMessage(contentTypeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("application/json")));
        SendMessage(contentTypeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("text/plain")));
        SendMessage(contentTypeCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Custom")));
        SendMessage(contentTypeCombo, CB_SETCURSEL, 0, 0); // text/xml

        // --------------------------------------------------
        // Accept
        // --------------------------------------------------
        HWND acceptCombo = GetDlgItem(hDlg, IDC_FI_ACCEPT);

        SendMessage(acceptCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("*/*")));
        SendMessage(acceptCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("application/xml")));
        SendMessage(acceptCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("application/json")));
        SendMessage(acceptCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("text/xml")));
        SendMessage(acceptCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("text/plain")));
        SendMessage(acceptCombo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("Custom")));
        SendMessage(acceptCombo, CB_SETCURSEL, 0, 0); // */*

        // Force hide custom fields
        ShowWindow(GetDlgItem(hDlg, IDC_FI_CONTENT_TYPE_CUSTOM), SW_HIDE);
        ShowWindow(GetDlgItem(hDlg, IDC_FI_ACCEPT_CUSTOM), SW_HIDE);

        // Dark mode
        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));

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
        if (hDarkBrush) { DeleteObject(hDarkBrush); hDarkBrush = nullptr; }

        if (::SendMessage(nppData._nppHandle, NPPM_ISDARKMODEENABLED, 0, 0))
            hDarkBrush = CreateSolidBrush(RGB(37, 37, 38));

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
        // --------------------------------------------------
        // Content-Type combo selection changed
        // --------------------------------------------------
        if (LOWORD(wParam) == IDC_FI_CONTENT_TYPE && HIWORD(wParam) == CBN_SELCHANGE)
        {
            HWND contentTypeCombo = GetDlgItem(hDlg, IDC_FI_CONTENT_TYPE);
            HWND customEdit       = GetDlgItem(hDlg, IDC_FI_CONTENT_TYPE_CUSTOM);

            int sel = static_cast<int>(SendMessage(contentTypeCombo, CB_GETCURSEL, 0, 0));

            // "Custom" is the last item
            bool isCustom = (sel == SendMessage(contentTypeCombo, CB_GETCOUNT, 0, 0) - 1);

            ShowWindow(customEdit, isCustom ? SW_SHOW : SW_HIDE);

            if (isCustom)
            {
                SetFocus(customEdit);
                SetWindowText(customEdit, TEXT(""));
            }

            return TRUE;
        }

        // --------------------------------------------------
        // Accept combo selection changed
        // --------------------------------------------------
        if (LOWORD(wParam) == IDC_FI_ACCEPT && HIWORD(wParam) == CBN_SELCHANGE)
        {
            HWND acceptCombo = GetDlgItem(hDlg, IDC_FI_ACCEPT);
            HWND customEdit  = GetDlgItem(hDlg, IDC_FI_ACCEPT_CUSTOM);

            int sel = static_cast<int>(SendMessage(acceptCombo, CB_GETCURSEL, 0, 0));

            bool isCustom = (sel == SendMessage(acceptCombo, CB_GETCOUNT, 0, 0) - 1);

            ShowWindow(customEdit, isCustom ? SW_SHOW : SW_HIDE);

            if (isCustom)
            {
                SetFocus(customEdit);
                SetWindowText(customEdit, TEXT(""));
            }

            return TRUE;
        }

        switch (LOWORD(wParam))
        {
        case IDOK:
        {
            HWND environmentCombo  = GetDlgItem(hDlg, IDC_FI_ENVIRONMENT);
            HWND methodCombo       = GetDlgItem(hDlg, IDC_FI_METHOD);
            HWND contentTypeCombo  = GetDlgItem(hDlg, IDC_FI_CONTENT_TYPE);
            HWND acceptCombo       = GetDlgItem(hDlg, IDC_FI_ACCEPT);
            HWND contentTypeCustom = GetDlgItem(hDlg, IDC_FI_CONTENT_TYPE_CUSTOM);
            HWND acceptCustom      = GetDlgItem(hDlg, IDC_FI_ACCEPT_CUSTOM);

            int environmentIndex  = static_cast<int>(SendMessage(environmentCombo, CB_GETCURSEL, 0, 0));
            int methodIndex       = static_cast<int>(SendMessage(methodCombo,       CB_GETCURSEL, 0, 0));
            int contentTypeIndex  = static_cast<int>(SendMessage(contentTypeCombo,  CB_GETCURSEL, 0, 0));
            int acceptIndex       = static_cast<int>(SendMessage(acceptCombo,       CB_GETCURSEL, 0, 0));

            if (environmentIndex == CB_ERR)
            {
                MessageBox(hDlg, TEXT("Please select an environment."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (methodIndex == CB_ERR)
            {
                MessageBox(hDlg, TEXT("Please select a method."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (contentTypeIndex == CB_ERR)
            {
                MessageBox(hDlg, TEXT("Please select a Content-Type."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (acceptIndex == CB_ERR)
            {
                MessageBox(hDlg, TEXT("Please select an Accept type."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            // --------------------------------------------------
            // Resolve Content-Type
            // --------------------------------------------------
            std::string contentType;
            int contentTypeCount = static_cast<int>(
                SendMessage(contentTypeCombo, CB_GETCOUNT, 0, 0));

            if (contentTypeIndex == contentTypeCount - 1) // Custom
            {
                TCHAR buf[256] = {};
                GetWindowText(contentTypeCustom, buf, 256);

                if (buf[0] == L'\0')
                {
                    MessageBox(hDlg, TEXT("Please enter a custom Content-Type."),
                        TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                    SetFocus(contentTypeCustom);
                    return TRUE;
                }

                contentType = WideToNarrow(buf);
            }
            else
            {
                TCHAR buf[256] = {};
                SendMessage(contentTypeCombo, CB_GETLBTEXT, contentTypeIndex,
                    reinterpret_cast<LPARAM>(buf));
                contentType = WideToNarrow(buf);
            }

            // --------------------------------------------------
            // Resolve Accept
            // --------------------------------------------------
            std::string accept;
            int acceptCount = static_cast<int>(
                SendMessage(acceptCombo, CB_GETCOUNT, 0, 0));

            if (acceptIndex == acceptCount - 1) // Custom
            {
                TCHAR buf[256] = {};
                GetWindowText(acceptCustom, buf, 256);

                if (buf[0] == L'\0')
                {
                    MessageBox(hDlg, TEXT("Please enter a custom Accept type."),
                        TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                    SetFocus(acceptCustom);
                    return TRUE;
                }

                accept = WideToNarrow(buf);
            }
            else
            {
                TCHAR buf[256] = {};
                SendMessage(acceptCombo, CB_GETLBTEXT, acceptIndex,
                    reinterpret_cast<LPARAM>(buf));
                accept = WideToNarrow(buf);
            }

            // --------------------------------------------------
            // Resolve Environment
            // --------------------------------------------------
            const std::vector<std::string>& environments = FDAConfig::getFIEnvironments();
            std::string environment = environments[environmentIndex];

            // --------------------------------------------------
            // Resolve Method
            // --------------------------------------------------
            TCHAR methodBuf[32] = {};
            SendMessage(methodCombo, CB_GETLBTEXT, methodIndex,
                reinterpret_cast<LPARAM>(methodBuf));
            std::string method = WideToNarrow(methodBuf);

            if (pReqData)
            {
                pReqData->environment = environments[environmentIndex];
                pReqData->method = method;
                pReqData->contentType = contentType;
                pReqData->accept = accept;
            }

            EndDialog(hDlg, IDOK);
            return TRUE;
        }

        case IDCANCEL:
        {
            EndDialog(hDlg, IDCANCEL);
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