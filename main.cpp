#include <iostream>
#include "MemClient.h"
#include "MemManager.h"
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <map>
using namespace std;


int main(int argc, char *argv[])
{
	CMemManager mempool;
	CMemClient cache(&mempool, 1024*1024);

	char str[] = "Hello BOYAA";
	cache.Append(str, strlen(str));


	return 0;
}