/* BEGIN_COMMON_COPYRIGHT_HEADER
 *
 * Razor - a lightweight, Qt based, desktop toolset
 * https://sourceforge.net/projects/razor-qt/
 *
 * Copyright: 2010-2011 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.ru>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation;  only version 2 of
 * the License is valid for this program.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * END_COMMON_COPYRIGHT_HEADER */


#include "razormainmenu.h"
//#include "razor.h"
#include <QDebug>
#include <QtGui/QMenu>
#include <razorqt/xdgdesktopfile.h>
#include <razorqt/xdgmenu/xdgmenu.h>
#include <razorqt/domhelper.h>
#include <QSettings>
#include <QFileInfo>
#include <QAction>
#include <QtGui/QMessageBox>
#include <razorqt/powermanager.h>
#include <razorqt/screensaver.h>

#include <razorqt/xdgenv.h>
//#include <razorbar.h>
#include <razorqt/xdgicon.h>
#include <razorqt/xdgdesktopfile.h>
#include <razorqt/xdgmenu/xdgmenuwidget.h>

#include <QStack>

#include <QCursor>
EXPORT_RAZOR_PANEL_PLUGIN_CPP(RazorMainMenu)


/************************************************

 ************************************************/
RazorMainMenu::RazorMainMenu(const RazorPanelPluginStartInfo* startInfo, QWidget* parent):
    RazorPanelPlugin(startInfo, parent),
    mMenu(0)
{
    setObjectName("MainMenu");

    addWidget(&mButton);
    connect(&mButton, SIGNAL(clicked()), this, SLOT(showMenu()));
    mPowerManager = new PowerManager(this);
    mPowerManager->setParentWidget(panel());

    mScreenSaver = new ScreenSaver(this);

    settingsChanged();

    QSizePolicy sp = mButton.sizePolicy();
    sp.setVerticalPolicy(QSizePolicy::Minimum);
    mButton.setSizePolicy(sp);

}


/************************************************

 ************************************************/
RazorMainMenu::~RazorMainMenu()
{
    settings().setValue("text", mButton.text());
    settings().setValue("icon_size", mMenuStyle.iconSize());
    settings().setValue("top_icon_size", mTopMenuStyle.iconSize());
}


/************************************************

 ************************************************/
void RazorMainMenu::showMenu()
{
    if (!mMenu)
        buildMenu();

    if (!mMenu)
        return;

    int x, y;

    switch (panel()->position())
    {
        case RazorPanel::PositionTop:
            x = mButton.mapToGlobal(QPoint(0, 0)).x();
            y = panel()->mapToGlobal(QPoint(0, panel()->sizeHint().height())).y();
            break;

        case RazorPanel::PositionBottom:
            x = mButton.mapToGlobal(QPoint(0, 0)).x();
            y = panel()->mapToGlobal(QPoint(0, 0)).y() - mMenu->sizeHint().height();
            break;

        case RazorPanel::PositionLeft:
            x = panel()->mapToGlobal(QPoint(panel()->sizeHint().width(), 0)).x();
            y = mButton.mapToGlobal(QPoint(0, 0)).y();
            break;

        case RazorPanel::PositionRight:
            x = panel()->mapToGlobal(QPoint(0, 0)).x() - mMenu->sizeHint().width();
            y = mButton.mapToGlobal(QPoint(0, 0)).y();
            break;

    }

    QPoint pos(x, y);
    mMenu->exec(pos);
}


/************************************************

 ************************************************/
void RazorMainMenu::settingsChanged()
{
    mButton.setText(settings().value("text", "").toString());
    mLogDir = settings().value("log_dir", "").toString();

    mMenuStyle.setIconSize(settings().value("icon_size", 16).toInt());
    mTopMenuStyle.setIconSize(settings().value("top_icon_size", 16).toInt());


    mMenuFile = settings().value("menu_file", "").toString();
    if (mMenuFile.isEmpty())
        mMenuFile = XdgMenu::getMenuFileName();

    QIcon icon =  XdgIcon::fromTheme(settings().value("button_icon").toString());
    if (!icon.isNull())
        mButton.setIcon(icon);
}


/************************************************

 ************************************************/
void RazorMainMenu::buildMenu()
{
    XdgMenu xdgMenu(mMenuFile);
    xdgMenu.setLogDir(mLogDir);

    bool res = xdgMenu.read();
    if (res)
    {
        mMenu = new XdgMenuWidget(xdgMenu, "", this);
        mMenu->setObjectName("TopLevelMainMenu");
        mMenu->setStyle(&mTopMenuStyle);
    }
    else
    {
        QMessageBox::warning(this, "Parse error", xdgMenu.errorString());
        return;
    }

    mMenu->addSeparator();

    QMenu* leaveMenu = mMenu->addMenu(XdgIcon::fromTheme("system-shutdown"), tr("Leave"));
    leaveMenu->addActions(mPowerManager->availableActions());

    mMenu->addActions(mScreenSaver->availableActions());
}
