#pragma once
//#ifndef _OBJECTPOOL_HPP_
//#define _OBJECTPOOL_HPP_

#include <cstdlib>
#include <mutex>
#include "debug.h"

template <class Type, size_t poolsize>
class CObjectPool
{
public:
    //构造并初始化对象池
    CObjectPool() noexcept {
        size_t realSize = sizeof(Type)+sizeof(NodeHeader);
        _poolBuf = new char[poolsize*realSize];
        _pHeader = (NodeHeader*)_poolBuf;
		//初始化NodeHeader信息
		NodeHeader* pTem = _pHeader;
		for (size_t i = 0; i < poolsize; ++i) {
            //将所有的Node用链表串起来
		    pTem->pNext = (NodeHeader*)((char*)pTem + realSize);
		    pTem->nRefCount = 0;
		    pTem->bPool = true;
            pTem = pTem->pNext;
		}
        //将最后一块MemoryBlock的pNext指向NULL
        pTem = (NodeHeader*)((char*)pTem - realSize);
        pTem->pNext = nullptr;
    }

    ~CObjectPool() {
        if(_poolBuf) {
            delete[] _poolBuf;
            _poolBuf = nullptr;
        }
    }

    //申请对象内存
	void* allocObjMemory(size_t size) {
        COUT("object new:%u\n", size);
		NodeHeader* pReturn = nullptr;
		if (nullptr == _pHeader) {
            //此时对象池中的对象已经用完了,需要通过new申请新的对象
			pReturn = (NodeHeader*)new char[(sizeof(Type)+sizeof(NodeHeader))];
			pReturn->bPool = false;
			pReturn->pNext = nullptr;
			pReturn->nRefCount = 1;
		} else {
            //返回pHeader指向的内存给用户用
			pReturn = _pHeader;
            //并把分配给用户的内存移出链表
			_pHeader = _pHeader->pNext;
			pReturn->nRefCount = 1;
		}
		return ((char*)pReturn + sizeof(NodeHeader));
	}

    //释放对象内存
	void freeObjMemory(void* pMem) {
		NodeHeader* pBlock = (NodeHeader*)((char*)pMem - sizeof(NodeHeader));
		if (--pBlock->nRefCount != 0) {
			return;
		}

		if (pBlock->bPool) {
			pBlock->pNext = _pHeader;
			_pHeader = pBlock;
		} else {
			delete[] pBlock;
		}
	}

private:
    struct NodeHeader {
        //指向下一块位置
        NodeHeader* pNext;
        //引用次数
        int nRefCount;
        //是否属于内存池中
        bool bPool;
    };

	//对象池缓冲区首地址
	char* _poolBuf;
	//头部内存单元
	NodeHeader* _pHeader;
	//
    std::mutex m_mutex;
};

template <class Type, size_t poolsize>
class CObjectPoolBase
{
public:
    void* operator new(size_t size) {
        return objectpool().allocObjMemory(size);
    }

    void operator delete(void* p) noexcept {
        objectpool().freeObjMemory(p);
    }

    template <typename ...Args>
    static Type* CreateObject(Args... args) {
        Type* obj = new Type(args...);
        return obj;
    }

    static void DestroyObject(Type* obj) {
        delete obj;
    }

private:
    using CTypePool = CObjectPool<Type, poolsize>;
    static CTypePool& objectpool() {
        static CTypePool pool;
        return pool;
    }
    //static CTypePool pool;
};
//#endif