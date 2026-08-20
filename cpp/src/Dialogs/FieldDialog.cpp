#include "../DockingFeature/resource.h"
#include "../PluginDefinition.h"
#include "../Core/FDAApplication.h"
#include "../Utils/StringConvert.h"
#include "../Models/CustomMenuModel.h"
#include "OptionDialog.h"
#include "FieldDialog.h"
#include <string>
#include <vector>
#include <windows.h>
#include <commctrl.h>

// -------------------------------------------------------
// Helpers
// -------------------------------------------------------
static void PopulateOptionsList(HWND hList, const std::vector<Option>& options)
{
    ListView_DeleteAllItems(hList);

    for (int i = 0; i < static_cast<int>(options.size()); ++i)
    {
        const Option& opt = options[i];

        LVITEM lvi = {};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        std::wstring wValue = NarrowToWide(opt.value);
        lvi.pszText = const_cast<LPWSTR>(wValue.c_str());
        ListView_InsertItem(hList, &lvi);

        std::wstring wLabel = NarrowToWide(opt.label);
        ListView_SetItemText(hList, i, 1, const_cast<LPWSTR>(wLabel.c_str()));
    }
}

static std::string GetComboSelection(HWND hCombo)
{
    int sel = static_cast<int>(SendMessage(hCombo, CB_GETCURSEL, 0, 0));
    if (sel == CB_ERR) return "";

    TCHAR buf[128] = {};
    SendMessage(hCombo, CB_GETLBTEXT, sel, reinterpret_cast<LPARAM>(buf));
    return WideToNarrow(buf);
}

static void SetComboSelection(HWND hCombo, const std::string& value)
{
    if (value.empty())
    {
        SendMessage(hCombo, CB_SETCURSEL, static_cast<WPARAM>(-1), 0);
        return;
    }
    std::wstring wValue = NarrowToWide(value);
    SendMessage(hCombo, CB_SELECTSTRING, static_cast<WPARAM>(-1),
        reinterpret_cast<LPARAM>(wValue.c_str()));
}

static bool IsRadioOrSelect(const std::string& fieldType)
{
    return fieldType == "RADIO" || fieldType == "SELECT";
}

static bool IsTextWithSearcher(const std::string& fieldType)
{
    return fieldType == "TEXTWITHSEARCHER";
}

static void UpdateFieldTypeDependents(HWND hDlg, const std::string& fieldType)
{
    HWND hSearcher = GetDlgItem(hDlg, IDC_FIELD_SEARCHER);
    HWND hOptionsLabel = GetDlgItem(hDlg, IDC_FIELD_OPTIONS_LABEL);
    HWND hOptionsList = GetDlgItem(hDlg, IDC_FIELD_OPTIONS_LIST);
    HWND hOptionsAdd = GetDlgItem(hDlg, IDC_FIELD_OPTIONS_ADD);
    HWND hOptionsEdit = GetDlgItem(hDlg, IDC_FIELD_OPTIONS_EDIT);
    HWND hOptionsDel = GetDlgItem(hDlg, IDC_FIELD_OPTIONS_DELETE);

    bool searcherApplicable = IsTextWithSearcher(fieldType);
    EnableWindow(hSearcher, searcherApplicable ? TRUE : FALSE);
    if (!searcherApplicable)
    {
        SendMessage(hSearcher, CB_SETCURSEL, static_cast<WPARAM>(-1), 0);
    }
    else if (SendMessage(hSearcher, CB_GETCURSEL, 0, 0) == CB_ERR)
    {
        SendMessage(hSearcher, CB_SETCURSEL, 0, 0);
    }

    bool optionsApplicable = IsRadioOrSelect(fieldType);
    int  showFlag = optionsApplicable ? SW_SHOW : SW_HIDE;

    ShowWindow(hOptionsLabel, showFlag);
    ShowWindow(hOptionsList, showFlag);
    ShowWindow(hOptionsAdd, showFlag);
    ShowWindow(hOptionsEdit, showFlag);
    ShowWindow(hOptionsDel, showFlag);

    EnableWindow(hOptionsList, optionsApplicable ? TRUE : FALSE);
    EnableWindow(hOptionsAdd, optionsApplicable ? TRUE : FALSE);

    if (!optionsApplicable)
    {
        EnableWindow(hOptionsEdit, FALSE);
        EnableWindow(hOptionsDel, FALSE);
    }
}

// -------------------------------------------------------
// Add Field dialog
// -------------------------------------------------------
INT_PTR CALLBACK FieldEditDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static HBRUSH           hDarkBrush = nullptr;
    static FieldEditParams* pParams = nullptr;
    static std::string      currentFieldType;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        pParams = reinterpret_cast<FieldEditParams*>(lParam);

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

        HWND hPlacement = GetDlgItem(hDlg, IDC_FIELD_PLACEMENT);
        HWND hType = GetDlgItem(hDlg, IDC_FIELD_TYPE);
        HWND hSearcher = GetDlgItem(hDlg, IDC_FIELD_SEARCHER);
        HWND hOptList = GetDlgItem(hDlg, IDC_FIELD_OPTIONS_LIST);

        if (pParams)
        {
            for (const std::string& p : pParams->fieldPlacementOptions)
                SendMessage(hPlacement, CB_ADDSTRING, 0,
                    reinterpret_cast<LPARAM>(NarrowToWide(p).c_str()));

            for (const std::string& t : pParams->fieldTypeOptions)
                SendMessage(hType, CB_ADDSTRING, 0,
                    reinterpret_cast<LPARAM>(NarrowToWide(t).c_str()));
            if (pParams->showFileType)
                SendMessage(hType, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(TEXT("FILE")));

            for (const std::string& s : pParams->searcherOptions)
                SendMessage(hSearcher, CB_ADDSTRING, 0,
                    reinterpret_cast<LPARAM>(NarrowToWide(s).c_str()));

            RECT optRect;
            GetClientRect(hOptList, &optRect);
            int optTotalWidth = optRect.right - optRect.left - 4;
            int valueWidth = optTotalWidth / 2;
            int labelWidth = optTotalWidth - valueWidth;

            LVCOLUMN col = {};
            col.mask = LVCF_TEXT | LVCF_WIDTH;
            col.pszText = const_cast<LPWSTR>(L"Value");
            col.cx = valueWidth;
            ListView_InsertColumn(hOptList, 0, &col);
            col.pszText = const_cast<LPWSTR>(L"Label");
            col.cx = labelWidth;
            ListView_InsertColumn(hOptList, 1, &col);
            ListView_SetExtendedListViewStyle(hOptList,
                LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

            // Pre-fill for Edit mode
            std::string initialFieldPlacement = pParams->isEditMode ? pParams->field.fieldPlacement : "CRITERIA";
            std::string initialFieldType = pParams->isEditMode ? pParams->field.fieldType : "TEXT";
            currentFieldType = initialFieldType;

            SetComboSelection(hPlacement, initialFieldPlacement);
            SetComboSelection(hType, initialFieldType);
            SetComboSelection(hSearcher, pParams->field.searcher);

            SetWindowText(GetDlgItem(hDlg, IDC_FIELD_LABEL),
                NarrowToWide(pParams->field.fieldLabel).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_FIELD_ID),
                NarrowToWide(pParams->field.fieldId).c_str());
            SetWindowText(GetDlgItem(hDlg, IDC_FIELD_DESCRIPTION),
                NarrowToWide(pParams->field.fieldDescription).c_str());

            CheckDlgButton(hDlg, IDC_FIELD_IS_DISABLED,
                pParams->field.isDisabled ? BST_CHECKED : BST_UNCHECKED);
            CheckDlgButton(hDlg, IDC_FIELD_IS_MANDATORY,
                pParams->field.isMandatory ? BST_CHECKED : BST_UNCHECKED);

            PopulateOptionsList(hOptList, pParams->field.options);
            UpdateFieldTypeDependents(hDlg, initialFieldType);

            SetWindowText(hDlg, pParams->isEditMode ? TEXT("Edit Field") : TEXT("Add Field"));
        }

        EnableWindow(GetDlgItem(hDlg, IDC_FIELD_OPTIONS_EDIT), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_FIELD_OPTIONS_DELETE), FALSE);

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

    case WM_NOTIFY:
    {
        NMHDR* pNmHdr = reinterpret_cast<NMHDR*>(lParam);

        if (pNmHdr->idFrom == IDC_FIELD_OPTIONS_LIST && pNmHdr->code == LVN_ITEMCHANGED)
        {
            HWND hOptList = GetDlgItem(hDlg, IDC_FIELD_OPTIONS_LIST);
            int  sel = ListView_GetNextItem(hOptList, -1, LVNI_SELECTED);

            std::string currentType = GetComboSelection(GetDlgItem(hDlg, IDC_FIELD_TYPE));
            bool optionsApplicable = IsRadioOrSelect(currentType);

            EnableWindow(GetDlgItem(hDlg, IDC_FIELD_OPTIONS_EDIT),
                (optionsApplicable && sel >= 0) ? TRUE : FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_FIELD_OPTIONS_DELETE),
                (optionsApplicable && sel >= 0) ? TRUE : FALSE);
        }
        break;
    }

    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
        case IDC_FIELD_TYPE:
        {
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                std::string newFieldType = GetComboSelection(GetDlgItem(hDlg, IDC_FIELD_TYPE));

                if (!IsRadioOrSelect(newFieldType) && pParams && !pParams->field.options.empty())
                {
                    int confirmResult = MessageBox(hDlg,
                        TEXT("Changing Field Type will discard the options you've added for this field.\n\nContinue?"),
                        TEXT("Finacle Dev Assist"), MB_YESNO | MB_ICONWARNING);

                    if (confirmResult != IDYES)
                    {
                        SetComboSelection(GetDlgItem(hDlg, IDC_FIELD_TYPE), currentFieldType);
                        return TRUE;
                    }

                    pParams->field.options.clear();
                    PopulateOptionsList(GetDlgItem(hDlg, IDC_FIELD_OPTIONS_LIST), pParams->field.options);
                }

                currentFieldType = newFieldType;
                UpdateFieldTypeDependents(hDlg, newFieldType);
            }
            break;
        }

        case IDC_FIELD_OPTIONS_ADD:
        {
            if (!pParams) return TRUE;

            OptionEditParams optParams;

            INT_PTR result = DialogBoxParam(
                FDAApplication::getModuleHandle(),
                MAKEINTRESOURCE(IDD_FDA_OPTION_EDIT_DIALOG),
                hDlg,
                OptionEditDialogProc,
                reinterpret_cast<LPARAM>(&optParams)
            );

            if (result == IDOK)
            {
                pParams->field.options.push_back(optParams.option);
                PopulateOptionsList(GetDlgItem(hDlg, IDC_FIELD_OPTIONS_LIST), pParams->field.options);
            }
            return TRUE;
        }

        case IDC_FIELD_OPTIONS_EDIT:
        {
            if (!pParams) return TRUE;

            HWND hOptList = GetDlgItem(hDlg, IDC_FIELD_OPTIONS_LIST);
            int  sel = ListView_GetNextItem(hOptList, -1, LVNI_SELECTED);
            if (sel < 0 || sel >= static_cast<int>(pParams->field.options.size())) return TRUE;

            OptionEditParams optParams;
            optParams.option = pParams->field.options[sel];

            INT_PTR result = DialogBoxParam(
                FDAApplication::getModuleHandle(),
                MAKEINTRESOURCE(IDD_FDA_OPTION_EDIT_DIALOG),
                hDlg,
                OptionEditDialogProc,
                reinterpret_cast<LPARAM>(&optParams)
            );

            if (result == IDOK)
            {
                pParams->field.options[sel] = optParams.option;
                PopulateOptionsList(hOptList, pParams->field.options);
            }
            return TRUE;
        }

        case IDC_FIELD_OPTIONS_DELETE:
        {
            if (!pParams) return TRUE;

            HWND hOptList = GetDlgItem(hDlg, IDC_FIELD_OPTIONS_LIST);
            int  sel = ListView_GetNextItem(hOptList, -1, LVNI_SELECTED);
            if (sel < 0 || sel >= static_cast<int>(pParams->field.options.size())) return TRUE;

            pParams->field.options.erase(pParams->field.options.begin() + sel);
            PopulateOptionsList(hOptList, pParams->field.options);
            EnableWindow(GetDlgItem(hDlg, IDC_FIELD_OPTIONS_EDIT), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_FIELD_OPTIONS_DELETE), FALSE);
            return TRUE;
        }

        case IDOK:
        {
            if (!pParams) { EndDialog(hDlg, IDCANCEL); return TRUE; }

            std::string placement = GetComboSelection(GetDlgItem(hDlg, IDC_FIELD_PLACEMENT));
            std::string fieldType = GetComboSelection(GetDlgItem(hDlg, IDC_FIELD_TYPE));

            TCHAR labelBuf[128] = {};
            TCHAR idBuf[128] = {};
            TCHAR descBuf[512] = {};
            GetWindowText(GetDlgItem(hDlg, IDC_FIELD_LABEL), labelBuf, 128);
            GetWindowText(GetDlgItem(hDlg, IDC_FIELD_ID), idBuf, 128);
            GetWindowText(GetDlgItem(hDlg, IDC_FIELD_DESCRIPTION), descBuf, 512);

            std::string fieldLabel = WideToNarrow(labelBuf);
            std::string fieldId = WideToNarrow(idBuf);
            std::string fieldDesc = WideToNarrow(descBuf);

            auto trim = [](std::string& s) {
                size_t f = s.find_first_not_of(" \t");
                size_t l = s.find_last_not_of(" \t");
                s = (f == std::string::npos) ? "" : s.substr(f, l - f + 1);
                };
            trim(fieldLabel);
            trim(fieldId);
            trim(fieldDesc);

            // --- Field-level validation ---
            if (placement.empty())
            {
                MessageBox(hDlg, TEXT("Field Placement must be selected."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (fieldType.empty())
            {
                MessageBox(hDlg, TEXT("Field Type must be selected."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (fieldLabel.empty())
            {
                MessageBox(hDlg, TEXT("Field Label cannot be empty."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_FIELD_LABEL));
                return TRUE;
            }

            if (fieldId.empty())
            {
                MessageBox(hDlg, TEXT("Field ID cannot be empty."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_FIELD_ID));
                return TRUE;
            }

            if (fieldId.find(' ') != std::string::npos)
            {
                MessageBox(hDlg, TEXT("Field ID cannot contain spaces."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_FIELD_ID));
                return TRUE;
            }

            for (const std::string& existingId : pParams->existingFieldIds)
            {
                if (existingId == fieldId)
                {
                    MessageBox(hDlg, TEXT("Field ID already exists.\nChoose a different ID."),
                        TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                    SetFocus(GetDlgItem(hDlg, IDC_FIELD_ID));
                    return TRUE;
                }
            }

            if (fieldType == "FILE" && pParams->fileFieldAlreadyExists)
            {
                MessageBox(hDlg, TEXT("Only one FILE field is allowed for an UPLOAD menu."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (IsRadioOrSelect(fieldType) && pParams->field.options.empty())
            {
                MessageBox(hDlg, TEXT("SELECT and RADIO fields require at least one option.\nUse Add to add one."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            // --- Build the field ---
            pParams->field.fieldPlacement = placement;
            pParams->field.fieldType = fieldType;
            pParams->field.fieldLabel = fieldLabel;
            pParams->field.fieldId = fieldId;
            pParams->field.fieldDescription = fieldDesc;
            pParams->field.searcher = IsTextWithSearcher(fieldType)
                ? GetComboSelection(GetDlgItem(hDlg, IDC_FIELD_SEARCHER))
                : "";
            pParams->field.isDisabled = (IsDlgButtonChecked(hDlg, IDC_FIELD_IS_DISABLED) == BST_CHECKED);
            pParams->field.isMandatory = (IsDlgButtonChecked(hDlg, IDC_FIELD_IS_MANDATORY) == BST_CHECKED);

            if (IsTextWithSearcher(fieldType) && pParams->field.searcher.empty())
            {
                MessageBox(hDlg, TEXT("Searcher must be selected for a TEXTWITHSEARCHER field."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (!IsRadioOrSelect(fieldType))
                pParams->field.options.clear();

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
