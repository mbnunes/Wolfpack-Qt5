/*
 *     Wolfpack Emu (WP)
 * UO Server Emulation Program
 *
 * Copyright 2001-2004 by holders identified in AUTHORS.txt
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Palace - Suite 330, Boston, MA 02111-1307, USA.
 *
 * In addition to that license, if you are running this program or modified
 * versions of it on a public system you HAVE TO make the complete source of
 * the version used by you available or provide people with a location to
 * download it.
 *
 * Wolfpack Homepage: http://wpdev.sf.net/
 */

#if !defined(__WPCONSOLE_H__)
#define __WPCONSOLE_H__

// Platform specifics
#include "platform.h"
#include "typedefs.h"
#include "singleton.h"
#include "server.h"

// System Includes
#include <qstringlist.h>
#include <qstring.h>
#include <qmutex.h>

/*!
	\brief This is an enumeration
*/
enum enConsoleColors {
	WPC_NORMAL = 0,
	WPC_RED,
	WPC_GREEN,
	WPC_YELLOW,
	WPC_WHITE,
	WPC_BROWN
};

/*!
	\brief This is an enumeration of possible font
		types for console output.
*/
enum enFontType {
	FONT_SERIF = 0,
	FONT_NOSERIF,
	FONT_FIXEDWIDTH
};

/*!
	\brief This class encapsulates all access to server console features.
*/
class cConsole {

private:
	QStringList linebuffer_;
	QString incompleteLine_;
	QStringList commandQueue;
	QMutex commandMutex;
	QString progress;

	/*!
		\brief This method is internally used to handle a command.
		\params command The command to execute.
		\params silent Should the command fail silently.
	*/
	bool handleCommand(const QString &command, bool silent = false);

public:


	/*!
		\brief Construct a new console object. Only available to
			the \s SingletonHolder class.
	*/
	cConsole();

	/*!
		\brief Destructor for an instance of the console object.
	*/
	~cConsole();

	// Send a message to the console
	void send(const QString &sMessage);

	/*!
		\brief Send a message to the console and to the logfile as well.
		\params loglevel The loglevel for the message.
		\params message The message you want to log.
		\params timestamp Should the message be timestamped.
	*/
	void log(UINT8 logLevel, const QString &message, bool timestamp = true);

	/*!
		\brief Give the console time to start.
	*/
	void start();

	/*!
		\brief Give the console time to process events.
	*/
	void poll();

	/*!
		\brief Stop the console.
	*/
	void stop();

	/*!
		\brief Send a progress line to the console.
			It has to be terminted by \s sendDone,
			\s sendFail or \s sendSkip.
		\params title The title of the progress line.
	*/
	void sendProgress(const QString &title);

	/*!
		\brief End a progress line and indicate success.
	*/
	void sendDone();

	/*!
		\brief End a progress line and indicate failure.
	*/
	void sendFail();

	/*!
		\brief End a progress line and indicate that the
			process was skipped.
	*/
	void sendSkip();

	/*!
		\brief Change the console foreground color for
			text output.
		\params color The new textcolor.
	*/
	void changeColor(enConsoleColors color);

	/*!
		\brief Changes the console window title.
		\params title The new console title.
	*/
	void setConsoleTitle(const QString &title);

	/*!
		\brief Notify the console of a server state change.
		\params newstate The new serverstate.
	*/
	void notifyServerState(enServerState newstate);

	/*!
		\brief Returns a reference to the linebuffer
			storing all lines sent to the console.
	*/
	const QStringList &linebuffer() const {
		return linebuffer_;
	}

	/*!
		\brief Queue a command to the console. This is thread-safe.
		\params command The command you want to queue.
	*/
	void queueCommand(const QString &command);

	/*!
		\brief This function sets the text attribute for advanced
			consoles.
		\params bold Specifies whether the sent text should be bold.
		\params italic Specifies whether the sent text should be italic.
		\params underlined Specifies whether the sent text should be underlined.
		\params r The red part of the text color.
		\params g The green part of the text color.
		\params b The blue part of the text color.
		\params size The size of the text.
		\params font The font used to draw the text.
	*/
	void setAttributes(bool bold, bool italic, bool underlined, unsigned char r, unsigned char g, unsigned char b, unsigned char size, enFontType font);
};

typedef SingletonHolder<cConsole> Console;

#endif
