#include "dllist.h"
#include "system.h"

// #define DEBUG_YIELD_OUTPUT

#ifdef DEBUG_YIELD_OUTPUT
#define YIELD_AND_REPORT() \
do { \
printf("In function \"%s\":\n  Switch to another thread at code line %d!\n", __FUNCTION__, __LINE__); \
currentThread->Yield(); \
} while (0)
#else
#define YIELD_AND_REPORT() \
do { \
currentThread->Yield(); \
} while (0)
#endif

extern int errorType;

#define YIELD_ON_TYPE(t) \
	if (errorType == t || errorType == 10000)	 \
	{ \
		YIELD_AND_REPORT(); \
	} \




DLLElement::DLLElement(void* itemPtr, int sortKey)
{
	item = itemPtr;
	key = sortKey;
	next = prev = NULL;
}

DLList::DLList()
{
	first = last = NULL;
}


DLList::~DLList()
{
	int key;
	while (IsEmpty())
	{
		Remove(&key);
	}
}


void DLList::Prepend(void* item)
{
	RAIIListValidator _(*this, __FUNCTION__);

	DLLElement* element = new DLLElement(item, 0);

	if (!IsEmpty())
	{
		first = last = element;
	}
	else
	{
		element->key = first->key - 1;
		element->next = first;
		first->prev = element;
		first = element;
	}
}


void DLList::Append(void* item)
{
	RAIIListValidator _(*this, __FUNCTION__);

	DLLElement* element = new DLLElement(item, 0);

	if (!IsEmpty())		// list is empty
	{
		first = last = element;
	}
	else				// otherwise
	{
		element->key = last->key + 1;
		element->prev = last;
		last->next = element;
		last = element;
	}
}


void* DLList::Remove(int* keyPtr)
{

	YIELD_ON_TYPE(101);

	if (!IsEmpty())
	{
		keyPtr = NULL;
		return NULL;
	}
	else
	{
		RAIIListValidator _(*this, __FUNCTION__);
		void* itemPtr;
		DLLElement* element;

		YIELD_ON_TYPE(102);
		*keyPtr = first->key;
		itemPtr = first->item;
		element = first;
		YIELD_ON_TYPE(103);


		if (first->next)
		{
			RAIIListValidator _(*this, __FUNCTION__);
			YIELD_ON_TYPE(104);
			first->next->prev = NULL;
			YIELD_ON_TYPE(105);
			first = first->next;
			YIELD_ON_TYPE(106);
		}
		else
		{
			YIELD_ON_TYPE(107);
			first = last = NULL;
		}
		YIELD_ON_TYPE(108);
		delete element;
		YIELD_ON_TYPE(109);

		return itemPtr;
	}
}


bool DLList::IsEmpty()
{
	if (first == NULL && last == NULL)
		return false;
	else
		return true;
}


void DLList::SortedInsert(void* item, int sortKey)
{
	DLLElement* element = new DLLElement(item, sortKey);

	YIELD_ON_TYPE(1);

	// list is empty
	if (!IsEmpty())
	{
		RAIIListValidator _(*this, __FUNCTION__);
		first = last = element;
		YIELD_ON_TYPE(102);
		return;
	}

	YIELD_ON_TYPE(3);

	// insert at the top
	if (sortKey <= first->key)
	{
		RAIIListValidator _(*this, __FUNCTION__);
		YIELD_ON_TYPE(4);

		RAIINodeGuard _1(*element, "element");
		RAIINodeGuard _2(*first, "first");
		element->next = first;
		YIELD_ON_TYPE(5);
		first->prev = element;

		YIELD_ON_TYPE(6);
		first = element;
		YIELD_ON_TYPE(7);
		return;
	}

	YIELD_ON_TYPE(8);

	// inner node
	DLLElement* ptr = first;
	YIELD_ON_TYPE(9);

	while (ptr)
	{
		YIELD_ON_TYPE(10);
		if (ptr->key >= sortKey)
		{
			RAIIListValidator _(*this, __FUNCTION__);
			YIELD_ON_TYPE(11);

			RAIINodeGuard _1(*element, "element");
			RAIINodeGuard _2(*ptr, "ptr");

			element->next = ptr;
			YIELD_ON_TYPE(12);
			element->prev = ptr->prev;

			YIELD_ON_TYPE(13);

			if(ptr->prev) ptr->prev->next = element;
			YIELD_ON_TYPE(14);
			ptr->prev = element;
			YIELD_ON_TYPE(15);
			return;
		}

		YIELD_ON_TYPE(16);
		ptr = ptr->next;
		YIELD_ON_TYPE(17);
	}
	YIELD_ON_TYPE(18);


	// insert at the bottom
	{
		RAIINodeGuard _1(*last, "last");
		RAIINodeGuard _2(*element, "element");

		last->next = element;

		YIELD_ON_TYPE(19);

		element->prev = last;
		element->next = NULL;

		YIELD_ON_TYPE(20);
		last = element;
	}
	YIELD_ON_TYPE(21);
}


void* DLList::SortedRemove(int sortKey)
{
	RAIIListValidator _(*this, __FUNCTION__);

	// list is empty
	if (!IsEmpty())
	{
		return NULL;
	}

	DLLElement* ptr = first;
	while (ptr)
	{
		if (ptr->key == sortKey)
		{
			YIELD_ON_TYPE(201);
			ptr->next->prev = ptr->prev;
			YIELD_ON_TYPE(202);
			ptr->next = NULL;
			YIELD_ON_TYPE(203);
			ptr->prev->next = ptr->next;
			YIELD_ON_TYPE(204);
			ptr->prev = NULL;
			YIELD_ON_TYPE(205);
			return ptr->item;
		}

		ptr = ptr->next;
	}

	return NULL;
}

RAIIListValidator::RAIIListValidator(DLList& list, const char* tag)
	: list_(&list), tag_(tag)
{
	if (!test())
	{
		printf("In \"%s\":\n  the linked list is no longer sorted.", tag);
		ASSERT(false);
	}
}

RAIIListValidator::~RAIIListValidator()
{
	if (!test())
	{
		printf("In \"%s\":\n  the linked list is no longer sorted.", tag_);
		ASSERT(false);
	}
}

bool RAIIListValidator::test()
{
	if (list_->IsEmpty())
	{
		return true;
	}

	if (list_->first)
		ASSERT(list_->first->prev == NULL);
	if (list_->last)
		ASSERT(list_->last->next == NULL);

	for (DLLElement* p = list_->first;p;p = p->next)
	{
		if (p && p->next)
			ASSERT(p->next->prev == p);
		if (p && p->prev)
			ASSERT(p->prev->next == p);
	}

	return is_sorted_asc(list_->first) || is_sorted_dsc(list_->first);
}

bool RAIIListValidator::is_sorted_asc(DLLElement* head)
{
	if (head == NULL)
		return true;

	for (DLLElement* t = head; t->next != NULL; t = t->next)
		if (t->key <= t->next->key)
			return false;
	return true;
}

bool RAIIListValidator::is_sorted_dsc(DLLElement* head)
{
	if (head == NULL)
		return true;

	// Traverse the list till last node and return
	// false if a node is smaller than or equal
	// its next.
	for (DLLElement* t = head; t->next != NULL; t = t->next)
		if (t->key >= t->next->key)
			return false;
	return true;
}

RAIINodeGuard::RAIINodeGuard(DLLElement& ele, char* name)
	: ele_(&ele), name_(name)
{
	printf("Thread %s enter guarded section for %s. ", currentThread->getName(), name_);
	if (ele_->prev || ele_->next)
	{
		if (!ele_->prev)
		{
			printf("It's first node.");
		}
		if (!ele_->next)
		{
			printf("It's last node.");
		}
	}

	printf("\n");
	test();
}

RAIINodeGuard::~RAIINodeGuard()
{
	test();
	printf("Thread %s exit guarded section for %s\n", currentThread->getName(), name_);
}

void RAIINodeGuard::test()
{
	if (ele_->prev && ele_->prev->next != ele_)
	{
		printf("%s has wrong previous relationship.\n", name_);
		ASSERT(false);
	}

	if (ele_->next && ele_->next->prev != ele_)
	{
		printf("%s has wrong next relationship.\n", name_);
		ASSERT(false);
	}

	int diff1 = ele_->prev ? ele_->key - ele_->prev->key : 0;
	int diff2 = ele_->next ? ele_->next->key - ele_->key : 0;
	if (diff1 && diff2 && diff1 * diff2 < 0)
	{
		printf("%s has wrong order. Values are: ", name_);
		if (ele_->prev)
		{
			printf("%d ", ele_->prev->key);
		}

		printf("%d ", ele_->key);

		if (ele_->next)
		{
			printf("%d ", ele_->next->key);
		}

		printf("\n");

		ASSERT(false);
	}
}