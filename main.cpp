#include <iostream>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <map>

#include "MemClient.h"
#include "MemManager.h"


using namespace std;

void test1(void)
{
	CMemManager mempool;
	CMemClient tmp(&mempool,   1024*512);
	CMemClient cache(&mempool, 1024*1024);

	mempool.Debug();
	for(int i=0; i<10; ++i)
	{
		CMemClient *t = new CMemClient(&mempool, 1024*512);
		// delete t;				// 这里如果不释放，这块内存无法再分配出去，直到 CMemManager 对象销毁，才回收
	}
	mempool.Debug();				// 总共向系统申请 12M内存
}

void test2(void)
{
	// 手动设置内存申请水位阈值，测试是否会拆掉大内存进行弥补
	CMemManager mempool(1024*512*9);
	CMemClient tmp(&mempool,   1024*512);
	CMemClient cache(&mempool, 1024*1024);

	mempool.Debug();
	for(int i=0; i<10; ++i)
	{
		CMemClient *t = new CMemClient(&mempool, 1024*512);
		// delete t;
	}
	mempool.Debug();				// 总共向系统申请 11M内存， 相比 test1, 有效减少1M的内存申请
}


int main(int argc, char *argv[])
{
	test1();

	test2();

	return 0;
}