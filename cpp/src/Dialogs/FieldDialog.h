#pragma once

#include <windows.h>
#include <string>
#include <vector>
#include "../Models/CustomMenuModel.h"

struct FieldEditParams
{
    Field field;                    // pre-filled for Edit, default-constructed for Add
    bool isEditMode = false;           // Add vs Edit — controls the dialog title only

    // Option lists supplied by the parent (main window), already reflecting
    // its own config-driven values — this dialog does no config/JSON work
    // of its own, it only renders what it's given.
    std::vector<std::string> fieldPlacementOptions; // e.g. CRITERIA / DETAIL / RESULT
    std::vector<std::string> fieldTypeOptions;      // base set from config, WITHOUT "FILE"
    std::vector<std::string> searcherOptions;       // full searcher list from config

    // FILE is shown in the Field Type dropdown whenever Menu Type == UPLOAD —
    // unconditional on whether one has already been added elsewhere.
    bool showFileType = false;

    // Whether a FILE field already exists in the parent's Added Fields
    // collection, EXCLUDING the field currently being edited (if any).
    // Used only for validation on OK.
    bool fileFieldAlreadyExists = false;

    // Field IDs already present in the parent's Added Fields collection,
    // EXCLUDING the current field's own ID when editing.
    std::vector<std::string> existingFieldIds;
};

INT_PTR CALLBACK FieldEditDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);