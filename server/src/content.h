
#if !defined(__CONTENT_H__)
#define __CONTENT_H__

// Forward declarations
class ContainerIterator;
class ContainerContent;
class cItem;
class QString;

#include <functional>

/*
	Use this to make sure that a certain item type exists in a container.
*/
class ContainerComparer {
public:
	virtual bool operator() (cItem *item) const {
		return false;
	}
};

// This class manages the content of a container
class ContainerContent {
friend class ContainerIterator;
friend class ContainerCopyIterator;

private:
	cItem **content;
public:
	ContainerContent(const ContainerContent &src) {
		// Copy content
		if (!src.content) {
			content = 0;
		} else {
			size_t count = src.maxCount() + 2; // How many slots
			content = new cItem* [count];
			for (size_t i = 0; i < count; ++i) {
				content[i] = src.content[i];
			}
		}
	}

	ContainerContent() {
		content = 0;
	}

	~ContainerContent() {
		clear();
	}

	// Tries to find an item in the container using the given ternary function.
	inline bool match(const ContainerComparer &functor) {
		bool result = false;
		size_t count = this->count();

		for (size_t i = 2; i < 2 + count; ++i) {			
			if (functor(content[i])) {
				result = true;
				break;
			}
		}

		return result;
	}

	inline size_t count() const {
		if (!content) {
			return 0;
		} else {
			return reinterpret_cast<size_t>(content[0]);
		}
	}

	inline size_t maxCount() const {
		if (!content) {
			return 0;
		} else {
			return reinterpret_cast<size_t>(content[1]);
		}
	}

	/*
		This method truncates the array or adds new elements to it.
	*/
	inline void ensureCapacity(size_t count) {
		// Round up to the nearest multiple of 16 for the item part
		size_t maxcount = ((count + 15) / 16) * 16 + 2; // 2 for real size and used size

		// Nothing at all...
		if (maxcount == 2) {
			clear();
		} else if (maxcount != maxCount()) {
			if (content) {
				// Save old pointer
				cItem **oldContent = content;
				content = new cItem*[maxcount];

				// Set the new size and maxsize
				content[0] = reinterpret_cast<cItem*>(count); // Write the _new_ count.
				content[1] = reinterpret_cast<cItem*>(maxcount);

				// Copy old items over
				size_t oldCount = reinterpret_cast<size_t>(oldContent[0]);
				for (size_t i = 2; (i < oldCount + 2) && (i < 2 + count); ++i) {
					content[i] = oldContent[i];
				}
		        
				// Delete old content
				delete [] oldContent;
			} else {
				content = new cItem*[maxcount];
				content[0] = reinterpret_cast<cItem*>(count);
				content[1] = reinterpret_cast<cItem*>(maxcount);
			}
		} else if (content) {
			content[0] = reinterpret_cast<cItem*>(count);
		}
	}

	/*
		Checks if this item container contains the given item.
	*/
	inline bool contains(const cItem *item) const {
		bool result = false;

		size_t count = this->count(); // If there is no content, this will always be zero
	
		for( unsigned int i = 2; i < count + 2; ++i )
		{
			if (content[i] == item) {
				result = true;
				break;
			}
		}

		return result;
	}

	/*
		Removes an item from the array
	*/
	inline void remove(const cItem *item) {
		bool found = false;
		size_t count = this->count();

		// Iterate trough all items.
		for( unsigned int i = 2; i < 2 + count; ++i )
		{
			if (content[i] != item) {
				continue; // Skip this item
			}

			// We found the item we want to remove here
			// What we do now is seek ahead and write all following items in the array
			// one index downwards
			for( unsigned int j = i + 1; j < 2 + count; ++j )
			{
				content[j - 1] = content[j]; // Swap Values
			}

			found = true;
			break; // It's insured that there is only once instance of an item here
		}

		if (found)  {
			ensureCapacity(count - 1); // Decrease itemcount
		}
	}

	/*
		Adds an item to the array if it's not already there.
	*/
	inline void add(cItem *item) {
		if (!contains(item)) {
			size_t count = this->count(); // Retrieve current count
			ensureCapacity(count + 1); // Resize array and write the new count
			content[2 + count] = item; // Write to the end of the array
		}
	}

	/*
		Clear the array.
	*/
	inline void clear() {
		delete [] content;
		content = 0;
	}

	/*
		Return a string representation of the array.
	*/
	QString dump();
};

// This class manages an iteration over the content of an item.
// This iterator is not invalidated by removing items from a container. However,
// the iterator could skip an item if it points to an item behind the deleted
// item.
class ContainerIterator {
private:
	size_t pos; // Position within the items array
	const ContainerContent &content;
public:
	/*
		Iterate over the content of an item.
	*/
	ContainerIterator::ContainerIterator(const cItem *item);

	/*
		Iterate over a ContainerContent instance.
	*/
	ContainerIterator(const ContainerContent &container) : content(container), pos(0) {
	}

    // Get the current element
	cItem *operator *() {
		size_t count = content.count();

		if (pos >= count) {
			return 0;
		} else {
			return content.content[2 + pos];
		}
	}

	// Jump back to the first item in the iteration
	void reset() {
		pos = 0;
	}

	// Step to the next position
	ContainerIterator &operator ++() {
		pos++;
		return *this;
	}

	// Step to the previous position
	ContainerIterator &operator --() {
		pos++;
		return *this;
	}

	// Is this iterator at the end?
	bool atEnd() {
		size_t count = content.count();
		return pos >= count;
	}

	// Utility Wrapper
	size_t count() {
		return content.count();
	}

	// Seek to a given position in the array
	void seek(size_t pos) {
		this->pos = pos;
	}
};

// This class works as mentioned above but contains a copy
// of the content.
class ContainerCopyIterator {
private:
	size_t pos; // Position within the items array
	ContainerContent content;
public:
	/*
		Iterate over the content of an item.
	*/
	ContainerCopyIterator::ContainerCopyIterator(const cItem *item);

	/*
		Iterate over a ContainerContent instance.
	*/
	ContainerCopyIterator(const ContainerContent &container) : pos(0) {
		content = ContainerContent(container);
	}

    // Get the current element
	cItem *operator *() {
		size_t count = content.count();

		if (pos >= count) {
			return 0;
		} else {
			return content.content[2 + pos];
		}
	}

	// Jump back to the first item in the iteration
	void reset() {
		pos = 0;
	}

	// Step to the next position
	ContainerCopyIterator &operator ++() {
		pos++;
		return *this;
	}

	// Step to the previous position
	ContainerCopyIterator &operator --() {
		pos++;
		return *this;
	}

	// Is this iterator at the end?
	bool atEnd() {
		size_t count = content.count();
		return pos >= count;
	}

	// Utility Wrapper
	size_t count() {
		return content.count();
	}

	// Seek to a given position in the array
	void seek(size_t pos) {
		this->pos = pos;
	}
};

#endif
