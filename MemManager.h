// +-----------------------------------------------------------------------+
// | MemManager                                                            |
// +-----------------------------------------------------------------------+
// | Author: linyubin                                                      |
// +-----------------------------------------------------------------------+
// | Date: 2017-12-02                                                      |
// +-----------------------------------------------------------------------+
// | Desc:                                                                 |
// | 1. CMemManager 采用空闲块链表的方式进行内存池管理，这部分的设计主要   |
// | 考虑到游戏数据包的频繁交互，需要有内存空间来存储接收的数据包数据      |
// +-----------------------------------------------------------------------+




#ifndef __MEMMANAGER_H__
#define __MEMMANAGER_H__

#include <list>
#include <map>
#include <set>
#include <algorithm>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <unistd.h>
#include <iostream>

typedef unsigned int uint;
typedef std::map<uint, std::list<void*> > mlist;
typedef std::map<uint, std::set<void*> > mset;

const static uint MAX_ALLOC_SIZE = 1 << 29;			// 单次申请的最大内存大小 512M
const static uint MAX_BLOCKLIST_COUNT = 1 << 12;	// 空闲链表的最大初始化个数  4096
// const static uint MAX_ALLOC_WATER_LEVEL = 1 << 29;	// 当前需要调整内存块的最大水位 512M，也就是总分配内存达512M时，释放大内存给小内存用,避免内存进一步扩大

class CMemManager
{
public:
	CMemManager(uint allocWaterLevel=1<<29);		// 默认值为 1<<29, 也就是 512M
	~CMemManager();

private:
	CMemManager(const CMemManager &other);

public:
	// 仅提供分配和回收两个接口即可
	void *MemAllocate(uint allocSize, uint &fitBlockSize);
	int MemRecycle(void *m_memAddr, const uint blockSize);

public:
	int Debug(void);								// 查看成员信息，调试用

private:
	uint fit_block_size(uint allocSize);
	int init_block_list(uint blockSize);
	uint extend_block_count(uint blockSize);
	int extend_block_list(uint blockSize);
	uint reduce_block_count(uint blockSize);
	int reduce_block_list(uint blockSize);
	int release_large_block(uint curBlockSize);

private:
	mlist m_blockFree;								// 当前空闲内存块链表, 有点类似于 hash 的拉链处理
	mset m_allocStub;								// 记录所有从系统申请的内存
	uint m_allocSize;								// 记录从系统申请的内存总大小
	uint m_alloc_water_level;						// 系统内存分配水位，申请的内存数超过该水位时，进行内存拆补，释放掉大块内存，供给小内存分配
};

#endif
