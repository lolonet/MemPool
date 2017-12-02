// +-----------------------------------------------------------------------+
// | MemClient                                                             |
// +-----------------------------------------------------------------------+
// | Author: linyubin                                                      |
// +-----------------------------------------------------------------------+
// | Date: 2017-11-30                                                      |
// +-----------------------------------------------------------------------+
// | Desc:                                                                 |
// | 1. MemClient 主要是对从内存池分配出来的一块内存块进行封装处理         |
// +-----------------------------------------------------------------------+




#ifndef __MEMCLIENT_H__
#define __MEMCLIENT_H__

#include "MemManager.h"
#include <iostream>
#include <cstdio>

typedef unsigned int uint;

class CMemManager;

class CMemClient
{
public:
	CMemClient(CMemManager *pMemManager, uint blockSize = 4 * 1024);		// 初始大小为一个页面
	~CMemClient();

private:
	CMemClient(CMemClient &other);

public:
	char *Data(void);
	uint Len(void);
	int Copy(char *data, uint len);
	int Append(char *data, uint len);
	int Skip(uint toSkiplen);
	int Clear(void);

	int Test(void);

private:
	int release(void);

private:
	CMemManager *m_pMemManager;							// 指向内存池
	char *m_memAddr;									// 内存基址
	uint m_blockSize;									// 实际内存快大小，实际的内存块，会比构造函数提供的大一点，凑满KB倍数
	uint m_dataLen;										// 已经使用的内存长度
	uint m_skipLen;										// 跳跃的长度，偏移量, 考虑业务读取的时候，可能会多次读取

};


#endif
