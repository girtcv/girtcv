//#pragma once
#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#include "debug.h"
#include <vector>
#include <thread>
#include <mutex>
#include <queue>
#include <future>
#include <atomic>


/** 任务队列类
 * @brief 任务类用来保存需要执行的任务函数和任务参数
 * 任务类可以根据使用需求自行设计,
 * 但必须实现callback函数供线程池回调
 */
class CTask {
//typedef void (*TaskCallBack)(void*);
using TaskCallBack = void (*)(void*);
public:
    CTask(TaskCallBack taskFunc, void* param = nullptr)
    :m_taskFunc(taskFunc), m_param(param) {}

    ///@brief 任务类必须要实现callback函数以供线程池调用
    inline void callback() {
        this->m_taskFunc(this->m_param);//执行回调函数
    }

    ~CTask() {
    //    TAG();
    }

private:
    //线程回调函数
    std::packaged_task<void(void*)> m_taskFunc;
    //回调函数的参数
    void* m_param;
};

template <typename T>
class CThreadPool {
public:
    ///@param thnum 线程数量
    CThreadPool(size_t thnum = 1) noexcept :m_status(true) {
        for (size_t i = 0; i < thnum; i++) {
            m_threads.push_back(std::thread(&CThreadPool::run, this));
        }
    }

    ///@brief 添加执行任务到任务队列中
    ///@param task 需要执行任务
    void push_task(T* task) {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_tasks.emplace(task);
    }
    void push_task(std::shared_ptr<T>&& task) {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        m_tasks.emplace(task);
    }

    ///@brief 杀死所有子线程
    inline void kill_all() {
        //m_status = 0;
        m_status.store(false);
    }

    ///@brief 获取线程数量
    inline size_t GetThreadNum() const {
        return m_threads.size();
    }

    ~CThreadPool() {
#ifdef _DBG //阻塞 等待子线程将任务都执行完才退出
        while (!m_tasks.empty()){}
#endif
	    kill_all();
        for(auto& worker : m_threads) {
            if(worker.joinable()) {
                worker.join();
            }
        }
        //SHOW(("threads exit succeed!\n"));
    }

    CThreadPool(const CThreadPool&) = delete;
    CThreadPool& operator=(const CThreadPool&) = delete;

private:
    ///@brief 查询任务队列是否有任务，有就拿出来执行
    ///@return true:有任务 false:没有任务
    bool pop(std::shared_ptr<T>& task) {
        std::lock_guard<std::mutex> lock(m_queue_mutex);
        if (m_tasks.empty()) {
            return false;
        }
        //如果任务队列不为空，就取出任务来执行
        task = m_tasks.front();
        m_tasks.pop();
        return true;
    }

    ///@brief 线程的执行函数
    void run() {
        std::shared_ptr<T> task;
        while(m_status) {
            bool is_pop = pop(task);
            if(is_pop) { //如果任务队列有任务，则执行任务
                task->callback();//执行回调函数
            } else {
                //建议OS调度其它线程(等价与线程休眠)
                std::this_thread::yield();
            }
        }
    }

private:
    //线程的状态
    //ssize_t m_status;
    std::atomic_bool m_status;
    //任务队列的互斥锁
    std::mutex m_queue_mutex;
    //线程队列
    std::vector<std::thread> m_threads;
    //任务队列
    std::queue<std::shared_ptr<T> > m_tasks;
};
#endif