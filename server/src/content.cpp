
#include "content.h"
#include "items.h"
#include <qstring.h>

/*
	Iterate over the content of an item.
*/
ContainerIterator::ContainerIterator(const cItem *item) : content(item->content()), pos(0) {
}

ContainerCopyIterator::ContainerCopyIterator(const cItem *item) : pos(0) {
	content = ContainerContent(item->content());
}

QString ContainerContent::dump() {
	QString result = "Content of array:\n";
	result += QString("Count: %1; MaxCount: %2\n").arg(count()).arg(maxCount());

	size_t count = this->count();
	for (size_t i = 2; i < 2 + count; ++i) {
		result += QString("At %1: 0x%2\n").arg(i - 2).arg(reinterpret_cast<size_t>(content[i]), 0, 16);
	}
	return result;
}
