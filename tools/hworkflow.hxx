/*
---           `yaal' 0.0.0 (c) 1978 by Marcin 'Amok' Konarski            ---

  hworkflow.hxx - this file is integral part of `yaal' project.

  i.  You may not make any changes in Copyright information.
  ii. You must attach Copyright information to any part of every copy
      of this software.

Copyright:

 You can use this software free of charge and you can redistribute its binary
 package freely but:
  1. You are not allowed to use any part of sources of this software.
  2. You are not allowed to redistribute any part of sources of this software.
  3. You are not allowed to reverse engineer this software.
  4. If you want to distribute a binary package of this software you cannot
     demand any fees for it. You cannot even demand
     a return of cost of the media or distribution (CD for example).
  5. You cannot involve this software in any commercial activity (for example
     as a free add-on to paid software or newspaper).
 This program is distributed in the hope that it will be useful, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. Use it at your own risk.
*/

#ifndef YAAL_TOOLS_HWORKFLOW_HXX_INCLUDED
#define YAAL_TOOLS_HWORKFLOW_HXX_INCLUDED 1

#include "hcore/hpointer.hxx"
#include "hcore/hresource.hxx"
#include "hcore/hboundcall.hxx"
#include "hcore/hthread.hxx"
#include "hcore/hlist.hxx"

namespace yaal {

namespace tools {

/*! \brief Interface for HWorkFlow class to be used by HWorker class.
 */
class HWorkFlowInterface {
public:
	typedef HWorkFlowInterface this_type;
	class HTask;
	typedef yaal::hcore::HBoundCall<> call_t;
	typedef yaal::hcore::HBoundCall<bool()> want_restart_t;
	typedef yaal::hcore::HResource<HTask> task_t;
	virtual ~HWorkFlowInterface( void ){}
	task_t pop_task( void );
private:
	virtual task_t do_pop_task( void ) = 0;
};

typedef yaal::hcore::HExceptionT<HWorkFlowInterface> HWorkFlowInterfaceException;

/*! \brief Thread Pool idiom implementation.
 */
class HWorkFlow : public HWorkFlowInterface {
public:
	typedef HWorkFlow this_type;
	typedef HWorkFlowInterface base_type;
	typedef yaal::hcore::HPointer<HWorkFlow> ptr_t;
	class HWorker;
	typedef yaal::hcore::HResource<HWorker> worker_t;
	/*! \brief Current state of HWorkFlow.
	 */
	enum class STATE {
		RUNNING, /*!< This HWorkFlow is working normally. */
		STOPPING, /*!< This HWorkFlow is currently being stopped but new tasks can be scheduled. */
		INTERRUPTING, /*!< This HWorkFlow is currently being stopped but new tasks can be scheduled. */
		STOPPED, /*!< This HWorkFlow is stopped now, no tasks are running but new tasks can be scheduled. */
		ABORTING, /*!< This HWorkFlow is being aborted right now, new tasks are rejected. */
		CLOSING, /*!< This HWorkFlow is currently being stopped but new tasks are rejected. */
		CLOSED /*!< This HWorkFlow is stopped now, new tasks are rejected. */
	};
	/*! \brief Mode for winding up HWorkFlow.
	 */
	enum class WINDUP_MODE {
		ABORT, /*!< Try to interrupt currently running tasks, drop (abort) scheduled tasks, new tasks are rejected. _state -> ABORTING */
		INTERRUPT, /*!< Interrupt currently running tasks, keep scheduled tasks, new tasks can be scheduled. _state -> INTERRUPTING */
		SUSPEND, /*!< Finish currently runing tasks normally, new tasks can be scheduled. _state -> STOPPING */
		CLOSE /*!< Finish currently runing tasks normally, new tasks are rejected. _state -> STOPPING */
	};
private:
	typedef yaal::hcore::HList<worker_t> worker_pool_t;
	typedef yaal::hcore::HList<task_t> task_queue_t;
	int _workerPoolSize;
	int _busyWorkers;
	STATE _state;
	/*! Tasks executors.
	 */
	worker_pool_t _workers;
	/*! Task queue.
	 */
	task_queue_t _tasks;
	yaal::hcore::HSemaphore _semaphore;
	yaal::hcore::HMutex _mutex;
public:
	/*! \brief Construct HWorkFlow object.
	 *
	 * \param workerPoolSize - maximum number of workers in this worker pool.
	 */
	HWorkFlow( int workerPoolSize = 1 );
	/*! \brief Destroy HWorkFlow object.
	 *
	 * All scheduled tasks that are running are bound to finish naturally.
	 * All scheduled tasks that not yet started are bound to start and then finish naturally.
	 */
	virtual ~HWorkFlow( void );
	/*! \brief Schedule execution of task in this worker pool.
	 *
	 * \param task - task to execute by this worker pool.
	 * \param asyncStop - how to notify executing task that it should stop its execution.
	 * \param wantRestart - test telling if task should be restarted if stopped by asyncStop.
	 */
	void schedule_task( call_t task, call_t asyncStop = call_t(), want_restart_t wantRestart = want_restart_t() );
	/*! \brief Immediatelly start execution of given task in this worker pool.
	 *
	 * \param task - task to execute by this worker pool.
	 * \param asyncStop - how to notify executing task that it should stop its execution.
	 * \param wantRestart - test telling if task should be restarted if stopped by asyncStop.
	 */
	void start_task( call_t task, call_t asyncStop = call_t(), want_restart_t wantRestart = want_restart_t() );
	/*! \brief Restart execution of tasks scheduled in this HWorkFlow.
	 *
	 * \throw HWorkFlowException on parallel start.
	 */
	void start( void );
	/*! \brief Windup all scheduled tasks.
	 *
	 * Join all currently running threads and handle
	 * scheduled tasks.
	 *
	 * Winding up can be performed in several modes (see #WINDUP_MODE).
	 *
	 * Warning: If you windup with SUSPEND or CLOSE mode and have
	 * permanently running task then your program will hang indefinitely.
	 *
	 * When this method return all threads are joined.
	 *
	 * \param windupMode - mode of tasks winding up.
	 * \throw HWorkFlowException on parallel stop.
	 */
	void windup( WINDUP_MODE windupMode );
	/*! \brief Just like windoup(...) but not wait for threads to join.
	 */
	void schedule_windup( WINDUP_MODE windupMode );
	/*! \brief Join all threads.
	 */
	void join( void );
	/*! \brief Tell if calling join() would be a non-blocking call.
	 */
	bool can_join( void );
private:
	virtual task_t do_pop_task( void ) override;
};

typedef yaal::hcore::HExceptionT<HWorkFlow, HWorkFlowInterfaceException> HWorkFlowException;

/*! \brief Finest unit of working capacity.
 */
class HWorkFlow::HWorker {
public:
	typedef HWorker this_type;
	typedef std::atomic<bool> async_flag_t;
private:
	HWorkFlowInterface* _workFlow;
	yaal::hcore::HThread _thread;
	HWorkFlow::STATE _state;
	HWorkFlowInterface::task_t _task;
	async_flag_t _hasTask;
	async_flag_t _canJoin;
	mutable yaal::hcore::HMutex _mutex;
private:
	HWorker( HWorkFlowInterface* );
	void spawn( void );
	void spawn( HWorkFlowInterface::task_t );
	void finish( void );
	void async_stop( HWorkFlow::STATE );
	void run( void );
	bool has_task( void ) const;
	bool can_join( void ) const;
	HWorker( HWorker const& );
	HWorker& operator = ( HWorker const& );
	friend class HWorkFlow;
	template<typename tType, typename... arg_t>
	friend yaal::hcore::HResource<tType> yaal::hcore::make_resource( arg_t&&... );
};

typedef yaal::hcore::HExceptionT<HWorkFlow::HWorker> HWorkerException;

}

}

#endif /* #ifndef YAAL_TOOLS_HWORKFLOW_HXX_INCLUDED */

