// +-----------------------------------------------------------------------+
// | MemManager                                                            |
// +-----------------------------------------------------------------------+
// | Author: linyubin                                                      |
// +-----------------------------------------------------------------------+
// | Date: 2017-11-30                                                      |
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

typedef unsigned int uint;
typedef std::map<uint, std::list<void*> > mlist;
typedef std::map<uint, std::set<void*> > mset;

const static uint MAX_ALLOC_SIZE = 1 << 29;			// 单次申请的最大内存大小 512M
const static uint MAX_BLOCKLIST_COUNT = 1 << 12;	// 空闲链表的最大初始化个数  4096


class CMemManager
{
public:
	CMemManager();
	~CMemManager();

public:
	// 仅提供分配和回收两个接口即可
	void *MemAllocate(uint allocSize, uint &fitBlockSize);
	int MemRecycle(void *m_memAddr, const uint blockSize);
	int Test(void);

private:
	uint fit_block_size(uint allocSize);
	int init_block_list(uint blockSize);
	uint extend_block_count(uint blockSize);
	int extend_block_list(uint blockSize);
	uint reduce_block_count(uint blockSize);
	int reduce_block_list(uint blockSize);

private:
	mlist m_blockFree;								// 当前空闲内存块链表, 有点类似于 hash 的拉链处理
	mset m_allocStub;								// 记录所有从系统申请的内存
	uint m_allocSize;								// 记录从系统申请的内存总大小
};

#endif
