#ifndef RWLOCKHEAD_H
#define RWLOCKHEAD_H

#include <windows.h>

/*************************************************************************
*** file: RWLock.h
*** Reference:
***     http://bbs.csdn.net/topics/390331427?page=1
*************************************************************************/

class CRWLock
{
public:
    CRWLock(): m_readCount(0), m_writeCount(0) {    ::InitializeCriticalSection(&mtx); }
    ~CRWLock() { ::DeleteCriticalSection(&mtx); }

    void ReadLock()
    {
        while(true)
        {
            if(!m_writeCount)
            {
                ::InterlockedIncrement((volatile long*)&m_readCount);
                if(m_writeCount)
                {
                    // 有读写冲突，让位于写
                    ::InterlockedDecrement((volatile long*)&m_readCount);
                    continue;
                }
                //获得读锁
                break;
            }

            ::Sleep(0);
        }
    }
    void ReadUnlock() { ::InterlockedDecrement((volatile long*)&m_readCount); }

    void WriteLock()
    {
        ::InterlockedIncrement((volatile long*)&m_writeCount);
        ::EnterCriticalSection(&mtx);
        // 等待已经在读的线程结束读操作
        while(true)
        {
            if(!m_readCount)
                break;

            ::Sleep(0);
        }
    }
    void WriteUnlock()
    {
        ::LeaveCriticalSection(&mtx);
        ::InterlockedDecrement((volatile long*)&m_writeCount);
    }
private:
    CRWLock(const CRWLock &);
    CRWLock & operator = (const CRWLock &);
    CRITICAL_SECTION mtx;
    int m_readCount;
    int m_writeCount;
};

#endif // RWLOCKHEAD_H
