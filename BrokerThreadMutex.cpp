#include "BrokerThreadMutex.h"

typedef boost::unique_lock< boost::mutex > Lock_t;

/*
*
*	This is a method initializing lock of task.
*	The number of writers, readers, and pending writers would be 0
*
*/
void BROKER_THREAD::task_lock_init() {
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
void BROKER_THREAD::task_mutex_wlock(task_lock_t* lock) {
	boost::mutex::scoped_lock(lock->lock_mutex);
	lock->pending_writers++;
	Lock_t wait_cond(lock->lock_mutex);
	while ((lock->writer_number > 0) || (lock->reader_number)) {
		lock->writer_proceed.wait(wait_cond);
	}
	lock->task_count++;
	lock->pending_writers--;
	lock->writer_number++;
}

/*
*
*	This is a method mutex locking of task when a thread suppose to read.
*	The number of reader increase, and this number only decrease through unlock method
*	The method would wait when there is a writer or pending writers by using a condition reader_proceed, which means this thread is waiting for a signal from other thread.
*
*/
void BROKER_THREAD::task_mutex_rlock(task_lock_t* lock) {
	boost::mutex::scoped_lock(lock->lock_mutex);
	lock->pending_readers++;
	Lock_t wait_cond(lock->lock_mutex);
	while ((lock->pending_writers > 0) || (lock->writer_number)) {
		lock->reader_proceed.wait(wait_cond);
	}
	lock->task_count--;
	lock->pending_readers--;
	lock->reader_number++;
}

/*
*
*	This is a method mutex unlocking of task.
*	Check the number of writer and reader, and set them 0 if it was bigger than 0;
*	If there are waiting threads, send them a signal to continue their works.
*
*/
void BROKER_THREAD::task_mutex_unlock(task_lock_t* lock) {
	boost::mutex::scoped_lock(lock->lock_mutex);
	if (lock->writer_number > 0) {
		lock->writer_number = 0;
	}
	else if (lock->reader_number > 0) {
		lock->reader_number = 0;
	}
	if ((lock->reader_number == 0) && (lock->pending_writers > 0)) {
		lock->writer_proceed.notify_all();
	}
	else if ((lock->reader_number > 0) || (lock->pending_readers > 0) || (lock->task_count > 0)) {
		lock->reader_proceed.notify_all();
	}
}
