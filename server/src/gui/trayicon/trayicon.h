#ifndef _SYSTEMTRAY_H
#define _SYSTEMTRAY_H

#include <QSystemTrayIcon>
#include <QAction>

class TrayIcon : public QSystemTrayIcon
{
	Q_OBJECT
	public:
		TrayIcon (QObject *);
		
	private slots:
		void systray_trigger (QSystemTrayIcon::ActivationReason reason);
		void toggle_hide ();
		void build_menu ();

	private:
		QAction *m_hide_action;
};

#endif