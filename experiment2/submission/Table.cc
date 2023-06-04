#include "system.h"
#include "Table.h"

Table::Table(int size):size_(size) {
    items_ = new void* [size];
    lock_ = new Lock("tablelock");
}

Table::~Table()
{
    delete[] items_;
    delete lock_;
}

int Table::Alloc(void *object)
{
    LockGuard _(lock_);
    for (int i = 0;i < size_;i++)
    {
        if (!items_[i])
        {
            items_[i] = object;
            return i;
        }
    }
    return -1;
}

void* Table::Get(int index)
{
    if (index < 0 || index >= size_)
    {
        return NULL;
    }

    LockGuard _(lock_);
    return items_[index];
}

void Table::Release(int index)
{
    LockGuard _(lock_);
    items_[index] = NULL;
}

