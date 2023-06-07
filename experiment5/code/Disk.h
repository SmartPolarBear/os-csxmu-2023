//
// Created by bear on 6/7/2023.
//

#ifndef DISKOPERATE_H_INCLUDED
#define DISKOPERATE_H_INCLUDED
#define system_size 100*1024*1024           //系统大小
#define block_size 1024                     //盘块大小
#define block_count system_size/block_size  //系统盘块数目
/* 初始化文件系统 */
void initFileSystem();
/* 磁盘分配 */
int getBlock(int blockSize);
/* 获得盘块的物理地址 */
char *getBlockAddr(int blockNum);
/* 获得物理地址的盘块号 */
int getAddrBlock(char *addr);
/* 释放盘块、 */
int releaseBlock(int blockNum, int blockSize);
/* 退出系统 */
void exitSystem();
#endif