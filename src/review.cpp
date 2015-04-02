//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "review.h"
#include <core.hpp>
#include <collectable_smartptr.hpp>
#include <localization.hpp>
#include <QMessageBox>
#include <query.hpp>
#include <querypool.hpp>
#include <mainwindow.hpp>
#include <generic.hpp>
#include <ui_mainwindow.h>
#include <wikiedit.hpp>
#include <wikisite.hpp>
#include <wikipage.hpp>
#include <wikiuser.hpp>
#include <syslog.hpp>
#include <configuration.hpp>

using namespace Huggle;

review::review()
{
}

review::~review()
{
    delete this->menuAfd;
    delete this->menuProd;
}

bool review::Register()
{
    this->Init();
    return true;
}

bool review::IsWorking()
{
    return true;
}

void review::Hook_MainWindowOnLoad(void *window)
{
    this->Window = (Huggle::MainWindow*)window;
    this->menuProd = new QAction("PROD", this->Window->ui->menuPage);
    this->Window->ui->menuPage->addAction(this->menuProd);
    connect(this->menuProd, SIGNAL(triggered()), this, SLOT(ClickPROD()));
}

bool review::Hook_RevertPreflight(void *edit)
{
    if (edit == nullptr)
        return true;
    WikiEdit *E = (WikiEdit*)edit;
    if (E->GetSite()->Name != "review")
        return true;
    if (E->Page->FounderKnown() && E->Page->GetFounder() == E->User->Username)
    {
        int result = Generic::MessageBox("Revert", "This edit was made by a person who created this page, do you want to request a speedy deletion instead?",
                                                Huggle::MessageBoxStyleQuestion);
        if (result == QMessageBox::Yes)
        {
            this->Window->RequestPD(E);
            return false;
        }
    }
    return true;
}

bool review::Hook_SpeedyBeforeOK(void *edit, void *form)
{
    SpeedyForm *sw = (SpeedyForm*)form;
    if (((WikiEdit*)edit)->GetSite()->Name != "review")
        return true;
    // blank a target page if warning is g3 or g10
    if (sw->GetSelectedTagID() == "db-g3" || sw->GetSelectedTagID() == "db-g10")
    {
        sw->ReplacingText = "{{subst:Courtesy blanked}}";
        sw->ReplacePage = true;
    }
    // don't send a message to user on G7
    if (sw->GetSelectedTagID() == "db-g7")
        sw->SetMessageUserCheck(false);
    return true;
}

void review::ClickPROD()
{
    if (!this->Window->CheckEditableBrowserPage() || !review::WikiCk(this->Window->GetCurrentWikiSite()))
        return;
    if (this->Window->GetCurrentWikiEdit()->Page->IsTalk())
    {
        Generic::pMessageBox(this->Window, "Error", "You can't PROD talk pages");
        return;
    }
    QString pn = this->Window->CurrentEdit->Page->GetNS()->GetCanonicalName();
    if (pn == "Template")
    {
        Generic::MessageBox("Error", "You can't use PROD for templates or categories");
        return;
    }
    ProdWn *ptr = new ProdWn(this->Window->CurrentEdit, this->Window);
    ptr->setAttribute(Qt::WA_DeleteOnClose, true);
    ptr->show();
}

#if QT_VERSION < 0x050000
    Q_EXPORT_PLUGIN2("org.huggle.extension.qt", review)
#endif


