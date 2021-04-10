#include "HLock.h"
#include <synchapi.h>

HMutex::HMutex(void) {
	InitializeCriticalSection(&mCriticalSection);
}
HMutex::~HMutex(void) {
	DeleteCriticalSection(&mCriticalSection);
}
void HMutex::Lock() {
	EnterCriticalSection(&mCriticalSection);
}

void HMutex::Unlock() {
	LeaveCriticalSection(&mCriticalSection);
}

HLock::HLock(HMutex& mutex) : mMutex(mutex), mLocked(true) {
	mMutex.Lock();
}

HLock::~HLock(void) {
	mMutex.Unlock();
}

void HLock::SetUnlock() {
	mLocked = false;
}

HLock::operator bool() const {
	return mLocked;
}