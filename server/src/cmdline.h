/****************************************************************************
** $Id: cmdline.h,v 1.1 2002/10/22 19:48:21 thiagocorrea Exp $
**
** Implementation of CmdLine class
**
** Copyright (C) 2002 Bernhard Rosenkraenzer.  All rights reserved.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
**********************************************************************/

#if !defined(__CMDLINE_H__)
#define __CMDLINE_H__

#include "qstring.h"
#include "qmap.h"
#include "qstringlist.h"
/** @brief Command line handling
 * @author Bernhard Rosenkraenzer <bero@redhat.com>
 * TODO: Add support for short options
 */
class CmdLine {
public:
	/** @brief Parse command lines
	 * @param argc number of arguments (from main())
	 * @param argv arguments (from main())
	 */
	CmdLine(int argc, char **argv);

	~CmdLine();
	
	/** @brief See what argument was passed to an option.
	 * e.g. option("foo") will return "Linux" if the application
	 * was started with --foo=Linux
	 * If just --foo was specified, "yes" will be returned.
	 * @param opt Option to check
	 * @param default default value to return if the parameter was
	 * not specified on the command line (default: "")
	 * @return 
	 */
	QString option(QString const &opt, QString const &dflt="");

	/** @brief Return a list of arguments that are not options
	 * @return list of arguments that are not options
	 */
	QStringList args() { return *arg; }

	/** @brief Return the name of the executable
	 * @return name of executable (argv[0])
	 */
	QString const &name() const { return appname; }
private:
	QStringList* arg;
	QString appname;
	QMap<QString, QString> options;
};

#endif //__CMDLINE_H__
