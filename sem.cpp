/*************************************************************************
  -> 文件名: sem.cpp
  -> 作者: bean
  -> 邮箱: 17331372728@163.com
  -> 创建时间: Thu 16 May 2019 06:28:48 AM PDT
  -> 备注:    posix标准信号量的使用
  使用信号量实现一个线程安全的队列
 *************************************************************************/
#include<iostream>
#include<pthread.h>
#include<semaphore.h>
#include<vector>
using namespace std;

class RingQueue
{
    private:
        vector<int> _queue;
        int _capacity;
        int _customer_step; //消费者的脚步
        int _productor_step;//生产者的脚步
        pthread_mutex_t _mutex;//互斥锁实现线程安全
        sem_t   _idle_space; //计数还剩多少空闲空间
        sem_t   _data_space; //计数还有多少空间有数据
    private:
        void QueueLock()
        {
            pthread_mutex_lock(&_mutex);
        }
        void QueueUnlock()
        {
            pthread_mutex_unlock(&_mutex);
        }
        void CustomerWait()
        {
           // int sem_destroy(sem_t *sem);
           // 计数判断，若<=0;则阻塞等待
           // int sem_trywait(sem_t *sem);
           // 计数判断，若<=0;则报错返回
           // int sem_timedwait(sem_t *sem, const struct timespec *abs_timeout);
           // 计数判断，若<=0;则限时等待
           sem_wait(&_data_space);  //如果数据空间为0,则要等待存放
        }
        void CustomerWakeUp()
        {
            //int sem_post(sem_t *sem);
            //唤醒sem信号量等待队列上的线程
            sem_post(&_idle_space);
        }
        void ProductorWait()
        {
            sem_wait(&_idle_space);//如果空闲为0就不能继续存放
        }
        void ProductorWakeUp()
        {
            sem_post(&_data_space);
        }
    public:
        RingQueue(int cap = 10): _capacity(cap) 
         {
             _productor_step = 0;
             _customer_step = 0;
            pthread_mutex_init(&_mutex,NULL);
            //int sem_init(sem_t *sem, int pshared, unsigned int value);
            //信号量初始化
            //sem：          信号量
            //pshared:      
            //  0   用于线程间的同步与互斥
            //  !0  用于进程间的同步与互斥
            //value：   信号量计数初值
            sem_init(&_idle_space,0,_capacity);
            sem_init(&_data_space,0,0);
         }
        ~RingQueue()
        {
            pthread_mutex_destroy(&_mutex);
            //int sem_destroy(sem_t *sem);
            //销毁信号量
            sem_destroy(&_idle_space);
            sem_destroy(&_data_space);
        }
        void QueuePush(int data)
        {
            ProductorWait(); //如果放在外面，如果先加锁，一旦没有存储空间，就会阻塞
            QueueLock();
            _queue[_productor_step] = data; //将数据存放在当前脚步位置
            _productor_step++;
            _productor_step %= _capacity;
            QueueUnlock();
            CustomerWakeUp();
        }
        void QueuePop(int *data)
        {
            CustomerWait();
            QueueLock();
            *data = _queue[_customer_step]; //获取消费者当前的位置
            _customer_step++;
            _customer_step %= _capacity;
            QueueUnlock();
            ProductorWakeUp();
        }
};

void* thr_productor(void* argc)
{
    int i = 0;
    RingQueue *q = (RingQueue*)argc;
    while(1)
    {
        q->QueuePush(i);
        cout << "Put data----->" << i << endl;
        i++;
    }
    return NULL;
}
void* thr_customer(void* argc)
{
    RingQueue *q = (RingQueue*)argc;
    while(1)
    {
        int data;
        q->QueuePop(&data);
        cout << "Get data----->" <<data << endl;
    }
    return NULL;
}

int main(int argc, char* argv[])
{
    int i, ret;
    pthread_t ctid[4],ptid[4];
    RingQueue q;

    for(int i = 0; i < 4; ++i)
    {
        ret = pthread_create(&ptid[i],NULL, thr_productor,(void*)&q);
        if(ret != 0)
        {
            cout << "pthread ctreate error" << endl;
            return -1;
        }
    }
    for(int i = 0; i < 4; ++i)
    {
        ret = pthread_create(&ctid[i],NULL, thr_productor,(void*)&q);
        if(ret != 0)
        {
            cout << "pthread ctreate error" << endl;
            return -1;
        }
    }

    for(int i = 0; i < 4; ++i)
    {
        pthread_join(ptid[i], NULL);
    }
    for(int i = 0; i < 4; ++i)
    {
        pthread_join(ctid[i], NULL);
    }
    return 0;
}
