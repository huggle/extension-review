//This program is free software: you can redistribute it and/or modify
//it under the terms of the GNU General Public License as published by
//the Free Software Foundation, either version 3 of the License, or
//(at your option) any later version.

//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.

#ifndef REVIEW_H
#define REVIEW_H

#define HUGGLE_EXTENSION
#include <iextension.hpp>
#include <QAction>
#include <QDeclarativeItem>

namespace Huggle
{
    class MainWindow;
    class WikiSite;
}

class review : public QDeclarativeItem, public Huggle::iExtension
{
    Q_OBJECT
#if QT_VERSION >= 0x050000
        Q_PLUGIN_METADATA(IID "org.huggle.extension.qt" FILE "review.json")
#endif
    Q_INTERFACES(Huggle::iExtension)
    public:
        static bool WikiCk(Huggle::WikiSite *site);

        review();
        ~review();
        bool Register();
        bool IsWorking();
        void Hook_MainWindowOnLoad(void *window);
        QString GetExtensionName() { return "Review"; }
        QString GetExtensionAuthor() { return "Petr Bena"; }
        QString GetExtensionDescription() { return "Implements options to review edits"; }
        QString GetExtensionVersion() { return "1.0.0"; }
        bool RequestCore() { return true; }
        bool RequestNetwork() { return true; }
        bool RequestConfiguration() { return true; }
    private slots:
        void ClickAccept();
        void ClickReject();
    private:
        QAction *menuAccept;
        QAction *menuReject;
        Huggle::MainWindow *Window;
};

QML_DECLARE_TYPE(review)

#endif // REVIEW_H
