/*****************************************************************************
    NumeRe: Framework fuer Numerische Rechnungen
    Copyright (C) 2019  Erik Haenel et al.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
******************************************************************************/

#include "revisiondialog.hpp"
#include "../../kernel/core/ui/language.hpp"
#include "../NumeReWindow.h"

BEGIN_EVENT_TABLE(RevisionDialog, wxDialog)
    EVT_TREE_ITEM_RIGHT_CLICK(-1, RevisionDialog::OnRightClick)
    EVT_TREE_ITEM_ACTIVATED(-1, RevisionDialog::OnItemActivated)
    EVT_MENU_RANGE(ID_REVISIONDIALOG_SHOW, ID_REVISIONDIALOG_RESTORE, RevisionDialog::OnMenuEvent)
END_EVENT_TABLE()

extern Language _guilang;


RevisionDialog::RevisionDialog(wxWindow* parent, FileRevisions* rev, const wxString& currentFileName)
    : wxDialog(parent, wxID_ANY, _guilang.get("GUI_DLG_REVISIONDIALOG_TITLE"), wxDefaultPosition, wxSize(750, 500), wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX), revisions(rev), currentFile(currentFileName)
{
    mainWindow = static_cast<NumeReWindow*>(parent);

    wxBoxSizer* vsizer = new wxBoxSizer(wxVERTICAL);
    revisionList = new wxcode::wxTreeListCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTR_TWIST_BUTTONS | wxTR_FULL_ROW_HIGHLIGHT);
    revisionList->AddColumn(_guilang.get("GUI_DLG_REVISIONDIALOG_REV"), 150);
    revisionList->AddColumn(_guilang.get("GUI_DLG_REVISIONDIALOG_DATE"), 150);
    revisionList->AddColumn(_guilang.get("GUI_DLG_REVISIONDIALOG_COMMENT"), 450);
    revisionList->AddRoot(currentFile);

    populateRevisionList();

    vsizer->Add(revisionList, 1, wxEXPAND | wxALL, 5);
    vsizer->Add(CreateButtonSizer(wxOK), 0, wxALIGN_CENTER_HORIZONTAL | wxALL, 5);

    SetSizer(vsizer);
}


void RevisionDialog::populateRevisionList()
{
    revisionList->SetItemText(revisionList->GetRootItem(), 0, currentFile + " (" + revisions->getCurrentRevision() + ")");

    wxArrayString revList = revisions->getRevisionList();
    wxTreeItemId currentItem;

    for (size_t i = 0; i < revList.size(); i++)
    {
        if (revList[i].substr(0, 3) == "tag")
        {
            wxTreeItemId currentTagItem = revisionList->AppendItem(currentItem, revList[i].substr(0, revList[i].find('\t')));
            revisionList->SetItemText(currentTagItem, 1, revList[i].substr(revList[i].find('\t')+1, revList[i].find('\t', revList[i].find('\t')+1) - revList[i].find('\t')-1));
            revisionList->SetItemText(currentTagItem, 2, revList[i].substr(revList[i].rfind('\t')+1));
        }
        else
        {
            currentItem = revisionList->AppendItem(revisionList->GetRootItem(), revList[i].substr(0, revList[i].find('\t')));
            revisionList->SetItemText(currentItem, 1, revList[i].substr(revList[i].find('\t')+1, revList[i].find('\t', revList[i].find('\t')+1) - revList[i].find('\t')-1));
            revisionList->SetItemText(currentItem, 2, revList[i].substr(revList[i].rfind('\t')+1));
        }
    }

    revisionList->Expand(revisionList->GetRootItem());
}


void RevisionDialog::showRevision(const wxString& revString)
{
    wxString revision = revisions->getRevision(revString);
    mainWindow->ShowRevision(revString + "-" + currentFile, revision);
}


void RevisionDialog::OnRightClick(wxTreeEvent& event)
{
    clickedItem = event.GetItem();

    // do nothing, if the parent node is not the
    // table root node
    if (event.GetItem() == revisionList->GetRootItem())
        return;

    // Create the menu
    wxMenu popUpmenu;

    // Append commons
    popUpmenu.Append(ID_REVISIONDIALOG_SHOW, _guilang.get("GUI_DLG_REVISIONDIALOG_SHOW"));
    popUpmenu.AppendSeparator();
    popUpmenu.Append(ID_REVISIONDIALOG_TAG, _guilang.get("GUI_DLG_REVISIONDIALOG_TAG"));
    popUpmenu.Append(ID_REVISIONDIALOG_RESTORE, _guilang.get("GUI_DLG_REVISIONDIALOG_RESTORE"));

    // Display the menu
    PopupMenu(&popUpmenu, event.GetPoint());

}


void RevisionDialog::OnItemActivated(wxTreeEvent& event)
{
    wxString revID = revisionList->GetItemText(event.GetItem());
    showRevision(revID);
}


void RevisionDialog::OnMenuEvent(wxCommandEvent& event)
{
    wxString revID = revisionList->GetItemText(clickedItem);

    switch (event.GetId())
    {
        case ID_REVISIONDIALOG_SHOW:
            showRevision(revID);
            break;
        case ID_REVISIONDIALOG_TAG:
            {
                wxTextEntryDialog textdialog(this, _guilang.get("GUI_DLG_REVISIONDIALOG_PROVIDETAGCOMMENT"), _guilang.get("GUI_DLG_REVISIONDIALOG_PROVIDETAGCOMMENT_TITLE"), wxEmptyString, wxCENTER | wxOK | wxCANCEL);
                int ret = textdialog.ShowModal();

                if (ret == wxID_OK)
                {
                    revisions->tagRevision(revID, textdialog.GetValue());
                    revisionList->DeleteChildren(revisionList->GetRootItem());
                    populateRevisionList();
                }

                break;
            }
        case ID_REVISIONDIALOG_RESTORE:
            {
                wxFileDialog filedialog(this, _guilang.get("GUI_DLG_REVISIONDIALOG_RESTOREFILE"), wxGetCwd(), currentFile, wxFileSelectorDefaultWildcardStr, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
                int ret = filedialog.ShowModal();

                if (ret == wxID_OK)
                    revisions->restoreRevision(revID, filedialog.GetPath());

                break;
            }
    }
}

