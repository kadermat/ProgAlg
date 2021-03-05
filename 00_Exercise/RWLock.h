#pragma once

#include <mutex>
#include <iostream>


using namespace std;

class ConditionVariable : public condition_variable {
	size_t m_waitingThreads;				// number of waiting threads

public:
	ConditionVariable() : m_waitingThreads(0) {}
	void wait(unique_lock<mutex>& m) {
		m_waitingThreads++;
		condition_variable::wait(m);
		m_waitingThreads--;
	}
	bool hasWaitingThreads() const { return m_waitingThreads > 0; }
};

class RWLock {
	mutex m_mutex;							// re-entrance not allowed
	ConditionVariable m_readingAllowed;		// true: no writer at work
	ConditionVariable m_writingAllowed;		// true: no reader and no writer at work
	bool m_writeLocked = false;				// locked for writing
	size_t m_readLocked = 0;				// number of concurrent readers

public:
	size_t getReaders() const {
		
		return m_readLocked;
	}

	void lockR() {
		//cout << "Starting to lock Read" << "\n";
		unique_lock<mutex> monitor(m_mutex);
		if(m_writeLocked ||m_writingAllowed.hasWaitingThreads()) {
			do { 
				m_readingAllowed.wait(monitor); // use do while for spurious whait 
			} while (m_writeLocked);
		}
		m_readLocked++;
		//cout << "Done to lock Read" << "\n";
	}

	void unlockR() {
		//cout << "Starting to unlock Read" << "\n";

		unique_lock<mutex> monitor(m_mutex);

		if (m_readLocked > 0)
		{
			m_readLocked--;
			if (m_readLocked == 0)
			{
				m_writingAllowed.notify_one();
			}
		}
		//cout << "Done to unlock Read" << "\n";


	}

	void lockW() {
		//cout << "Starting to lock Write" << "\n";
		unique_lock<mutex> monitor(m_mutex);
		while (m_readLocked > 0 || m_writeLocked == true)
		{
			m_writingAllowed.wait(monitor);
		}
		m_writeLocked = true;
		//cout << "Done to lock Write" << "\n";
	}

	void unlockW() {
		//cout << "Starting to unlock Write" << "\n";
		unique_lock<mutex> monitor(m_mutex);
		m_writeLocked = false;
		m_readingAllowed.notify_all();
		m_writingAllowed.notify_one();
		//cout << "Done to unlockWrite" << "\n";

	}
};