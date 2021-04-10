#pragma once
#include <windows.h>

#define hlock(M)  for(HLock M##_lock = M; M##_lock; M##_lock.SetUnlock())

class HMutex {
public:
	HMutex(void);
	~HMutex(void);
	void Lock();
	void Unlock();
private:
	CRITICAL_SECTION mCriticalSection;
};

class HLock {
public:
	HLock(HMutex& mutex);
	~HLock(void);
	void SetUnlock();
	operator bool() const;
private:
	HMutex& mMutex;
	bool mLocked;
};