
#if !defined(__MACROS_H__)
#define __MACROS_H__

#include <QObject>
#include <QString>
#include <QKeySequence>
#include <QDomDocument>
#include <QDomElement>

/*
	Base class for performing actions.
*/
class cBaseAction : public QObject {
Q_OBJECT
protected:
	QKeySequence sequence_;
public:
	cBaseAction();
	virtual ~cBaseAction();

	virtual QDomElement save(QDomDocument &document) = 0;
	virtual void load(QDomElement &element) = 0;

public slots:
	virtual void perform() = 0;

	void setSequence(const QKeySequence &sequence);
	const QKeySequence &sequence() const;
};

inline void cBaseAction::setSequence(const QKeySequence &sequence) {
	sequence_ = sequence;
}

inline const QKeySequence &cBaseAction::sequence() const {
	return sequence_;
}

/*
	Builtin action for key bindings.
*/
class cCoreAction : public cBaseAction  {
Q_OBJECT
protected:
	QString action_; // Id of internal action
public:
	QDomElement save(QDomDocument &document);
	void load(QDomElement &element);
public slots:
	const QString &action() const;
	void setAction(const QString &data);
	void perform();
	void perform(const QString &action);

	// Actions
	void toggleWarmode();
	void createScreenshot();
};

inline const QString &cCoreAction::action() const {
	return action_;
}

inline void cCoreAction::setAction(const QString &data) {
	action_ = data;
}

/*
	This is used for macroing actions
*/
class cMacroAction : public cBaseAction {
Q_OBJECT
protected:
	QString script_;
public:
	QDomElement save(QDomDocument &document);
	void load(QDomElement &element);
public slots:
	void perform();
	const QString &script() const;
	void setScript(const QString &script);
};

inline const QString &cMacroAction::script() const {
	return script_;
}

inline void cMacroAction::setScript(const QString &data) {
	script_ = data;
}

#endif
