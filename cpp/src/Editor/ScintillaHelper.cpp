#include "ScintillaHelper.h"
#include "../PluginDefinition.h"

HWND ScintillaHelper::getCurrentEditor()
{
    int current = 0;

    SendMessage(nppData._nppHandle, NPPM_GETCURRENTSCINTILLA, 0, (LPARAM)&current);

    if (current == 0)
        return nppData._scintillaMainHandle;

    return nppData._scintillaSecondHandle;
}

std::string ScintillaHelper::getCurrentWord(HWND editor)
{
    int pos = (int)::SendMessage(editor, SCI_GETCURRENTPOS, 0, 0);
    int start = pos;

    while (start > 0)
    {
        char c = (char)::SendMessage(editor, SCI_GETCHARAT, start - 1, 0);

        if (!isalnum((unsigned char)c) &&
            c != '_' &&
            c != '<' &&
            c != '-')
        {
            break;
        }

        start--;
    }

    int length = pos - start;
    if (length <= 0)
        return "";

    Sci_TextRangeFull tr{};

    tr.chrg.cpMin = start;
    tr.chrg.cpMax = pos;

    std::vector<char> buffer(length + 1);

    tr.lpstrText = buffer.data();

    ::SendMessage(editor, SCI_GETTEXTRANGEFULL, 0, (LPARAM)&tr);

    return std::string(tr.lpstrText);
}


void ScintillaHelper::showAutoComplete(const std::string& prefix, const std::vector<std::string>& suggestions)
{
    HWND editor = getCurrentEditor();

    if (editor == nullptr)
    {
        return;
    }

    if (suggestions.empty())
    {
        return;
    }

    std::string list;
    for (size_t i = 0; i < suggestions.size(); ++i)
    {
        list += suggestions[i];
        if (i != suggestions.size() - 1)
            list += "|";
    }

    char separator = '|';

    ::SendMessage(editor, SCI_AUTOCSETSEPARATOR, (WPARAM)separator, 0);
    ::SendMessage(editor, SCI_AUTOCSETIGNORECASE, TRUE, 0);
    ::SendMessage(editor, SCI_AUTOCSHOW, (WPARAM)prefix.length(), (LPARAM)list.c_str());
}