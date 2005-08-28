
#if !defined(__LOGIN_H__)
#define __LOGIN_H__

#include <q3valuelist.h>
#include <qhostaddress.h>
#include <qobject.h>
#include <qstringlist.h>
//Added by qt3to4:
#include <qstring.h>

#include "gui/control.h"
#include "gui/window.h"
#include "gui/imagebutton.h"
#include "gui/textfield.h"
#include "gui/asciilabel.h"

// Structure used for the shardlist
struct stShardEntry {
	QString name;
	QHostAddress pingAddress;
	unsigned char percentFull;
	unsigned char timezone;
	unsigned short id;
};

// This is a starting location
struct stStartLocation {
    Q3CString name;
	Q3CString exactName;
	unsigned char index;
};

enum enMenuPage {
	PAGE_LOGIN = 0,
	PAGE_CONNECTING,
	PAGE_VERIFYING,
	PAGE_SHARDLIST,
	PAGE_SELECTCHAR,
	PAGE_CONFIRMDELETE,
	PAGE_DELETING,
	PAGE_ENTERING,
};

class cCharSelection;

class cLoginDialog : public QObject {
Q_OBJECT

private:
	cCharSelection *charSelectWidget;
	cWindow *container; // The main container
	cAsciiLabel *lastShardName;
	cImageButton *movieButton, *nextButton, *backButton;
	cContainer *accountLoginGump;
	cContainer *shardSelectGump;
	cTextField *inpAccount, *inpPassword;
	cContainer *shardList;
	cContainer *confirmDeleteDialog;
	cAsciiLabel *confirmDeleteText;
	cContainer *statusDialog;
	cContainer *selectCharDialog;
	cAsciiLabel *statusLabel;
	enMenuPage page;
	cImageButton *statusCancel, *statusOk;
	cBorderGump *selectCharBorder[6];
	QStringList characterNames;

	void buildConfirmDeleteGump();
	void buildAccountLoginGump();
	void buildShardSelectGump();
	void buildStatusGump();
	void buildSelectCharGump();

	Q3ValueList<stShardEntry> shards;
	unsigned int shardEntryOffset;
	bool errorStatus;
public:
	void onScrollShardList(int oldpos, int newpos);

	cLoginDialog();
	~cLoginDialog();

	void setStatusText(const QString &text);
	void setErrorStatus(bool error);

	void show(enMenuPage page); // Show the login dialog
	void hide(); // Hide the login dialog

	// Callback for the ShardList
	void clearShardList();
	void addShard(const stShardEntry &shard);
	void setCharacterList(const QStringList &characters);

public slots:
	// Button callbacks for this page.
	void quitClicked(cControl *sender);
	void nextClicked(cControl *sender);
	void backClicked(cControl *sender);
	void myUoClicked(cControl *sender);
	void creditsClicked(cControl *sender);
	void accountClicked(cControl *sender);
    void movieClicked(cControl *sender);
	void helpClicked(cControl *sender);
	void deleteCharClicked(cControl *sender);
	void createCharClicked(cControl *sender);
	void statusCancelClicked(cControl *sender);
	void statusOkClicked(cControl *sender);
	void charSelected(cControl *sender);

	// These are connected to the uoSocket
	void socketError(const QString &error);
	void socketHostFound();
	void socketConnect();
	void selectShard(int id);
	void enterPressed(cTextField *field);
	void selectLastShard();
};

extern cLoginDialog *LoginDialog; // There is only one LoginDialog instance at a time

#endif
