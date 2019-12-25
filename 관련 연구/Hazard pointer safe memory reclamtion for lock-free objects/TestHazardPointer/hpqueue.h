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

        //������ ���ڵ尡 �ִ��� ã�ƺ���.
        for( p = *pHead; p != NULL; p = p->pNext )
        {
            if( p->active == 0 &&
                InterlockedCompareExchange( (LONG*)&(p->active), (LONG)1, (LONG)0 ) == 0 )
                return p;
        }

        //�����ڵ带 �ϳ� �����Ѵ�.
        p = new HpRec();
        
        //���� ������ ���ڵ带 Head�� �ִ´�.
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


//����ÿ� Thread ���� ������ vector�� �����ϱ� ���ؼ�, Thread ���� ������ vector�� ��ü ����Ʈ�� ������ �־�� �Ѵ�.
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

//Hazard Pointer�� ����� Lock-Free Queue
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

        //�ʹ� ���� Hp �˻� �۾��� ���� �ʵ��� Retire�� Node�� ������ ���� �Ǿ��� ��쿡�� �����Ѵ�.
        if( rList->Size() >= 10 )
            Scan();
    }

    void Scan()
    {
        SimpleList* rList = GetRetireList();

        //���� ������� Hazard Pointer���� ������.
        vector<PVOID> hpList;
        for( HpRec* p = this->pHpRecHead; p != NULL; p = p->pNext )
        {
            if( p->pNode != NULL )
                hpList.push_back( p->pNode );
        }

        //�˻��� ���� �ϱ� ���� �ϴ� ������ �����Ѵ�.
        sort( hpList.begin(), hpList.end(), less<PVOID>() );

        //���� Thread�� RetireList�� �ִ� ��Ҹ��� �˻��� �����Ѵ�.
        SimpleList::Node* p = rList->pHead;
        SimpleList::Node* temp;
        while( p != NULL )
        {
            //Hazard Pointer�� ��ϵ��� �ʾҴٸ�
            if( binary_search( hpList.begin(), hpList.end(), p->pData ) == false )
            {
                //�޸𸮿��� �����Ѵ�.
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
