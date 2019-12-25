#pragma once

#include <windows.h>
#include <vector>
#include <algorithm>

using namespace std;

class HpRec
{
public:
    HpRec* pNext;

    int active;
    PVOID pNode;

public:
    HpRec()
    {
        this->pNext = NULL;
        this->active = 1;
        this->pNode = NULL;
    }

    ~HpRec()
    {
    }

    static HpRec* Alloc( HpRec** pHead )
    {
        HpRec* p;

        //기존에 빈레코드가 있는지 찾아본다.
        for( p = *pHead; p != NULL; p = p->pNext )
        {
            if( p->active == 0 &&
                InterlockedCompareExchange( (LONG*)&(p->active), (LONG)1, (LONG)0 ) == 0 )
                return p;
        }

        //새레코드를 하나 생성한다.
        p = new HpRec();
        
        //새로 생성한 레코드를 Head에 넣는다.
        HpRec* oldHead;
        do
        {
            oldHead = *pHead;
            p->pNext = oldHead;
        }
        while( InterlockedCompareExchangePointer( (PVOID*)pHead, p, oldHead ) != oldHead );

        return p;
    }

    static void Release( HpRec* p )
    {
        p->active = 0;
        p->pNode = NULL;
    }

    static void DeleteAll( HpRec** pHead )
    {
        HpRec* p = *pHead;
        HpRec* n;
        while( p != NULL )
        {
            n = p->pNext;
            delete p;
            p = n;
        }
    }
};

class SimpleList
{
public:
    class Node
    {
    public:
        Node* pPrev;
        Node* pNext;
        PVOID pData;

    public:
        Node( PVOID pData )
        {
            this->pPrev = NULL;
            this->pNext = NULL;
            this->pData = pData;
        }

        ~Node()
        {
        }
    };

public:
    Node* pHead;
    Node* pTail;
    int size;

public:
    SimpleList()
    {
        this->pHead = NULL;
        this->pTail = NULL;
        this->size = 0;
    }

    ~SimpleList()
    {
        Node* p = this->pHead;
        Node* n;
        while( p != NULL )
        {
            n = p->pNext;
            delete p->pData;
            delete p;
            p = n;
        }
    }

    Node* Append( PVOID pData )
    {
        Node* node = new Node( pData );

        if( this->pTail == NULL )
        {
            this->pHead = this->pTail = node;
        }
        else
        {
            this->pTail->pNext = node;
            node->pPrev = this->pTail;
            this->pTail = node;
        }

        this->size += 1;

        return node;
    }

    void Delete( Node* node )
    {
        if( this->pHead == node && this->pTail == node )
        {
            this->pHead = this->pTail = NULL;
        }
        else if( this->pHead == node )
        {
            node->pNext->pPrev = NULL;
            this->pHead = node->pNext;
        }
        else if( this->pTail == node )
        {
            node->pPrev->pNext = NULL;
            this->pTail = node->pPrev;
        }
        else
        {
            node->pPrev->pNext = node->pNext;
            node->pNext->pPrev = node->pPrev;
        }

        delete node;
        this->size -= 1;

        return;
    }

    int Size()
    {
        return this->size;
    }
};


//종료시에 Thread 마다 생성한 vector를 제거하기 위해서, Thread 마다 생성한 vector의 전체 리스트를 가지고 있어야 한다.
class RetireListRec
{
public:
    RetireListRec* pNext;
    SimpleList* rList;

public:
    RetireListRec()
    {
        this->pNext = NULL;
        this->rList = NULL;
    }

    ~RetireListRec()
    {
        delete this->rList;
    }

    static void Alloc( RetireListRec** pHead, SimpleList* rList )
    {
        RetireListRec* p = new RetireListRec();
        p->rList = rList;
        
        RetireListRec* oldHead;
        do
        {
            oldHead = *pHead;
            p->pNext = oldHead;
        }
        while( InterlockedCompareExchangePointer( (PVOID*)pHead, p, oldHead ) != oldHead );
    }

    static void DeleteAll( RetireListRec** pHead )
    {
        RetireListRec* p = *pHead;
        RetireListRec* n;
        while( p != NULL )
        {
            n = p->pNext;
            delete p;
            p = n;
        }
    }
};

//Hazard Pointer가 적용된 Lock-Free Queue
template<class T>
class HpLockFreeQueue
{
private:
    template<class T>
    class Node
    {
    public:
        Node* pNext;
        T data;

    public:
        Node( T data )
        {
            this->pNext = NULL;
            this->data = data;
        }

        ~Node()
        {
        }
    };

private:
    Node<T>* pHead;
    Node<T>* pTail;

private:
    HpRec* pHpRecHead;
    RetireListRec* pRetireListHead;
    DWORD tlsSlot;

private:
    SimpleList* GetRetireList()
    {
        SimpleList* rList = (SimpleList*)TlsGetValue( this->tlsSlot );
        if( rList == NULL )
        {
            rList = new SimpleList();
            TlsSetValue( this->tlsSlot, rList );
            RetireListRec::Alloc( &(this->pRetireListHead), rList );
        }

        return rList;
    }

    void RetireNode( PVOID pNode )
    {
        SimpleList* rList = GetRetireList();
        rList->Append( pNode );

        //너무 자주 Hp 검색 작업을 하지 않도록 Retire된 Node가 적절한 수가 되었을 경우에만 수행한다.
        if( rList->Size() >= 10 )
            Scan();
    }

    void Scan()
    {
        SimpleList* rList = GetRetireList();

        //현재 사용중인 Hazard Pointer들을 모은다.
        vector<PVOID> hpList;
        for( HpRec* p = this->pHpRecHead; p != NULL; p = p->pNext )
        {
            if( p->pNode != NULL )
                hpList.push_back( p->pNode );
        }

        //검색을 쉽게 하기 위해 일단 정렬을 수행한다.
        sort( hpList.begin(), hpList.end(), less<PVOID>() );

        //현재 Thread의 RetireList에 있는 요소마다 검색을 수행한다.
        SimpleList::Node* p = rList->pHead;
        SimpleList::Node* temp;
        while( p != NULL )
        {
            //Hazard Pointer에 등록되지 않았다면
            if( binary_search( hpList.begin(), hpList.end(), p->pData ) == false )
            {
                //메모리에서 삭제한다.
                delete p->pData;

                temp = p->pNext;
                rList->Delete( p );
                p = temp;
            }
            else
            {
                p = p->pNext;
            }
        }
    }

public:
    HpLockFreeQueue()
    {
        this->pHead = new Node<T>( NULL );
        this->pTail = this->pHead;

        this->pHpRecHead = NULL;
        this->pRetireListHead = NULL;
        this->tlsSlot = TlsAlloc();
    }

    ~HpLockFreeQueue()
    {
        Node<T>* p = this->pHead;
        Node<T>* n;
        while( p != NULL )
        {
            n = p->pNext;
            delete p;
            p = n;
        }

        HpRec::DeleteAll( &(this->pHpRecHead) );
        RetireListRec::DeleteAll( &(this->pRetireListHead) );
        TlsFree( this->tlsSlot );
    }

    void Enqueue( T data )
    {
        Node<T>* node = new Node<T>( data );

        HpRec* hpRec = HpRec::Alloc( &(this->pHpRecHead) );

        while( true )
        {
            Node<T>* oldTail = this->pTail;
            
            hpRec->pNode = oldTail;

            if( oldTail != this->pTail )
                continue;

            Node<T>* oldTailNext = oldTail->pNext;

            if( oldTailNext != NULL )
            {
                InterlockedCompareExchangePointer( (PVOID*)&(this->pTail), oldTailNext, oldTail );
                continue;
            }

            if( InterlockedCompareExchangePointer( (PVOID*)&(oldTail->pNext), node, NULL ) == NULL )
            {
                HpRec::Release( hpRec );
                InterlockedCompareExchangePointer( (PVOID*)&(this->pTail), node, oldTail );
                return;
            }
        }
    }

    int Dequeue( T* out )
    {
        HpRec* hpRec0 = HpRec::Alloc( &(this->pHpRecHead) );
        HpRec* hpRec1 = HpRec::Alloc( &(this->pHpRecHead) );

        while( true )
        {
            Node<T>* oldHead = this->pHead;
            Node<T>* oldTail = this->pTail;
            
            hpRec0->pNode = oldHead;

            if( oldHead != this->pHead )
                continue;

            Node<T>* oldHeadNext = oldHead->pNext;

            hpRec1->pNode = oldHeadNext;

            if( oldHead != this->pHead )
                continue;

            if( oldHeadNext == NULL )
            {
                HpRec::Release( hpRec0 );
                HpRec::Release( hpRec1 );
                return 0;
            }

            if( oldHead == oldTail )
            {
                InterlockedCompareExchangePointer( (PVOID*)&(this->pTail), oldHeadNext, oldHead );
                continue;
            }

            T ret = oldHeadNext->data;

            if( InterlockedCompareExchangePointer( (PVOID*)&(this->pHead), oldHeadNext, oldHead ) == oldHead )
            {
                HpRec::Release( hpRec0 );
                HpRec::Release( hpRec1 );

                RetireNode( oldHead );
                
                *out = ret;
                return 1;
            }
        }
    }
};
