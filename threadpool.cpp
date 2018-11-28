/**
 * threadpool.c
 *
 * This file will contain your implementation of a threadpool.
 */
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <math.h>
#include <list>
#include <queue>
#include <set>
#include "threadpool.h"
using namespace std;
typedef struct {
    void (*function)(void *);
    void *args;
    int priority;
} task;

struct compare{
bool operator()(const task &t1,const task &t2){
  if(t1.priority<t2.priority)
    return true;
  return false;
}
};
// // _threadpool is the internal threadpool structure that is
// // cast to type "threadpool" before it given out to callers
typedef struct _threadpool_st {
   // you should fill in this structure with whatever you need
    int min_threads;
    int max_threads;
    int thread_count;
     int pending_task;
    multiset<task,compare> request_queue;
    vector<pthread_t> threads;
     pthread_mutex_t pool_mutex;
     pthread_cond_t cond;
    int working_threads;
    int idle_threads;
    int destroy;

    int total;
    
} _threadpool;    


static void* thread_function(void* th_pool){
      _threadpool* pool=(_threadpool*)th_pool;
      task thread_task;
      while(1){
        pthread_mutex_lock(&(pool->pool_mutex));

        while(pool->pending_task==0 & pool->destroy!=1){
        	cout<<"***************sleeping**************************"<<endl;
                  pthread_cond_wait(&(pool->cond),&(pool->pool_mutex));
        	cout<<"********************wakeup************************"<<endl;

        }
        if(pool->destroy==1 && pool->pending_task==0){
        	break;
        }
        thread_task.function=(*(pool->request_queue.begin())).function;
        thread_task.args=(*(pool->request_queue.begin())).args;

        thread_task.priority=(*(pool->request_queue.begin())).priority;

        cout<<"Args: "<<thread_task.args<<" priority: "<<thread_task.priority<<endl;


        (pool->request_queue).erase(pool->request_queue.begin());
        pool->pending_task--;
        pool->working_threads++;
        pthread_mutex_unlock(&(pool->pool_mutex));
        (*(thread_task.function))(thread_task.args);
        pool->working_threads--;

        //resize
  		if(pool->working_threads<(pool->thread_count/2) && pool->thread_count!=pool->min_threads)
  		{
        cout<<"=======Shrinking======"<<endl;
  			pool->thread_count--;
  			pool->idle_threads--;
  			pthread_exit(NULL);

        	return NULL;
  			
  		 }
	//end

      }
        pthread_mutex_unlock(&(pool->pool_mutex));

        pthread_exit(NULL);
        return NULL;
    }


threadpool create_threadpool(int num_threads_in_pool) {
  _threadpool *pool;

  // sanity check the argument
  if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
    return NULL;

  pool = (_threadpool *) malloc(sizeof(_threadpool));
  if (pool == NULL) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;
  }
  // add your code here to initialize the newly created threadpool
  		//resize start
  		pool->max_threads=num_threads_in_pool;
  		pool->min_threads=3;
  		pool->idle_threads=num_threads_in_pool>pool->min_threads?pool->min_threads:num_threads_in_pool;
  		//resize end

        pool->working_threads=0;
        pool->thread_count=0;
        pool->pending_task=0;
        pool->destroy=0;
        (pool->threads).resize(pool->idle_threads);
        // (pool->threads).resize(num_threads_in_pool);

        pthread_mutex_init(&(pool->pool_mutex), NULL);
        pthread_cond_init(&(pool->cond), NULL);
        // cout<<"Idle Threads: "<<pool->idle_threads<<endl;
        for (int i = 0; i <pool->idle_threads; ++i)
        {
          /* code */
          pthread_create(&(pool->threads[i]),NULL,thread_function,static_cast<void *>(pool));
          pool->thread_count++;
        }
        // cout<<"Thread-count: "<<pool->thread_count<<endl;
 

  return (threadpool) pool;
}



void dispatch(threadpool from_me, dispatch_fn dispatch_to_here,
	      void *arg,int priority) {
  _threadpool *pool = (_threadpool *) from_me;

  // add your code here to dispatch a thread

  	pthread_mutex_lock(&(pool->pool_mutex));
  	if(pool->destroy==1){
  	pthread_mutex_unlock(&(pool->pool_mutex));
  		return;
  	}

  	task task_thread;
  	task_thread.function=dispatch_to_here;
    task_thread.args=arg;
  	task_thread.priority=priority;
  	
  	// (pool->request_queue).push_back(task_thread);
    (pool->request_queue).insert(task_thread);
  	pool->pending_task++;
  	//resize
    // cout<<"###############val: "<<(pool->thread_count*3)/4<<endl;
    // cout<<"###############working: "<<pool->working_threads<<endl;
    // cout<<"###############Thread count: "<<pool->thread_count<<endl;
  	if(pool->working_threads==(pool->thread_count*3)/4)
  		{
  			int check=pool->idle_threads;
  			int create_new=(check*2>=pool->max_threads)?pool->max_threads:check*2;
  			pool->threads.resize(create_new);
  			cout<<"+++++++++++++resizing "<<pool->working_threads<<" thread count "<<pool->thread_count<<endl;
  			for (int i = pool->idle_threads; i < create_new; ++i)
  		  		{
  		  			/* code */
  		  		  pthread_create(&(pool->threads[i]),NULL,thread_function,static_cast<void *>(pool));
  		          pool->thread_count++;
  		  		}
  		  		pool->idle_threads+=(create_new-pool->idle_threads);
        cout<<"+++++++++++after resize Thread-count: "<<pool->thread_count<<endl;

  		 }
	//end
  	pthread_cond_signal(&(pool->cond));

  	pthread_mutex_unlock(&(pool->pool_mutex));

}

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;
  // add your code here to kill a threadpool
  	pthread_mutex_lock(&(pool->pool_mutex));

  	pool->destroy=1;
  	//wake all threads to complete tasks
  	pthread_cond_broadcast(&(pool->cond));
  	//so that theprocess calling destroy can allow other threads to use pool to finish their
  	//task in request queue
  	pthread_mutex_unlock(&(pool->pool_mutex));

  	for (int i = 0; i < pool->thread_count; ++i)
  	{
	//join the thread, after its execution is done(ensured by pthread_join()), 
  		//to the thread calling destroy
  			pthread_join(pool->threads[i],NULL);
  	}

  	if((pool->threads).empty()){
  		cout<<"deleting.."<<endl;
  		delete (&(pool->threads));
  		delete(&( pool->request_queue));
  		pthread_mutex_lock(&(pool->pool_mutex));
  		pthread_mutex_destroy(&(pool->pool_mutex));
  		pthread_cond_destroy(&(pool->cond));


  	}
	//because we used malloc in create thread
  	free(pool);
cout<<"Done"<<endl;
}

