#include "MemManager.h"

CMemManager::CMemManager()
{
	m_blockFree.clear();
	m_allocStub.clear();
	m_allocSize = 0;
}

CMemManager::~CMemManager()
{
	mset::iterator itSet = m_allocStub.begin();
	for(; itSet != m_allocStub.end(); ++itSet)
	{
		std::set<void*>::iterator it = itSet->second.begin();
		for(; it != itSet->second.end(); ++it)
		{
			// 内存分配采用 new char, 释放做类型转换,告知 delete 如何释放内存
			delete (char*)*it;
		}
	}
}


void *CMemManager::MemAllocate(uint allocSize, uint &fitBlockSize)
{
	if (allocSize == 0 || allocSize > MAX_ALLOC_SIZE)
		return NULL;

	// 获取合适大小内存块, KB的整数倍
	const uint fitSize = this->fit_block_size(allocSize);

	// 获取fitSize所在空闲链表
	mlist::iterator itBlock = m_blockFree.find(fitSize);
	if (itBlock == m_blockFree.end())
	{
		// 当前 blockSize 并没有分配过内存，需要进行空闲链表初始化
		const int ret = this->init_block_list(fitSize);
		if (ret != 0)
			return NULL;
	}
	std::list<void*> &freeList = m_blockFree[fitSize];

	// 检测当前空闲链表是否有空闲块
	if (freeList.empty())
	{
		const int ret = this->extend_block_list(fitSize);
		if (ret != 0)
			return NULL;
	}

	// 从空闲链表摘取一块，分配出去
	void *mem = freeList.front(); 
	freeList.pop_front();
	fitBlockSize = fitSize;
	return mem;
}


int CMemManager::MemRecycle(void *m_memAddr, const uint blockSize)
{
	mlist::iterator itBlock = m_blockFree.find(blockSize);
	if (itBlock == m_blockFree.end())
	{
		// 大小不符合，说明不是从该内存池分配出去的
		return -1;
	}

	// 回收添加到链表中
	std::list<void*> &freeList = m_blockFree[blockSize];
	freeList.push_back(m_memAddr);

	// 检测当前链表快是否过多，进行内存收缩
	this->reduce_block_list(blockSize);

	return 0;
}

//////////////////////////////////////////////////////////////////////////////////////

// 根据 allocSize 调整为 KB的整数倍
uint CMemManager::fit_block_size(uint allocSize)
{
	// 除去 1024  2^10 
	allocSize = allocSize >> 10;

	// 计算2的多少倍
	uint i = 0;
	while(allocSize > 1)
	{
		allocSize = allocSize >> 1;
		i++;
	}

	// 基准是 2^10;
	i += 10;	
	return 1 << i;
}

// m_blockFree 还没有链表对象，初始化空闲链表，同时扩展链表空闲块
int CMemManager::init_block_list(uint blockSize)
{
	// 利用 map 调用函数时，自动生成对象
	m_blockFree[blockSize].clear();
	m_allocStub[blockSize].clear();

	// 进行内存扩展
	return extend_block_list(blockSize);
}

// 根据 blockSize 计算需要预分配的块数，采用指数退让的做法，blockSize 越大，那么预分配的块数越少 
uint CMemManager::extend_block_count(uint blockSize)
{
	const uint i = std::max(1, (int)blockSize >> 10);
	const uint count = std::max(1, (int)(MAX_BLOCKLIST_COUNT/i) );
	return count;
}

// 当前blockSize链表为空，需要扩展链表空闲块
int CMemManager::extend_block_list(uint blockSize)
{
	const uint extend_count = this->extend_block_count(blockSize);

	std::list<void*> &freeList = m_blockFree[blockSize];
	std::set<void*> &allocSet = m_allocStub[blockSize];

	for(int i=0; i<extend_count; ++i)
	{
		void *mem = new char[blockSize];
		if (mem == NULL)
			return -1;

		// 清零初始化后，再分配出去
		memset(mem, 0, blockSize);

		freeList.push_back(mem);
		allocSet.insert(mem);
	}

	// 记录分配的大小
	m_allocSize += extend_count * blockSize;

	return 0;
}

// 计算当前 blockSize 所在的链表，应该释放掉多少块内存给系统，做内存收缩
// 采用的收缩规则是 空闲块大于初始分配的2倍以上，则做收缩
uint CMemManager::reduce_block_count(uint blockSize)
{
	// 获取初始分配大小
	const uint extend_count = this->extend_block_count(blockSize);
	const uint freelistSize = m_blockFree[blockSize].size();
	if (freelistSize > 2*extend_count)
		return freelistSize - extend_count;

	return 0;
}

// 内存收缩，归还给系统
int CMemManager::reduce_block_list(uint blockSize)
{
	const uint reduce_count = this->reduce_block_count(blockSize);

	std::list<void*> &freeList = m_blockFree[blockSize];
	std::set<void*> &allocSet = m_allocStub[blockSize];

	for(int i=0; i<reduce_count; ++i)
	{
		void *mem = freeList.front();
		freeList.pop_front();

		std::set<void*>::iterator it = allocSet.find(mem);
		if (it != allocSet.end())
			allocSet.erase(it);

		delete (char*)mem;
	}

	// 记录分配的大小
	m_allocSize -= reduce_count * blockSize;
	return 0;
}



int CMemManager::Test(void)
{
	printf("m_blockFree show............\n");
	mlist::iterator itMap = m_blockFree.begin();
	for(; itMap != m_blockFree.end(); ++itMap)
	{
		std::list<void*> &freeList = itMap->second;
		printf("blocksize=%u, count=%d\n", itMap->first, freeList.size());
		std::list<void*>::iterator itList = freeList.begin();
	}
	printf(".........................\n");

	printf("m_allocSize=%db, %dk, %dm, %dG \n", 
		m_allocSize, m_allocSize/1024, m_allocSize/(1024*1024), m_allocSize/(1024*1024*1024));
	mset::iterator itSet = m_allocStub.begin();
	for(; itSet != m_allocStub.end(); ++itSet)
	{
		const uint sizeb = itSet->first;
		const uint sizek = sizeb/1024;
		const uint sizem = sizek/1024;
		const uint sizeg = sizem/1024;
		printf("Stub blockSize=%uB %uK %uM %uG, count=%d\n", 
			sizeb, sizek, sizem, sizeg, itSet->second.size());
	} 

	return 0;
}