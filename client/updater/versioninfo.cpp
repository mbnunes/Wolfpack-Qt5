
#include "versioninfo.h"

cVersionInfo *cVersionInfo::fromXml(const QDomElement &element) {
	if (element.nodeName() != "version") {
		return 0;
	}

	// Get the last modification time
	QDateTime lastModification;
	lastModification.setTime_t(element.attribute("generated").toUInt());
	lastModification.setTimeSpec(Qt::UTC);
	lastModification = lastModification.toLocalTime();

	// Iterate over subnodes
	QDomElement moduleNode = element.firstChildElement("module");
	Modules modules;

	while (!moduleNode.isNull()) {
		// This is rather lousy error checking
		if (moduleNode.hasAttribute("name") && moduleNode.hasAttribute("url")) {
			QString name = moduleNode.attribute("name");
			QString url = moduleNode.attribute("url");
			uint size = moduleNode.attribute("size").toUInt();
			Components components;
            
			// Get all the files within this module
			QDomElement fileNode = moduleNode.firstChildElement("file");
			while (!fileNode.isNull()) {
				if (fileNode.hasAttribute("path") && fileNode.hasAttribute("modified")) {
					QString path = fileNode.attribute("path");
					QDateTime modified;
					modified.setTime_t(fileNode.attribute("modified").toUInt());
					modified.setTimeSpec(Qt::UTC);
					modified = modified.toLocalTime();

					components.append(cComponentVersion(path, modified));
				}

				fileNode = fileNode.nextSiblingElement("file");
			}

			// Append the module
			modules.append(cModuleVersion(name, url, size, components));
		}

		moduleNode = moduleNode.nextSiblingElement("module");
	}

	return new cVersionInfo(modules, lastModification);
}
