// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "dllist.h"
#include "hello.h"

// functions in dllist-driver.cc
extern void Insert(DLList* L, int N, int whichThread);
extern void Remove(DLList* L, int N, int whichThread);

// testnum is set in main.cc
int testnum = 1;

// those are set in main.cc
int threadnum = 16;	// number of threads, default: 2
int itemnum = 6;	// number of items, default: 6
int errorType = 0;	// type of error

// global, pointer to doubly-linked list
DLList* L;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;

    for (num = 0; num < 5; num++) {
        printf("*** thread %d looped %d times\n", which, num);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// SimpleThread2
// 	Manipulate doubly-linked list here.
//----------------------------------------------------------------------

void
SimpleThread2(int which)
{
    Insert(L, itemnum, which);
    Remove(L, itemnum, which);
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread* t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest2
//  Create a doubly-linked list by calling Insert first, 
//  and then remove items at the top of the list by those threads.
//----------------------------------------------------------------------

char name[64] = { 0 };

void
ThreadTest2()
{
    DEBUG('t', "Entering ThreadTest2");

    // allocate the list
    L = new DLList();

    // fork thread
    for (int i = 1; i < threadnum; ++i)
    {
        memset(name, 0, sizeof(name));
        snprintf(name, 64, "thread %d", i);
        Thread* t = new Thread(name);
        t->Fork(SimpleThread2, i);
    }
    SimpleThread2(0);
}

#include "Table.h"

extern "C" int rand();

Table table(5);

void TestTableHandle(int which)
{
    int* object = new int, index;
    *object = rand();
    printf("add object %d to table in thread %d\n", *object, which);
    index = table.Alloc((void*)object);
    if (index != -1) {
        ASSERT(((int*)table.Get(index)) == object);
        printf("get object %d to table in thread %d\n", *(int*)(table.Get(index)), which);
        currentThread->Yield();
        table.Release(index);
    }
}

void TestTable()
{
    printf("Test Table Class");

    // fork thread
    for (int i = 1; i < threadnum; ++i)
    {
        memset(name, 0, sizeof(name));
        snprintf(name, 64, "thread %d", i);
        Thread* t = new Thread(name);
        t->Fork(TestTableHandle, i);
    }
    TestTableHandle(0);
}

#include "BoundedBuffer.h"

BoundedBuffer *bb;

void TestBufferHandle(int which)
{
    int* buf = new int[16];
    // one is write and others is consumer the data 
    if (which == 1) {
        printf("thread %d put 15 elements:", which);
        for (int i = 0;i < 16;i++)
        {
            buf[i] = rand();
            printf(" %d", buf[i]);
        }
        printf("\n");
        bb->Write(buf, 15);
    }
    else {
        printf("thread %d will get %d elements\n", which,which-1);
        bb->Read((void*)buf, which - 1);
        printf("the datas from buffer in thread %d\n", which);
        for (int i = 0; i < which - 1; i++) {
            printf("%d ", buf[i]);
        }
        printf("\n");
    }
    delete[] buf;
}

void TestBuffer()
{
    bb = new BoundedBuffer(16);
    // fork thread
    for (int i = 1; i < threadnum; ++i)
    {
        memset(name, 0, sizeof(name));
        snprintf(name, 64, "thread %d", i);
        Thread* t = new Thread(name);
        t->Fork(TestBufferHandle, i);
    }
    TestBufferHandle(0);
}


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    hello();
    switch (testnum) {
    case 1:
        ThreadTest1();
        break;
    case 2:
        ThreadTest2();	// test for doubly-linked list goes here
        break;
    case 3:
        TestTable();
    case 4:
        TestBuffer();
    default:
        printf("No test specified.\n");
        break;
    }
}
