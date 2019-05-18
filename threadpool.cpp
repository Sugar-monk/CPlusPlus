/*************************************************************************
-> 文件名: threadpool.cpp
-> 作者: bean
-> 邮箱: 17331372728@163.com
-> 创建时间: Fri 17 May 2019 11:04:45 PM PDT
-> 备注:    线程池的实现
    线程池类
    任务类
*************************************************************************/
#include<iostream>
#include<stdlib.h>
#include<stdio.h>
#include<string>
#include<pthread.h>
#include<queue>
#include<vector>
#include<time.h>
#define MAX_THR 5
#define MAX_QUE 10

using namespace std;

typedef bool (*taskCallback)(int data);

class MyTask
{
    private:
        int _data;
        taskCallback _callback;
    public:
        MyTask(){}
        MyTask(int data, taskCallback callback)
            :_data(data)
            ,_callback(callback)
        {}
        ~MyTask(){}
        void setTask(int data, taskCallback callback)
        {
            _data = data;
            _callback = callback;
        }
        bool Run()
        {
            return _callback(_data);
        }

};

class ThreadPool
{
    private:
        bool _quit_flag;//线程中线程的退出标志
        int _thr_max;   //线程池中最大线程数量
        int _thr_cur;   //线程池中当前线程数量
        queue<MyTask> _queue;
        int _capacity;
        pthread_mutex_t _mutex;
        pthread_cond_t _cond_pro;
        pthread_cond_t _cond_con;
    private:
        void QueueLock()
        {
            pthread_mutex_lock(&_mutex);
        }
        void QueueUnlock()
        {
            pthread_mutex_unlock(&_mutex);
        }
        void ProWait()
        {
            pthread_cond_wait(&_cond_pro,&_mutex);
        }
        void ProWake()
        {
            pthread_cond_signal(&_cond_pro);
        }
        void ConWait()
        {
            //如果用户想要线程退出，这需要选择没有任务的时候退出
            if(_quit_flag == true)
            {
                _thr_cur--;
                QueueUnlock();
                printf("thread: %p --->exit\n", pthread_self());
                pthread_exit(NULL);
            }
            pthread_cond_wait(&_cond_con, &_mutex);
        }
        void ConWake()
        {
            pthread_cond_signal(&_cond_con);
        }
        void ConWakeAll()
        {
            pthread_cond_broadcast(&_cond_con);
        }
        bool QueueIsEmpty()
        {
            return _queue.empty();
        }
        bool QueueIsFull()
        {
            return (_capacity == _queue.size());
        }
        void QueuePop(MyTask *task)
        {
            *task = _queue.front();
            _queue.pop();
        }
        void QueuePush(MyTask &task)
        {
            _queue.push(task);
        }
    public:
        //如果不是全缺省，就不会调用默认构造函数
        ThreadPool(int max = MAX_THR, int capacity = MAX_QUE)
            :_thr_max(max)
            ,_capacity(capacity)
            ,_thr_cur(capacity)
        {
            pthread_mutex_init(&_mutex, NULL);
            pthread_cond_init(&_cond_pro,NULL);
            pthread_cond_init(&_cond_con,NULL);
        }
        ~ThreadPool()
        {
            pthread_mutex_destroy(&_mutex);
            pthread_cond_destroy(&_cond_pro);
            pthread_cond_destroy(&_cond_con);
        }

        //内成员函数会一个this指针，就成了两个参数，这样会与pthread_create的第三个参数不匹配
        static void* thr_start(void* arg)  
        {
            ThreadPool *p = (ThreadPool*)arg;
            while(1)
            {
                p->QueueLock();
                while(p->QueueIsEmpty())
                {
                    p->ConWait();
                }
                MyTask task;
                p->QueuePop(&task);
                p->ProWake();
                p->QueueUnlock();
                task.Run();
            }
            return NULL;
        }
        bool ThreadInit()
        {
            int ret;
            pthread_t tid;
            for(int i = 0; i < _thr_max; ++i)
            {
                ret = pthread_create(&tid, NULL, thr_start,(void*)this);
                if(ret != 0)
                {
                    cout << "pthread create error" << endl;
                    return false;
                }
                pthread_detach(tid); //线程分离
            }
        }
        bool AddTask(MyTask &task)
        {
            QueueLock();
            if(_quit_flag == true)
            {
                QueueUnlock();
                return false;
            }
            while(QueueIsFull())
            {
                ProWait();
            }
            QueuePush(task);
            ConWake();
            QueueUnlock();
            return true;
        }
        void ThreadPoolQuit()
        {
            QueueLock();
            _quit_flag = true;
            QueueUnlock();
            while(_thr_cur > 0)
            {
                ConWakeAll();
                usleep(1000);
            }
        }
};

bool taskHandler(int data)
{
    srand(time(NULL));
    int n = rand() % 5;
    printf("thread: %p is sleep %d sec\n",pthread_self(), n);
    sleep(n);
    return true;
}

int main(int argc, char* argv[])
{
    ThreadPool p;
    p.ThreadInit();
    MyTask task[10];
    for(int i = 0; i < 10; ++i)
    {
        task[i].setTask(i,taskHandler);
        p.AddTask(task[i]);
    }
    p.ThreadPoolQuit();
	return 0;
}

