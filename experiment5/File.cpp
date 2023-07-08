#include "File.h"
#include "Disk.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PATHMAX 1024
dirTable* rootDirTable; //根目录
dirTable* currentDirTable;  //当前位置
char path[PATHMAX]; //保存当前绝对路径

//初始化根目录
void initRootDir()
{
	//分配一个盘块空间给rootDirTable
	int startBlock = getBlock(1);
	char parent[] = "..";
	if (startBlock == -1)
		return;
	rootDirTable = (dirTable*)getBlockAddr(startBlock);
	rootDirTable->dirUnitAmount = 0;
	//仿照Linux，设置根目录的父目录仍为根目录
	addDirUnit(rootDirTable, parent, 0, startBlock);
	currentDirTable = rootDirTable;
	//初始化初始绝对路径
	path[0] = '/';
	path[1] = '\0';
}

//获得当前绝对路径
char* getPath()
{
	return path;
}

//展示当前目录 ls
void showDir()
{
	int unitAmount = currentDirTable->dirUnitAmount;
	printf("total:%d\n", unitAmount);
	printf("name\ttype\tsize\tFCB\tdataStartBlock\n");
	//遍历所有表项
	for (int i = 0; i < unitAmount; i++)
	{
		//获取目录项
		dirUnit unitTemp = currentDirTable->dirs[i];
		printf("%s\t%s\t", unitTemp.fileName, unitTemp.type == 0 ? "dir" : "file");
		//该表项是文件，继续输出大小和起始盘块号
		if (unitTemp.type == 1)
		{
			int FCBBlock = unitTemp.startBlock;
			FCB* fileFCB = (FCB*)getBlockAddr(FCBBlock);
			printf("%d\t%d\t%d\n", fileFCB->fileSize, FCBBlock, fileFCB->dataStartBlock);
		}
		else
		{
			int dirBlock = unitTemp.startBlock;
			dirTable* myTable = (dirTable*)getBlockAddr(dirBlock);
			printf("%d\t%d\t-\n", myTable->dirUnitAmount, unitTemp.startBlock);
		}
	}
}

//切换目录 cd
int changeDir(char dirName[])
{
	//目录项在目录位置
	int unitIndex = findUnitInTable(currentDirTable, dirName);
	//不存在
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	//判断目标文件是否为目录
	if (currentDirTable->dirs[unitIndex].type == 1)
	{
		printf("not a dir\n");
		return -1;
	}
	//修改当前目录
	int dirBlock = currentDirTable->dirs[unitIndex].startBlock;
	currentDirTable = (dirTable*)getBlockAddr(dirBlock);
	//修改全局绝对路径
	//返回到上一层的情况
	if (strcmp(dirName, "..") == 0)
	{
		//回退绝对路径
		int len = strlen(path);
		for (int i = len - 2; i >= 0; i--)
			if (path[i] == '/')
			{
				path[i + 1] = '\0';
				break;
			}
	}
	else
	{
		//进入下一级目录的情况
		strcat(path, dirName);
		strcat(path, "/");
	}
	return 0;
}

//修改文件名或者目录名 mv
int changeName(char oldName[], char newName[])
{
	int unitIndex = findUnitInTable(currentDirTable, oldName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	strcpy(currentDirTable->dirs[unitIndex].fileName, newName);
	return 0;
}


//创建文件 touch
int creatFile(char fileName[], int fileSize)
{
	//检测文件名字长度
	if (strlen(fileName) >= FILENAME_MAX_LENGTH)
	{
		printf("file name too long\n");
		return -1;
	}
	//获得FCB的空间
	int FCBBlock = getBlock(1);
	if (FCBBlock == -1)
		return -1;
	//获取文件数据空间
	int FileBlock = getBlock(fileSize);
	if (FileBlock == -1)
		return -1;
	//创建FCB
	if (creatFCB(FCBBlock, FileBlock, fileSize) == -1)
		return -1;
	//添加到目录项
	if (addDirUnit(currentDirTable, fileName, 1, FCBBlock) == -1)
		return -1;
	return 0;
}

//创建目录 mkdir
int creatDir(char dirName[])
{
	if (strlen(dirName) >= FILENAME_MAX_LENGTH)
	{
		printf("file name too long\n");
		return -1;
	}
	//为目录表分配空间
	int dirBlock = getBlock(1);
	if (dirBlock == -1)
		return -1;
	//将目录作为目录项添加到当前目录
	if (addDirUnit(currentDirTable, dirName, 0, dirBlock) == -1)
		return -1;
	//为新建的目录添加一个到父目录的目录项
	dirTable* newTable = (dirTable*)getBlockAddr(dirBlock);
	newTable->dirUnitAmount = 0;
	char parent[] = "..";
	if (addDirUnit(newTable, parent, 0, getAddrBlock((char*)currentDirTable)) == -1)
		return -1;
	return 0;
}

//创建FCB
int creatFCB(int fcbdataStartBlock, int filedataStartBlock, int fileSize)
{
	//找到fcb的存储位置
	FCB* currentFCB = (FCB*)getBlockAddr(fcbdataStartBlock);
	currentFCB->dataStartBlock = filedataStartBlock;//文件数据起始位置
	currentFCB->fileSize = fileSize;//文件大小
	currentFCB->link = 1;//文件链接数
	currentFCB->dataSize = 0;//文件已写入数据长度
	currentFCB->readPtr = 0;//文件读指针
	currentFCB->read_sem = sem_open("read_sem", O_CREAT, 0644, READER_MAX_NUM);
	if (currentFCB->read_sem == SEM_FAILED)
	{
		perror("sem_open error");
		exit(1);
	}
	currentFCB->write_sem = sem_open("write_sem", O_CREAT, 0644, 1);
	if (currentFCB->write_sem == SEM_FAILED)
	{
		perror("sem_open error");
		exit(1);
	}
	currentFCB->read_sem2 = sem_open("read_sem2", O_CREAT, 0644, 1);
	if (currentFCB->read_sem2 == SEM_FAILED)
	{
		perror("sem_open error");
		exit(1);
	}
	return 0;
}

//添加目录项
int addDirUnit(dirTable* myDirTable, char fileName[], int type, int FCBdataStartBlock)
{
	//获得目录表
	int dirUnitAmount = myDirTable->dirUnitAmount;
	//检测目录表是否已满
	if (dirUnitAmount == DIR_TABLE_MAX_SIZE)
	{
		printf("dirTables is full, try to delete some file\n");
		return -1;
	}
	//是否存在同名文件
	if (findUnitInTable(myDirTable, fileName) != -1)
	{
		printf("file already exist\n");
		return -1;
	}
	//构建新目录项
	dirUnit* newDirUnit = &myDirTable->dirs[dirUnitAmount];
	myDirTable->dirUnitAmount++;//当前目录表的目录项数量+1
	//设置新目录项内容
	strcpy(newDirUnit->fileName, fileName);
	newDirUnit->type = type;
	newDirUnit->startBlock = FCBdataStartBlock;
	return 0;
}

//删除文件 rm
int deleteFile(char fileName[])
{
	//忽略系统的自动创建的父目录
	if (strcmp(fileName, "..") == 0)
	{
		printf("can't delete ..\n");
		return -1;
	}
	//查找文件的目录项位置
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//判断类型
	if (myUnit.type == 0)//目录
	{
		printf("not a file\n");
		return -1;
	}
	int FCBBlock = myUnit.startBlock;
	//释放内存
	releaseFile(FCBBlock);
	//从目录表中剔除
	deleteDirUnit(currentDirTable, unitIndex);
	return 0;
}

//释放文件内存
int releaseFile(int FCBBlock)
{
	FCB* myFCB = (FCB*)getBlockAddr(FCBBlock);
	myFCB->link--;  //链接数减一
	//无链接，删除文件
	if (myFCB->link == 0)
	{
		//释放文件的数据空间
		releaseBlock(myFCB->dataStartBlock, myFCB->fileSize);
	}
	//释放FCB的空间
	sem_close(myFCB->read_sem);
	sem_close(myFCB->write_sem);
	sem_close(myFCB->read_sem2);
	sem_unlink("read_sem");
	sem_unlink("write_sem");
	sem_unlink("read_sem2");
	//释放FCB空间
	releaseBlock(FCBBlock, 1);
	return 0;
}

//删除目录项
int deleteDirUnit(dirTable* myDirTable, int unitIndex)
{
	//迁移覆盖
	int dirUnitAmount = myDirTable->dirUnitAmount;
	for (int i = unitIndex; i < dirUnitAmount - 1; i++)
	{
		myDirTable->dirs[i] = myDirTable->dirs[i + 1];
	}
	myDirTable->dirUnitAmount--;
	return 0;
}

//删除目录 rmdir
int deleteDir(char dirName[])
{
	//忽略系统的自动创建的父目录
	if (strcmp(dirName, "..") == 0)
	{
		printf("can't delete ..\n");
		return -1;
	}
	//查找文件
	int unitIndex = findUnitInTable(currentDirTable, dirName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return -1;
	}
	dirUnit myUnit = currentDirTable->dirs[unitIndex];
	//判断类型
	if (myUnit.type == 0)//目录
	{
		deleteFileInTable(currentDirTable, unitIndex);
	}
	else
	{
		printf("not a dir\n");
		return -1;
	}
	//从目录表中剔除
	deleteDirUnit(currentDirTable, unitIndex);
	return 0;
}

//删除文件/目录项
int deleteFileInTable(dirTable* myDirTable, int unitIndex)
{
	//查找文件
	dirUnit myUnit = myDirTable->dirs[unitIndex];
	//判断类型
	if (myUnit.type == 0)//目录
	{
		//找到目录位置
		int FCBBlock = myUnit.startBlock;
		dirTable* table = (dirTable*)getBlockAddr(FCBBlock);
		//递归删除目录下的所有文件
		printf("cycle delete dir %s\n", myUnit.fileName);
		int unitCount = table->dirUnitAmount;
		for (int i = 1; i < unitCount; i++)//忽略“..”
		{
			printf("delete %s\n", table->dirs[i].fileName);
			deleteFileInTable(table, i);
		}
		//释放目录表空间
		releaseBlock(FCBBlock, 1);
	}
	else
	{//文件
		//释放文件内存
		int FCBBlock = myUnit.startBlock;
		releaseFile(FCBBlock);
	}
	return 0;
}

//打开文件，获得控制块
FCB* open(char fileName[])
{
	int unitIndex = findUnitInTable(currentDirTable, fileName);
	if (unitIndex == -1)
	{
		printf("file not found\n");
		return NULL;
	}
	//控制块
	int FCBBlock = currentDirTable->dirs[unitIndex].startBlock;
	FCB* myFCB = (FCB*)getBlockAddr(FCBBlock);
	return myFCB;
}

//读文件
int read_file(char fileName[], int length)
{
	//打开文件，获得控制块
	FCB* myFCB = open(fileName);
	myFCB->readPtr = 0; //文件指针重置
	//读数据
	char* data = (char*)getBlockAddr(myFCB->dataStartBlock);
	int val;
	myFCB->read_sem = sem_open("read_sem", 0);
//	 获取记录读者数量的锁 
	myFCB->read_sem2 = sem_open("read_sem2", 0);
	sem_wait(myFCB->read_sem2);
	if (sem_wait(myFCB->read_sem) == -1)
		perror("sem_wait error");
	sem_getvalue(myFCB->read_sem, &val);
//	 根据拥有锁的进程数量来判断是否是第一个读者 
//	 如果是第一个读者就负责锁上写者锁 
	if (val == READER_MAX_NUM - 1)
	{
		myFCB->write_sem = sem_open("write_sem", 0);
		if (sem_wait(myFCB->write_sem) == -1)
			perror("sem_wait error");
	}
	sem_post(myFCB->read_sem2);
	int dataSize = myFCB->dataSize;
//	 printf("myFCB->dataSize = %d\n", myFCB->dataSize); 
	//在不超出数据长度下，读取指定长度的数据
	for (int i = 0; i < length && myFCB->readPtr < dataSize; i++, myFCB->readPtr++)
	{
		printf("%c", *(data + myFCB->readPtr));
	}
//	 下面两行只是为了模拟编辑器的关闭之前的情况， 
//	 这样就能控制进程不会立即释放锁 
	printf("\n> Read finished, press any key to continue....");
	getchar();
//	 如果是最后一个读者就负责释放写者锁 
	//这里进行了更改，旧：READER_MAX_NUM；新：READER_MAX_NUM-1 By. LeoHao
	sem_wait(myFCB->read_sem2);
	if (val == READER_MAX_NUM - 1)
	{
		sem_post(myFCB->write_sem);
	}
	sem_post(myFCB->read_sem);
	sem_post(myFCB->read_sem2);
	return 0;
}

//写文件，从末尾写入 write
int write_file(char fileName[], char content[])
{
	//打开文件，获得控制块
	FCB* myFCB = open(fileName);
	int contentLen = strlen(content);
	int fileSize = myFCB->fileSize * block_size;
	char* data = (char*)getBlockAddr(myFCB->dataStartBlock);
	myFCB->write_sem = sem_open("write_sem", 0);
//	 获得写者锁 
	if (sem_wait(myFCB->write_sem) == -1)
		perror("sem_wait error");
	//在不超出文件的大小的范围内写入
	for (int i = 0; i < contentLen && myFCB->dataSize < fileSize; i++, myFCB->dataSize++)
	{
		*(data + myFCB->dataSize) = content[i];
	}
	// 模拟编辑器,控制写者不立即退出
	printf("> Write finished, press any key to continue....");
	getchar();
	// 释放写者锁
	sem_post(myFCB->write_sem);
	if (myFCB->dataSize == fileSize)
		printf("file is full, can't write in\n");
	return 0;
}

//从目录中查找目录项目
int findUnitInTable(dirTable* myDirTable, char unitName[])
{
	//获得目录表
	int dirUnitAmount = myDirTable->dirUnitAmount;
	int unitIndex = -1;
	for (int i = 0; i < dirUnitAmount; i++)//查找目录项位置
		if (strcmp(unitName, myDirTable->dirs[i].fileName) == 0)
			unitIndex = i;
	return unitIndex;
}
