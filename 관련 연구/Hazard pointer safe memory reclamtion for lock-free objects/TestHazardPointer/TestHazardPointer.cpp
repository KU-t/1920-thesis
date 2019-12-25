// TestHazardPointer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <crtdbg.h>
#include <process.h>
#include <time.h>

#include "hpqueue.h"

#define MAX_CNT 100000

HpLockFreeQueue<int>* g_queue;
int* g_check;
int g_index = 0;

unsigned __stdcall ThreadFunc( void* pArguments )
{
    cout << "thread1 start\n";

    srand( (unsigned)time( NULL ) );

    int d, r;

    int c = 0;
    int end = 0;
    while( true )
    {
        c = rand();

        if( g_index <= MAX_CNT && c % 2 == 0 )
        {
            int idx = InterlockedIncrement( (LONG*)&g_index );
            if( idx > MAX_CNT )
                break;

            if( idx <= 0 ) cout << "error\n";

            g_queue->Enqueue( idx - 1 );
        }
        else
        {
            r = g_queue->Dequeue( &d );

            if( r > 0 )
            {
                g_check[d] = 1;
            }
            else if( g_index > MAX_CNT )
            {
                end++;
            }

            if( end > 1000 )
                break;
        }
    }

    cout << "thread1 end\n";

    _endthreadex( 0 );
    return 0;
}

void Test()
{
    g_queue = new HpLockFreeQueue<int>();
    g_check = new int[MAX_CNT];

    for( int i = 0; i < MAX_CNT; i++ )
    {
        g_check[i] = 0;
    }

    for( int i = 0; i < 4; i++ )
    {
        HANDLE hThread;
        unsigned threadID;
        hThread = (HANDLE)_beginthreadex( NULL, 0, &ThreadFunc, NULL, 0, &threadID );
    }

    char c;

    cout << "Threads Created. Wait For Threads End\n";
    cin >> c;

    for( int i = 0; i < MAX_CNT; i++ )
    {
        if( g_check[i] == 0 )
            cout << i << " check 0\n";
    }

    cout << "Checking Done\n";
    cin >> c;

    delete g_queue;
    delete g_check;
}

int _tmain(int argc, _TCHAR* argv[])
{
    Test();

    _CrtDumpMemoryLeaks();

	return 0;
}
