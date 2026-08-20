#pragma once

#include <windows.h>
#include "../Models/CustomMenuModel.h"

struct OptionEditParams
{
    Option option;
    bool isEditMode = false;
};

INT_PTR CALLBACK OptionEditDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);