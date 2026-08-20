#include "../DockingFeature/resource.h"
#include "../PluginDefinition.h"
#include "../Core/FDAApplication.h"
#include "../Utils/StringConvert.h"
#include "../Utils/Logger.h"
#include "../Models/CustomMenuModel.h"
#include "../Configuration/FDACustomMenuConfig.h"
#include "FieldDialog.h"
#include "CustomMenuDialog.h"
#include <string>
#include <vector>
#include <windows.h>
#include <commctrl.h>

// ------------------------------------------------------ -
// Dirty-flag state
// -------------------------------------------------------
struct CustomMenuState
{
    std::vector<Field> fields;
    bool isDirty = false;
    std::string currentMenuType;
};

// -------------------------------------------------------
// Helpers
// -------------------------------------------------------
static std::string GetComboSelection(HWND hCombo)
{
    int sel = static_cast<int>(SendMessage(hCombo, CB_GETCURSEL, 0, 0));
    if (sel == CB_ERR) return "";
    TCHAR buf[128] = {};
    SendMessage(hCombo, CB_GETLBTEXT, sel, reinterpret_cast<LPARAM>(buf));
    return WideToNarrow(buf);
}

static bool IsDesignAllowedForMenuType(const std::string& menuType, const std::string& design)
{
    if (design == "INC")
        return menuType.rfind("MRH", 0) == 0;

    return true;
}

static bool IsChecked(HWND hList, int item)
{
    UINT state = ListView_GetItemState(hList, item, LVIS_STATEIMAGEMASK);
    return ((state >> 12) - 1) == 1;
}

static std::vector<std::string> GetCheckedDesigns(HWND hList)
{
    std::vector<std::string> result;
    int count = ListView_GetItemCount(hList);
    for (int i = 1; i < count; ++i)
    {
        if (IsChecked(hList, i))
        {
            TCHAR buf[64] = {};
            ListView_GetItemText(hList, i, 0, buf, 64);
            result.push_back(WideToNarrow(buf));
        }
    }
    return result;
}

static bool FileFieldExistsElsewhere(const std::vector<Field>& fields, int excludeIndex)
{
    for (int i = 0; i < static_cast<int>(fields.size()); ++i)
    {
        if (i == excludeIndex) continue;
        if (fields[i].fieldType == "FILE") return true;
    }
    return false;
}

static std::vector<std::string> CollectExistingFieldIds(const std::vector<Field>& fields, int excludeIndex)
{
    std::vector<std::string> ids;
    for (int i = 0; i < static_cast<int>(fields.size()); ++i)
    {
        if (i == excludeIndex) continue;
        ids.push_back(fields[i].fieldId);
    }
    return ids;
}

static void UpdateDesignAvailability(HWND hDlg, const std::string& menuType)
{
    HWND hDesignList = GetDlgItem(hDlg, IDC_MENU_GENERATE_DESIGN_LIST);
    int count = ListView_GetItemCount(hDesignList);

    for (int i = 1; i < count; ++i)
    {
        TCHAR buf[64] = {};
        ListView_GetItemText(hDesignList, i, 0, buf, 64);
        std::string design = WideToNarrow(buf);

        if (!IsDesignAllowedForMenuType(menuType, design))
            ListView_SetItemState(hDesignList, i, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
    }

    if (IsChecked(hDesignList, 0))
        ListView_SetItemState(hDesignList, 0, INDEXTOSTATEIMAGEMASK(1), LVIS_STATEIMAGEMASK);
}

static void PopulateFieldsList(HWND hList, const std::vector<Field>& fields)
{
    ListView_DeleteAllItems(hList);

    for (int i = 0; i < static_cast<int>(fields.size()); ++i)
    {
        const Field& f = fields[i];

        std::wstring idxStr = std::to_wstring(i + 1);
        LVITEM lvi = {};
        lvi.mask = LVIF_TEXT;
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.pszText = const_cast<LPWSTR>(idxStr.c_str());
        ListView_InsertItem(hList, &lvi);

        std::wstring wFieldId = NarrowToWide(f.fieldId);
        std::wstring wFieldType = NarrowToWide(f.fieldType);
        std::wstring wFieldPlacement = NarrowToWide(f.fieldPlacement);

        ListView_SetItemText(hList, i, 1, const_cast<LPWSTR>(wFieldId.c_str()));
        ListView_SetItemText(hList, i, 2, const_cast<LPWSTR>(wFieldType.c_str()));
        ListView_SetItemText(hList, i, 3, const_cast<LPWSTR>(wFieldPlacement.c_str()));
        ListView_SetItemText(hList, i, 4, const_cast<LPWSTR>((f.isMandatory ? L"Yes" : L"No")));
        ListView_SetItemText(hList, i, 5, const_cast<LPWSTR>((f.isDisabled ? L"Yes" : L"No")));
    }
}

// -------------------------------------------------------
// Main dialog
// -------------------------------------------------------
INT_PTR CALLBACK CustomMenuDialogProc(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LPARAM lParam)
{
    static HBRUSH          hDarkBrush = nullptr;
    static CustomMenuState state;
    static CustomMenuData* pMenu = nullptr;

    switch (message)
    {
    case WM_INITDIALOG:
    {
        state = CustomMenuState();
        pMenu = reinterpret_cast<CustomMenuData*>(lParam);

        HICON hIconSmall = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR));
        HICON hIconBig = static_cast<HICON>(LoadImage(
            FDAApplication::getModuleHandle(),
            MAKEINTRESOURCE(IDI_FDA_SMALL), IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR));
        if (hIconSmall) SendMessage(hDlg, WM_SETICON, ICON_SMALL, reinterpret_cast<LPARAM>(hIconSmall));
        if (hIconBig)   SendMessage(hDlg, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIconBig));

        RECT pr, dr;
        GetWindowRect(nppData._nppHandle, &pr);
        GetWindowRect(hDlg, &dr);
        int x = pr.left + (pr.right - pr.left - (dr.right - dr.left)) / 2;
        int y = pr.top + (pr.bottom - pr.top - (dr.bottom - dr.top)) / 2;
        SetWindowPos(hDlg, nullptr, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

        if (!FDACustomMenuConfig::load())
        {
            MessageBox(hDlg, TEXT("Custom menu configuration file not found.\n\nPlease reinstall the plugin and try again."),
                TEXT("Finacle Dev Assist"), MB_OK | MB_ICONERROR);
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }

        HWND hMenuType = GetDlgItem(hDlg, IDC_MENU_TYPE);
        for (const std::string& t : FDACustomMenuConfig::getMenuTypes())
            SendMessage(hMenuType, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(NarrowToWide(t).c_str()));

        SendMessage(hMenuType, CB_SELECTSTRING, static_cast<WPARAM>(-1), reinterpret_cast<LPARAM>(TEXT("THREE_PAGE")));

        HWND hDesignList = GetDlgItem(hDlg, IDC_MENU_GENERATE_DESIGN_LIST);
        ListView_SetExtendedListViewStyle(hDesignList,
            LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES | LVS_EX_GRIDLINES);
        RECT designRect;
        GetClientRect(hDesignList, &designRect);
        LVCOLUMN designCol = {};
        designCol.mask = LVCF_TEXT | LVCF_WIDTH;
        designCol.pszText = const_cast<LPWSTR>(L"Generate Design");
        designCol.cx = designRect.right - designRect.left - 4;
        ListView_InsertColumn(hDesignList, 0, &designCol);

        {
            LVITEM lvi = {};
            lvi.mask = LVIF_TEXT;
            lvi.iItem = 0;
            lvi.pszText = const_cast<LPWSTR>(L"ALL");
            ListView_InsertItem(hDesignList, &lvi);
        }
        int di = 1;
        for (const std::string& d : FDACustomMenuConfig::getGenerateDesigns())
        {
            LVITEM lvi = {};
            lvi.mask = LVIF_TEXT;
            lvi.iItem = di;
            std::wstring wd = NarrowToWide(d);
            lvi.pszText = const_cast<LPWSTR>(wd.c_str());
            ListView_InsertItem(hDesignList, &lvi);
            ++di;
        }

        HWND hFieldsList = GetDlgItem(hDlg, IDC_MENU_FIELDS_LIST);
        ListView_SetExtendedListViewStyle(hFieldsList,
            LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        RECT fieldsRect;
        GetClientRect(hFieldsList, &fieldsRect);
        int fieldsTotalWidth = fieldsRect.right - fieldsRect.left - 4;

        int colNum = static_cast<int>(fieldsTotalWidth * 0.06);
        int colId = static_cast<int>(fieldsTotalWidth * 0.27);
        int colType = static_cast<int>(fieldsTotalWidth * 0.20);
        int colPlacement = static_cast<int>(fieldsTotalWidth * 0.20);
        int colMandatory = static_cast<int>(fieldsTotalWidth * 0.14);
        int colDisabled = fieldsTotalWidth - colNum - colId - colType - colPlacement - colMandatory;

        struct { LPCWSTR text; int width; } fieldsCols[] = {
            { L"#",         colNum       },
            { L"Field ID",  colId        },
            { L"Type",      colType      },
            { L"Placement", colPlacement },
            { L"Mandatory", colMandatory },
            { L"Disabled",  colDisabled  },
        };
        for (int c = 0; c < 6; ++c)
        {
            LVCOLUMN col = {};
            col.mask = LVCF_TEXT | LVCF_WIDTH;
            col.pszText = const_cast<LPWSTR>(fieldsCols[c].text);
            col.cx = fieldsCols[c].width;
            ListView_InsertColumn(hFieldsList, c, &col);
        }

        EnableWindow(GetDlgItem(hDlg, IDC_MENU_EDIT_FIELD), FALSE);
        EnableWindow(GetDlgItem(hDlg, IDC_MENU_DELETE_FIELD), FALSE);

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

        if (pNmHdr->idFrom == IDC_MENU_FIELDS_LIST && pNmHdr->code == LVN_ITEMCHANGED)
        {
            HWND hList = GetDlgItem(hDlg, IDC_MENU_FIELDS_LIST);
            int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
            EnableWindow(GetDlgItem(hDlg, IDC_MENU_EDIT_FIELD), sel >= 0 ? TRUE : FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_MENU_DELETE_FIELD), sel >= 0 ? TRUE : FALSE);
        }

        if (pNmHdr->idFrom == IDC_MENU_GENERATE_DESIGN_LIST)
        {
            static bool suppressDesignNotify = false;

            if (pNmHdr->code == LVN_ITEMCHANGING && !suppressDesignNotify)
            {
                NMLISTVIEW* pLv = reinterpret_cast<NMLISTVIEW*>(lParam);
                if ((pLv->uChanged & LVIF_STATE) &&
                    ((pLv->uOldState & LVIS_STATEIMAGEMASK) != (pLv->uNewState & LVIS_STATEIMAGEMASK)))
                {
                    HWND hDesignList = GetDlgItem(hDlg, IDC_MENU_GENERATE_DESIGN_LIST);
                    bool allChecked = IsChecked(hDesignList, 0);

                    if (pLv->iItem != 0)
                    {
                        TCHAR buf[64] = {};
                        ListView_GetItemText(hDesignList, pLv->iItem, 0, buf, 64);
                        std::string design = WideToNarrow(buf);
                        if (!IsDesignAllowedForMenuType(state.currentMenuType, design))
                        {
                            SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                            return TRUE;
                        }
                    }

                    if (allChecked && pLv->iItem != 0)
                    {
                        SetWindowLongPtr(hDlg, DWLP_MSGRESULT, TRUE);
                        return TRUE;
                    }
                }
            }

            if (pNmHdr->code == LVN_ITEMCHANGED && !suppressDesignNotify)
            {
                NMLISTVIEW* pLv = reinterpret_cast<NMLISTVIEW*>(lParam);
                if ((pLv->uChanged & LVIF_STATE) &&
                    ((pLv->uOldState & LVIS_STATEIMAGEMASK) != (pLv->uNewState & LVIS_STATEIMAGEMASK)))
                {
                    state.isDirty = true;

                    HWND hDesignList = GetDlgItem(hDlg, IDC_MENU_GENERATE_DESIGN_LIST);

                    if (pLv->iItem == 0)
                    {
                        bool allChecked = IsChecked(hDesignList, 0);
                        suppressDesignNotify = true;
                        int count = ListView_GetItemCount(hDesignList);
                        for (int i = 1; i < count; ++i)
                        {
                            TCHAR buf[64] = {};
                            ListView_GetItemText(hDesignList, i, 0, buf, 64);
                            std::string design = WideToNarrow(buf);

                            if (allChecked && !IsDesignAllowedForMenuType(state.currentMenuType, design))
                                continue;

                            ListView_SetItemState(hDesignList, i,
                                INDEXTOSTATEIMAGEMASK(allChecked ? 2 : 1), LVIS_STATEIMAGEMASK);
                        }
                        suppressDesignNotify = false;
                    }
                }
            }
        }
        break;
    }

    case WM_COMMAND:
    {
        WORD id = LOWORD(wParam);

        if ((id == IDC_MENU_NAME || id == IDC_MENU_DESCRIPTION) && HIWORD(wParam) == EN_CHANGE)
        {
            state.isDirty = true;
        }

        switch (id)
        {
        case IDC_MENU_TYPE:
        {
            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                state.isDirty = true;
                state.currentMenuType = GetComboSelection(GetDlgItem(hDlg, IDC_MENU_TYPE));
                UpdateDesignAvailability(hDlg, state.currentMenuType);
                SetFocus(GetDlgItem(hDlg, IDC_MENU_GENERATE_DESIGN_LIST));
            }
            return TRUE;
        }

        case IDC_MENU_ADD_FIELD:
        {
            std::string menuType = GetComboSelection(GetDlgItem(hDlg, IDC_MENU_TYPE));

            FieldEditParams params;
            params.isEditMode = false;
            params.fieldPlacementOptions = FDACustomMenuConfig::getFieldPlacements();
            params.fieldTypeOptions = FDACustomMenuConfig::getFieldTypes();
            params.searcherOptions = FDACustomMenuConfig::getSearchers();
            params.showFileType = (menuType == "UPLOAD");
            params.fileFieldAlreadyExists = FileFieldExistsElsewhere(state.fields, -1);
            params.existingFieldIds = CollectExistingFieldIds(state.fields, -1);

            INT_PTR result = DialogBoxParam(
                FDAApplication::getModuleHandle(),
                MAKEINTRESOURCE(IDD_FDA_FIELD_EDIT_DIALOG),
                hDlg,
                FieldEditDialogProc,
                reinterpret_cast<LPARAM>(&params)
            );

            if (result == IDOK)
            {
                state.fields.push_back(params.field);
                state.isDirty = true;
                PopulateFieldsList(GetDlgItem(hDlg, IDC_MENU_FIELDS_LIST), state.fields);
            }
            return TRUE;
        }

        case IDC_MENU_EDIT_FIELD:
        {
            HWND hList = GetDlgItem(hDlg, IDC_MENU_FIELDS_LIST);
            int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
            if (sel < 0 || sel >= static_cast<int>(state.fields.size())) return TRUE;

            std::string menuType = GetComboSelection(GetDlgItem(hDlg, IDC_MENU_TYPE));

            FieldEditParams params;
            params.isEditMode = true;
            params.field = state.fields[sel];
            params.fieldPlacementOptions = FDACustomMenuConfig::getFieldPlacements();
            params.fieldTypeOptions = FDACustomMenuConfig::getFieldTypes();
            params.searcherOptions = FDACustomMenuConfig::getSearchers();
            params.showFileType = (menuType == "UPLOAD");
            params.fileFieldAlreadyExists = FileFieldExistsElsewhere(state.fields, sel);
            params.existingFieldIds = CollectExistingFieldIds(state.fields, sel);

            INT_PTR result = DialogBoxParam(
                FDAApplication::getModuleHandle(),
                MAKEINTRESOURCE(IDD_FDA_FIELD_EDIT_DIALOG),
                hDlg,
                FieldEditDialogProc,
                reinterpret_cast<LPARAM>(&params)
            );

            if (result == IDOK)
            {
                state.fields[sel] = params.field;
                state.isDirty = true;
                PopulateFieldsList(hList, state.fields);
            }
            return TRUE;
        }

        case IDC_MENU_DELETE_FIELD:
        {
            HWND hList = GetDlgItem(hDlg, IDC_MENU_FIELDS_LIST);
            int sel = ListView_GetNextItem(hList, -1, LVNI_SELECTED);
            if (sel < 0 || sel >= static_cast<int>(state.fields.size())) return TRUE;

            state.fields.erase(state.fields.begin() + sel);
            state.isDirty = true;
            PopulateFieldsList(hList, state.fields);
            EnableWindow(GetDlgItem(hDlg, IDC_MENU_EDIT_FIELD), FALSE);
            EnableWindow(GetDlgItem(hDlg, IDC_MENU_DELETE_FIELD), FALSE);
            return TRUE;
        }

        case IDC_MENU_GENERATE:
        {
            TCHAR nameBuf[64] = {};
            TCHAR descBuf[128] = {};
            GetWindowText(GetDlgItem(hDlg, IDC_MENU_NAME), nameBuf, 64);
            GetWindowText(GetDlgItem(hDlg, IDC_MENU_DESCRIPTION), descBuf, 128);

            std::string menuName = WideToNarrow(nameBuf);
            std::string menuDesc = WideToNarrow(descBuf);
            std::string menuType = GetComboSelection(GetDlgItem(hDlg, IDC_MENU_TYPE));

            auto trim = [](std::string& s) {
                size_t f = s.find_first_not_of(" \t");
                size_t l = s.find_last_not_of(" \t");
                s = (f == std::string::npos) ? "" : s.substr(f, l - f + 1);
                };
            trim(menuName);
            trim(menuDesc);

            // --- Final validation ---
            if (menuName.empty())
            {
                MessageBox(hDlg, TEXT("Menu Name cannot be empty."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_MENU_NAME));
                return TRUE;
            }

            if (menuName.length() < 5 || menuName.length() > 9)
            {
                MessageBox(hDlg, TEXT("Menu Name must be between 5 and 9 characters."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_MENU_NAME));
                return TRUE;
            }

            if (menuDesc.empty())
            {
                MessageBox(hDlg, TEXT("Menu Description cannot be empty."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_MENU_DESCRIPTION));
                return TRUE;
            }

            if (menuDesc.length() > 40)
            {
                MessageBox(hDlg, TEXT("Menu Description cannot exceed 40 characters."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                SetFocus(GetDlgItem(hDlg, IDC_MENU_DESCRIPTION));
                return TRUE;
            }

            if (menuType.empty())
            {
                MessageBox(hDlg, TEXT("Menu Type must be selected."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            std::vector<std::string> selectedDesigns =
                GetCheckedDesigns(GetDlgItem(hDlg, IDC_MENU_GENERATE_DESIGN_LIST));
            if (selectedDesigns.empty())
            {
                MessageBox(hDlg, TEXT("At least one Generate Design must be selected."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (state.fields.empty())
            {
                MessageBox(hDlg, TEXT("At least one field must be added."),
                    TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                return TRUE;
            }

            if (menuType == "UPLOAD")
            {
                int fileCount = 0;
                for (const Field& f : state.fields)
                    if (f.fieldType == "FILE") ++fileCount;

                if (fileCount != 1)
                {
                    MessageBox(hDlg, TEXT("An UPLOAD menu must contain exactly one FILE field."),
                        TEXT("Finacle Dev Assist"), MB_OK | MB_ICONWARNING);
                    return TRUE;
                }
            }

            // --- Build and return to caller ---
            CustomMenuData menu;
            menu.menuType = menuType;
            menu.menuName = menuName;
            menu.menuDescription = menuDesc;
            menu.generateDesign = selectedDesigns;
            menu.fields = state.fields;

            if (pMenu)
                *pMenu = menu;

            state.isDirty = false;
            EndDialog(hDlg, IDOK);
            return TRUE;
        }

        case IDCANCEL:
        {
            if (state.isDirty)
            {
                int result = MessageBox(hDlg,
                    TEXT("You have unsaved changes.\n\nAre you sure you want to close?"),
                    TEXT("Finacle Dev Assist"), MB_YESNO | MB_ICONWARNING);
                if (result != IDYES)
                    return TRUE;
            }
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
        }
        }
        break;
    }

    case WM_CLOSE:
    {
        if (state.isDirty)
        {
            int result = MessageBox(hDlg,
                TEXT("You have unsaved changes.\n\nAre you sure you want to close?"),
                TEXT("Finacle Dev Assist"), MB_YESNO | MB_ICONWARNING);
            if (result != IDYES)
                return TRUE;
        }
        EndDialog(hDlg, IDCANCEL);
        return TRUE;
    }

    case WM_DESTROY:
    {
        if (hDarkBrush) { DeleteObject(hDarkBrush); hDarkBrush = nullptr; }
        break;
    }
    }

    return FALSE;
}
