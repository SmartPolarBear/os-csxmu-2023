#include "system.h"
#include "BoundedBuffer.h"

// Implementation of BoundedBuffer constructor
BoundedBuffer::BoundedBuffer(int maxsize): maxsize_(maxsize), buffer_(new char[maxsize]), mutex_(new Lock("bblock")), not_empty_(new Condition("bbne")), not_full_(new Condition("bbnf")), head_(0), tail_(0), size_(0)
{
}

BoundedBuffer::~BoundedBuffer()
{
    delete[] buffer_;
}

// Implementation of BoundedBuffer Write method
void BoundedBuffer::Write(void* data, int size) {
    LockGuard g(mutex_);
    while (!(size_ < maxsize_))
    {
        not_full_->Wait(mutex_);
    }
    char* data_ptr = (char*)(data);
    for (int i = 0; i < size; i++) {
        buffer_[tail_] = data_ptr[i];
        tail_ = (tail_ + 1) % maxsize_;
        size_++;
    }
    not_empty_->Broadcast(mutex_);
}

// Implementation of BoundedBuffer Read method
void BoundedBuffer::Read(void* data, int size) {
    LockGuard g(mutex_);
    while (!(size_ >= size))
    {
        not_empty_->Wait(mutex_);
    }
    char* data_ptr = (char*)(data);
    for (int i = 0; i < size; i++) {
        data_ptr[i] = buffer_[head_];
        head_ = (head_ + 1) % maxsize_;
        size_--;
    }
    not_full_->Broadcast(mutex_);
}
