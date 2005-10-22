
#if !defined(__VERSIONINFO_H__)
#define __VERSIONINFO_H__

#include <QString>
#include <QVector>
#include <QDateTime>
#include <QDomElement>

/*
	Class encapsulating a single component.
*/
class cComponentVersion {
protected:
	QString path_;
	QDateTime lastModified_;
public:
    cComponentVersion &operator=(const cComponentVersion &o);
	cComponentVersion(const QString &path, const QDateTime &lastModified);
	cComponentVersion();
    const QDateTime &lastModified() const;
	const QString &path() const;
};

inline cComponentVersion &cComponentVersion::operator=(const cComponentVersion &o) {
	path_ = o.path_;
	lastModified_ = o.lastModified_;
	return *this;
}

inline cComponentVersion::cComponentVersion() {
}

inline cComponentVersion::cComponentVersion(const QString &path, const QDateTime &lastModified) {
	path_ = path;
	lastModified_ = lastModified;
}

inline const QDateTime &cComponentVersion::lastModified() const {
	return lastModified_;
}

inline const QString &cComponentVersion::path() const {
	return path_;
}

typedef QVector<cComponentVersion> Components;

class cModuleVersion {
protected:
	QString name_;
	QString url_;
	Components components_;
	uint size_;
public:
	cModuleVersion(const QString &name, const QString &url, uint size, const Components &components);
	cModuleVersion();
	const QString &name() const;
	const QString &url() const;	
	const Components &components() const;
	const uint size() const;
};

inline const uint cModuleVersion::size() const {
	return size_;
}

inline cModuleVersion::cModuleVersion(const QString &name, const QString &url, uint size, const Components &components) {
	name_ = name;
	url_ = url;
	components_ = components;
	size_ = size;
}

inline cModuleVersion::cModuleVersion() {
}

inline const QString &cModuleVersion::name() const {
	return name_;
}

inline const QString &cModuleVersion::url() const {
	return url_;
}

inline const Components &cModuleVersion::components() const {
	return components_;
}

typedef QVector<cModuleVersion> Modules;

/*
	Class encapsulating version information.
*/
class cVersionInfo {
protected:
	Modules modules_;
	QDateTime lastUpdate_;
public:
	static cVersionInfo *fromXml(const QDomElement &element);

	cVersionInfo(Modules modules, const QDateTime &lastUpdate);
    const Modules &modules() const;
	const QDateTime &lastUpdate() const;
};

inline cVersionInfo::cVersionInfo(Modules modules, const QDateTime &lastUpdate) {
	modules_ = modules;
	lastUpdate_ = lastUpdate;
}

inline const Modules &cVersionInfo::modules() const {
	return modules_;
}

inline const QDateTime &cVersionInfo::lastUpdate() const {
	return lastUpdate_;
}

#endif
