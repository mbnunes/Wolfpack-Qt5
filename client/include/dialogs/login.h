
#if !defined(__LOGIN_H__)
#define __LOGIN_H__

#include <qvaluelist.h>
#include <qhostaddress.h>
#include <qobject.h>

#include "gui/control.h"
#include "gui/window.h"
#include "gui/imagebutton.h"
#include "gui/textfield.h"
#include "gui/asciilabel.h"

// Structure used for the shardlist
struct stShardEntry {
	QCString name;
	QHostAddress pingAddress;
	unsigned char percentFull;
	unsigned char timezone;
	unsigned short id;
};

enum enMenuPage {
	PAGE_LOGIN = 0,
	PAGE_CONNECTING,
	PAGE_VERIFYING,
	PAGE_SHARDLIST
};

class cLoginDialog : public QObject {
Q_OBJECT

private:
	cWindow *container; // The main container
	cImageButton *movieButton, *nextButton, *backButton;
	cContainer *accountLoginGump;
	cContainer *shardSelectGump;
	cTextField *inpAccount, *inpPassword;
	cContainer *shardList;
	cContainer *statusDialog;
	cAsciiLabel *statusLabel;
	enMenuPage page;

	void buildAccountLoginGump();
	void buildShardSelectGump();
	void buildStatusGump();

	QValueList<stShardEntry> shards;
	unsigned int shardEntryOffset;
public:
	void onScrollShardList(int oldpos, int newpos);

	cLoginDialog();
	~cLoginDialog();

	void setStatusText(const QString &text);

	void show(enMenuPage page); // Show the login dialog
	void hide(); // Hide the login dialog


	// Callback for the ShardList
	void clearShardList();
	void addShard(const stShardEntry &shard);

	// External connection callbacks.
	void onError(const QString &error);
	void onDnsLookupComplete(const QHostAddress &address, unsigned short port);
	void onConnect();

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
};

extern cLoginDialog *LoginDialog; // There is only one LoginDialog instance at a time

#endif
