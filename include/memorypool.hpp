#ifndef _MEMORYPOOL_HPP_
#define _MEMORYPOOL_HPP_

#include <cstdlib>
#include <mutex>
#include "debug.h"

class CMemoryPool;

//内存块 最小单元
using pair_t = unsigned int;
typedef struct _Pair
{
    pair_t size;
	pair_t blockNum;

	_Pair(pair_t pSize, pair_t blockNumber)
	:size(pSize), blockNum(blockNumber){}
}Pair;

//内存块 最小单元
struct MemoryBlock
{
    union {
    //没有使用时指向下一内存块位置
    MemoryBlock* pNext;
    //使用时指向所属的内存池
    CMemoryPool* pool;
    };

    //引用次数
    int nRefCount;
    //是否属于内存池中
    bool bPool;
};

//内存池
class CMemoryPool
{
public:
	///@breif:初始化内存池
	///@param size:内存块的大小
	///@param blockNum:有多少个内存块
	CMemoryPool(size_t size, size_t blockNum) noexcept
    :_sizeOfBlock(size), _numOfBlock(blockNum){
		if (_sizeOfBlock == 0) {
			_poolBuf = nullptr;
			_pHeader = nullptr;
			return;
		}

        //计算内存单元的大小
        size_t realSize = _sizeOfBlock + sizeof(MemoryBlock);
		//向系统申请池内存
		_poolBuf = (char*)malloc(realSize * _numOfBlock);
		_pHeader = (MemoryBlock*)_poolBuf;
		//遍历内存块初始化MemoryBlock信息
		MemoryBlock* pTem = _pHeader;
		for (size_t i = 0; i < _numOfBlock; ++i) {
            //将所有的Block用链表串起来
		    pTem->pNext = (MemoryBlock*)((char*)pTem + realSize);
		    pTem->nRefCount = 0;
		    pTem->bPool = true;
            pTem = pTem->pNext;
		}
        //将最后一块MemoryBlock的pNext指向NULL
        pTem = (MemoryBlock*)((char*)pTem - realSize);
        pTem->pNext = nullptr;
	}
	//委托构造
	CMemoryPool(Pair& pair)
	:CMemoryPool(pair.size, pair.blockNum){}

	CMemoryPool() = default;

	CMemoryPool& operator=(CMemoryPool&& rhs){
		if(this != &rhs) {
			if(_poolBuf) {
				free(_poolBuf);
			}

			_poolBuf = rhs._poolBuf;
			_pHeader = rhs._pHeader;
			_sizeOfBlock = rhs._sizeOfBlock;
			_numOfBlock = rhs._numOfBlock;
			rhs._poolBuf = nullptr;
		}
		return *this;
	}

	~CMemoryPool() {
		if (_poolBuf) {
			free(_poolBuf);
			_poolBuf = NULL;
		}
	}

	//申请内存
	void* allocMemory(size_t size) {
		MemoryBlock* pReturn = nullptr;
		//加锁保护共享数据
		_mutex.lock();
		if (nullptr == _pHeader) {
			_mutex.unlock();
            //此时内存池中的内存已经用完了,需要通过malloc申请新的内存
			pReturn = (MemoryBlock*)malloc(size + sizeof(MemoryBlock));
			pReturn->bPool = false;
			pReturn->nRefCount = 1;
			pReturn->pool = this;
		} else {
            //返回pHeader指向的内存给用户用
			pReturn = _pHeader;
            //并把分配给用户的内存移出链表
			_pHeader = _pHeader->pNext;
			_mutex.unlock();
			pReturn->nRefCount = 1;
            //将分配出去的mBlock指向内存池方便使用完后再放回内存池
            pReturn->pool = this;
		}
		//COUT("mpool new:%u inMpool:%u\n", size, pReturn->bPool);
		return ((char*)pReturn + sizeof(MemoryBlock));
	}

	//释放内存
	void freeMemory(void* pMem) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		if (--pBlock->nRefCount !=0) {
			return;
		}
		//判断是内存池还是malloc出来的内存
		if (pBlock->bPool) {//内存池出来的放回池中
        	std::lock_guard<std::mutex> lock(_mutex);
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		} else {//malloc出来的free掉
			free(pBlock);
		}
	}

private:
	//内存池首地址
	char* _poolBuf;
	//头部内存单元
	MemoryBlock* _pHeader;
	//内存单元大小
	size_t _sizeOfBlock;
	//内存单元数量
	size_t _numOfBlock;
	std::mutex _mutex;
};

//内存池管理工具
class CMemoryManager
{
public:
	static CMemoryManager& Instance() {
		static CMemoryManager mgr;
		return mgr;
	}

	//申请内存
	void* allocMem(size_t size) {
		return GetPool(size)->allocMemory(size);
	}

	//释放内存
	void freeMem(void* pMem) {
		if(pMem == nullptr) {
			//COUT("nullptr:%p\n", pMem);
			return;
		}
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		//如果pMem属于内存池中的内存,交还内存池释放
		pBlock->pool->freeMemory(pMem);
	}

	//增加引用内存块计数
	void addRef(void* pMem) {
		MemoryBlock* pBlock = (MemoryBlock*)((char*)pMem - sizeof(MemoryBlock));
		++pBlock->nRefCount;
	}

private:
#if 0
	///@breif:_MemoryX(X,Y) X:内存单元块的大小 Y:申请多少块内存
	CMemoryManager():_Memory0(0,0),_Memory64(64,10240),
	_Memory128(128,2048), _Memory512(512,128),
	_Memory1024(1024, 64){
		//每个内存池可以申请到的最大内存再加一
		static unsigned int sizeArr[]  = {
			1, 65, 129, 513, 1025};
		//内存池地址索引
		static CMemoryPool* poolAddress[] = {
		nullptr, &_Memory64, &_Memory128, &_Memory512,
		&_Memory1024, &_Memory0};

		_poolSizeArr = sizeArr;
		_poolArr = poolAddress;
		_arrlength = sizeof(sizeArr)/sizeof(unsigned int);
	}
#endif
	///@breif:_MemoryX(X,Y) X:内存单元块的大小 Y:申请多少块内存
	CMemoryManager(){
		Pair pairArr[] = { Pair(0,0),
		Pair(64,10240),Pair(128,2048),
		Pair(512,128),Pair(1024, 64)};

		auto s = sizeof(pairArr)/sizeof(Pair);
		COUT("sizeOfpairArr:%ld\n", s);
		//每个内存池可以申请到的最大内存再加一
		static size_t sizeArr[sizeof(pairArr)/sizeof(Pair)] = {0};
		for (size_t i = 0; i < sizeof(pairArr)/sizeof(Pair); ++i) {
			sizeArr[i] = pairArr[i].size+1;
		}

		static CMemoryPool mPoolArr[sizeof(pairArr)/sizeof(Pair)];
		for (size_t j = 0; j < sizeof(pairArr)/sizeof(Pair); ++j) {
			mPoolArr[j] = CMemoryPool(pairArr[j]);
		}

		//内存池地址索引
		static CMemoryPool* poolAddress[sizeof(pairArr)/sizeof(Pair)];
		size_t k;
		for (k = 1; k < sizeof(pairArr)/sizeof(Pair)-1; ++k) {
			poolAddress[k] = &mPoolArr[k];
		}
		poolAddress[0] = nullptr;
		poolAddress[k] = &mPoolArr[0];

		_poolSizeArr = sizeArr;
		_poolArr = poolAddress;
		_arrlength = sizeof(pairArr)/sizeof(Pair);
	}


	///@breif:根据要申请的内存大小去匹配合适的内存池
	///@param size:要申请的内存大小
	inline CMemoryPool* GetPool(size_t size) const {
		size_t i;
		for (i = 0; i < _arrlength; ++i) {
			if(size < _poolSizeArr[i]) {
				break;
			}
		}
		return _poolArr[i];
	}
	//这三个变量配合索引出合适的内存池
	size_t  _arrlength;
	size_t* _poolSizeArr;
	CMemoryPool** _poolArr;

	//虚拟内存池 用来管理malloc出来的大内存
	//CMemoryPool _Memory0;
#if 0
	//64byte内存池 <64的内存统一在这个池里申请
	//CMemoryPool _Memory64;
	//128byte内存池 64<size<128在这个池里申请
	// CMemoryPool _Memory128;
	// CMemoryPool _Memory512;
	// CMemoryPool _Memory1024;
#endif
};

void* operator new(size_t size) {
	return CMemoryManager::Instance().allocMem(size);
}

void operator delete(void* p) noexcept {
	return CMemoryManager::Instance().freeMem(p);
}

void* operator new[](size_t size) {
	return CMemoryManager::Instance().allocMem(size);
}

void operator delete[](void* p) noexcept {
	 CMemoryManager::Instance().freeMem(p);
}
#if 0 //可以被内存池接管Malloc和Free函数
void* Malloc(size_t size) {
	return CMemoryManager::Instance().allocMem(size);
}

void Free(void* p) {
	 CMemoryManager::Instance().freeMem(p);
}
#endif
#endif