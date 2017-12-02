#include "MemClient.h"


CMemClient::CMemClient(CMemManager *pMemManager, uint blockSize):m_pMemManager(pMemManager)
{
	if (m_pMemManager == NULL)
	{
		std::cerr << "init memmanager fail" << std::endl;
		return;
	}

	m_blockSize = m_dataLen = m_skipLen = 0;
	if (blockSize > 0)
	{
		m_memAddr = (char*)m_pMemManager->MemAllocate(blockSize, m_blockSize);
		if (m_memAddr == NULL)
		{
			std::cerr <<" MemAllocate fail " << std::endl;
		}
	}
}

CMemClient::~CMemClient()
{
	this->release();
}

char *CMemClient::Data(void)
{
	if (m_memAddr == NULL)
		return NULL;

	return this->m_memAddr + m_skipLen;			// 数据可能会跳跃
}

uint CMemClient::Len(void)
{
	return this->m_dataLen;
}

int CMemClient::Copy(char *data, uint len)
{
	if (m_memAddr == NULL || data == NULL || len == 0 || len > m_blockSize)
		return -1;

	memcpy(m_memAddr, data, len);
	m_dataLen = len;
	m_skipLen = 0;

	return 0;
}

int CMemClient::Append(char *data, uint len)
{
	if (m_memAddr == NULL || data == NULL || len == 0)
		return -1;

	// 目前已经使用的内存空间下标
	const uint useLen = m_skipLen + m_dataLen;

	// 如果后面还有空间存放 len，则继续追加
	if (useLen + len <= m_blockSize)
	{
		memcpy(m_memAddr+useLen, data, len);
		m_dataLen += len;
		return 0;
	}
	// else 后面剩余空间不够了

	// 因为 useLen 有 skipLen, 也就是前面 m_memAddr+skipLen 这段空间是没有使用的，尝试挪动后空间是否足够
	if (m_dataLen + len <= m_blockSize)
	{
		// 将空间往前移
		memmove(m_memAddr, m_memAddr+m_skipLen, m_dataLen);
		memcpy(m_memAddr+m_dataLen, data, len);
		m_dataLen += len;
		return 0;
	}
	// else 前面的空间即使利用起来，也不够存放，那就只能重新申请大内存了

	if (m_dataLen + len > m_blockSize)
	{
		const uint newAllocSize = 2 * (m_dataLen + len);
		uint newBlockSize = 0;
		char *newMemAddr = (char*)m_pMemManager->MemAllocate(newAllocSize, newBlockSize);
		if (newMemAddr == NULL)
		{
			std::cerr <<" MemAllocate fail " << std::endl;
			return -1;
		}

		// 拷贝旧数据到 newMemAddr
		memcpy(newMemAddr, m_memAddr+m_skipLen, m_dataLen);
		memcpy(newMemAddr+m_dataLen, data, len);

		// 回收旧内存
		m_pMemManager->MemRecycle(m_memAddr, m_blockSize);

		// 更新参数
		m_dataLen += len;
		m_skipLen = 0;
		m_blockSize = newBlockSize;
		m_memAddr = newMemAddr;
	}

	return 0;
}


int CMemClient::Skip(uint toSkiplen)
{
	// 如果跳过的长度超过数据长度，表明这段数据没有用了，直接从零开始
	if (toSkiplen >= m_dataLen)
	{
		this->Clear();
	}
	else
	{
		m_skipLen += toSkiplen;
		m_dataLen -= toSkiplen;
	}

	return 0;
}

int CMemClient::Clear(void)
{
	m_dataLen = 0;
	m_skipLen = 0;
	memset(m_memAddr, 0, m_blockSize);
	return 0;
}

// 这个函数只会在析构的时候被调用
int CMemClient::release(void)
{
	m_pMemManager->MemRecycle(m_memAddr, m_blockSize);
	m_dataLen = 0;
	m_skipLen = 0;
	m_memAddr = NULL;
	return 0;
}



int CMemClient::Test(void)
{
	printf("Check m_blockSize=%u, datalen=%u, skip=%u\n",m_blockSize, m_dataLen, m_skipLen);	
	return 0;
}

