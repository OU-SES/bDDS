/*
*
*	This is a class describging task in broker.
*	A single task is considered as a element of queue in DDS.
*	Datareader in Subscriber stores the task in response of receiving data from clients.
*	Darawriter in Publisher consumes the stored task in the queue.
*	Announcement is established by using shared mutex and thread condition.
*	The class has two different condition of statement (reading, writing) and one mutex.
*
*	Written by Hwimin Kim. June. 9. 2023
*/

#pragma once
#ifndef DDS_BROKER_THREAD
#define DDS_BROKER_THREAD
#include <boost/thread.hpp>
#include <boost/fiber/condition_variable.hpp>
namespace BROKER_THREAD {
	typedef struct {
		int reader_number;
		int writer_number;
		boost::condition_variable reader_proceed;
		boost::condition_variable writer_proceed;
		int pending_writers;
		int pending_readers;
		boost::mutex lock_mutex;
		int task_count;
	} task_lock_t;

	static task_lock_t mutex;
	static boost::mutex io_mutex;
	//methods for condition and mutex
	void task_lock_init();
	void task_mutex_rlock(task_lock_t* lock);
	void task_mutex_wlock(task_lock_t* lock);
	void task_mutex_unlock(task_lock_t* lock);
}
#endif // DDS_THREAD