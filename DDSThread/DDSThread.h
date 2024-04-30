/*
*
*	This is a class describging communication tasks in broker.
*	A single task is considered as a element of queue in DDS.
*	Datareader in Subscriber stores the task in response of receiving data from clients.
*	Darawriter in Publisher consumes the stored task in the queue.
*	Announcement is established by using shared mutex and thread condition.
*	The class has two different condition of statement (reading, writing) and one mutex.
*	
*	Written by Hwimin Kim. June. 9. 2020 
*/
//To do: check description later. The class should be seperated with Task. It should task general type of data as a task.

#pragma once
#ifndef DDS_THREAD
#define DDS_THREAD
#include <boost/thread.hpp>
#include <boost/fiber/condition_variable.hpp>
typedef boost::unique_lock< boost::mutex > Lock_t;

namespace DTHREAD {
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

	/*
	*
	*	This is a method initializing lock of task.
	*	The number of writers, readers, and pending writers would be 0
	*
	*/
	static void task_lock_init() {
		mutex.reader_number = mutex.writer_number = mutex.pending_writers = mutex.pending_readers = mutex.task_count = 0;
	}

	/*
	*
	*	This is a method mutex locking of task when a thread suppose to write.
	*	The number of pending writers increase at the beginning of the method, but eventually decrease again soon when the task is over.
	*	The number of writer increase, and this number only decrease through unlock method
	*	The method would wait when there are readers or writers by using a condition writer_proceed, which means this thread is waiting for a signal from other thread.
	*
	*/
	static void task_mutex_wlock() {
		boost::mutex::scoped_lock(mutex.lock_mutex);
		mutex.pending_writers++;
		Lock_t wait_cond(mutex.lock_mutex);
		while ((mutex.writer_number > 0) || (mutex.reader_number)) {
			mutex.writer_proceed.wait(wait_cond);
		}
		mutex.task_count++;
		mutex.pending_writers--;
		mutex.writer_number++;

	}

	/*
	*
	*	This is a method mutex locking of task when a thread suppose to read.
	*	The number of reader increase, and this number only decrease through unlock method
	*	The method would wait when there is a writer or pending writers by using a condition reader_proceed, which means this thread is waiting for a signal from other thread.
	*
	*/
	static void task_mutex_rlock() {
		boost::mutex::scoped_lock(mutex.lock_mutex);
		mutex.pending_readers++;
		Lock_t wait_cond(mutex.lock_mutex);
		while ((mutex.pending_writers > 0) || (mutex.writer_number)) {
			mutex.reader_proceed.wait(wait_cond);
		}
		mutex.task_count--;
		mutex.pending_readers--;
		mutex.reader_number++;

	}

	/*
	*
	*	This is a method mutex unlocking of task.
	*	Check the number of writer and reader, and set them 0 if it was bigger than 0;
	*	If there are waiting threads, send them a signal to continue their works.
	*
	*/
	static void task_mutex_unlock() {
		boost::mutex::scoped_lock(mutex.lock_mutex);
		if (mutex.writer_number > 0) {
			mutex.writer_number = 0;
		}
		else if (mutex.reader_number > 0) {
			mutex.reader_number = 0;
		}
		if ((mutex.reader_number == 0) && (mutex.pending_writers > 0)) {
			mutex.writer_proceed.notify_all();
		}
		else if ((mutex.reader_number > 0) || (mutex.pending_readers > 0) || (mutex.task_count > 0)) {
			mutex.reader_proceed.notify_all();
		}
	}
}
#endif // DDS_THREAD