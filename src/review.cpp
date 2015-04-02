//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#include "review.h"
#include <apiquery.hpp>
#include <core.hpp>
#include <collectable_smartptr.hpp>
#include <localization.hpp>
#include <QMessageBox>
#include <apiqueryresult.hpp>
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
#include <QInputDialog>

using namespace Huggle;

review::review()
{
    this->Window = nullptr;
}

review::~review()
{
    delete this->menuSeparator;
    delete this->menuAccept;
    delete this->menuReject;
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

void review::Hook_MainWindowOnRender()
{
    if (this->Window == nullptr)
        return;
    // Check if current wiki supports flagged revisions
    if (this->Window->GetCurrentWikiSite() == nullptr)
    {
        this->Switch(false);
        return;
    }
    bool supported = false;
    foreach (WikiSite_Ext extension, this->Window->GetCurrentWikiSite()->Extensions)
    {
        if (extension.Name == "Flagged Revisions")
        {
            supported = true;
            break;
        }
    }
    this->Switch(supported);
}

void review::Hook_MainWindowOnLoad(void *window)
{
    this->Window = (Huggle::MainWindow*)window;
    this->menuAccept = new QAction("Accept this revision", this->Window->ui->menuPage);
    this->menuReject = new QAction("Reject this revision", this->Window->ui->menuPage);
    this->menuSeparator = this->Window->ui->menuPage->addSeparator();
    this->Window->ui->menuPage->addAction(this->menuAccept);
    this->Window->ui->menuPage->addAction(this->menuReject);
    connect(this->menuReject, SIGNAL(triggered()), this, SLOT(ClickReject()));
    connect(this->menuAccept, SIGNAL(triggered()), this, SLOT(ClickAccept()));
    this->Switch(false);
}

static void Review_OK(Query *query)
{
    HUGGLE_DEBUG1("Approved OK");
    ApiQuery *q = (ApiQuery*)query;
    HUGGLE_DEBUG1(q->GetApiQueryResult()->Data);
    query->DecRef();
}

static void Review_Fail(Query *query)
{
    ApiQuery *q = (ApiQuery*)query;
    Syslog::HuggleLogs->ErrorLog("Unable to approve/reject edit: " + q->GetApiQueryResult()->ErrorMessage);
    HUGGLE_DEBUG1("Result: " + q->GetApiQueryResult()->Data);
    query->DecRef();
}

void review::ClickAccept()
{
    if (this->Window->CurrentEdit == nullptr || !this->Window->CheckExit() || !this->Window->CheckEditableBrowserPage())
        return;

    Collectable_SmartPtr<WikiEdit> edit = this->Window->CurrentEdit;
    bool ok = true;
    QString reason = QInputDialog::getText(this->Window, _l("reason"), "Provide a reason to accept this revision", QLineEdit::Normal,
                                    Huggle::Configuration::GenerateSuffix("Accepted using Huggle", edit->GetSite()->GetProjectConfig()), &ok);
    if (!ok)
        return;

    ApiQuery *query = new ApiQuery(ActionReview, this->Window->GetCurrentWikiSite());
    query->Parameters = "revid=" + QString::number(edit->RevID) + "&token=" +
            QUrl::toPercentEncoding(edit->GetSite()->GetProjectConfig()->Token_Csrf) +
            "&comment=" + QUrl::toPercentEncoding(reason);
    query->IncRef();
    query->callback = (Huggle::Callback) Review_OK;
    query->FailureCallback = (Huggle::Callback) Review_Fail;
    QueryPool::HugglePool->AppendQuery(query);
    query->UsingPOST = true;
    query->Process();
}

void review::ClickReject()
{
    if (this->Window->CurrentEdit == nullptr || !this->Window->CheckExit() || !this->Window->CheckEditableBrowserPage())
        return;

    Collectable_SmartPtr<WikiEdit> edit = this->Window->CurrentEdit;
    bool ok = true;
    QString reason = QInputDialog::getText(this->Window, _l("reason"), "Provide a reason to reject this revision", QLineEdit::Normal,
                                    Huggle::Configuration::GenerateSuffix("Rejected using Huggle", edit->GetSite()->GetProjectConfig()), &ok);
    if (!ok)
        return;

    ApiQuery *query = new ApiQuery(ActionReview, this->Window->GetCurrentWikiSite());
    query->Parameters = "revid=" + QString::number(edit->RevID) + "&token=" +
            QUrl::toPercentEncoding(edit->GetSite()->GetProjectConfig()->Token_Csrf) +
            "&comment=" + QUrl::toPercentEncoding(reason) + "&unapprove=1";
    query->IncRef();
    query->UsingPOST = true;
    query->callback = (Huggle::Callback) Review_OK;
    query->FailureCallback = (Huggle::Callback) Review_Fail;
    QueryPool::HugglePool->AppendQuery(query);
    query->Process();
}

void review::Switch(bool status)
{
    this->menuAccept->setEnabled(status);
    this->menuReject->setEnabled(status);
}

#if QT_VERSION < 0x050000
    Q_EXPORT_PLUGIN2("org.huggle.extension.qt", review)
#endif


