#include "trayicon.h"

#include <QSystemTrayIcon>
#include <QSettings>
#include <QMenu>

#include "../mainwindow.h"

TrayIcon::TrayIcon (QObject *parent) : QSystemTrayIcon (parent)
{
	QSettings s;

	connect (this, SIGNAL (activated (QSystemTrayIcon::ActivationReason)),
			 this, SLOT (systray_trigger (QSystemTrayIcon::ActivationReason)));

	this->setToolTip("Wolfpack");
	setIcon (QIcon (":/gui/images/icon_red.png"));
	
	MainWindow *mw = dynamic_cast<MainWindow*> (parent);

	QAction *a;
	
	QMenu *systray_menu = new QMenu (mw);

	m_hide_action = systray_menu->addAction (tr ("Hide main window"), this, SLOT (toggle_hide ()));

	connect (systray_menu, SIGNAL (aboutToShow ()), this, SLOT (build_menu ()));

	setContextMenu (systray_menu);
}

void
TrayIcon::build_menu ()
{
	MainWindow *mw = dynamic_cast<MainWindow*> (parent ());

	if (mw->isHidden ()) {
		m_hide_action->setText (tr ("Show main window"));
	} else {
		m_hide_action->setText (tr ("Hide main window"));
	}
}

void
TrayIcon::toggle_hide ()
{
	MainWindow *pw = dynamic_cast<MainWindow*> (parent ());
	if (pw->isHidden ()) {
		pw->show ();
		m_hide_action->setText (tr ("Hide main window"));
	} else {
		pw->hide ();
		m_hide_action->setText (tr ("Show main window"));
	}
}

void 
TrayIcon::systray_trigger (QSystemTrayIcon::ActivationReason reason)
{
	MainWindow *pw = dynamic_cast<MainWindow*> (parent ());

	if (reason == QSystemTrayIcon::Trigger)
	{
		if (pw->isHidden())
		{
			pw->show ();
			pw->activateWindow();
		}
		else
			pw->hide();
	}
}