#pragma once
#include "buffer.hpp"
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <memory>
#include <atomic>

namespace zlog
{
    enum class AsyncType
    {
        ASYNC_SAFE,  // 固定长度的缓冲区--阻塞
        ASYNC_UNSAFE // 扩容缓冲区
    };

    class AsyncLooper
    {
    public:
        using Functor = std::function<void(Buffer &)>;
        using ptr = std::shared_ptr<AsyncLooper>;
        AsyncLooper(const Functor &func, AsyncType looperType = AsyncType::ASYNC_SAFE)
            : looperType_(looperType), stop_(false),
              thread_(std::thread(&AsyncLooper::threadEntry, this)), callBack_(func)
        {
        }

        void push(const char *data, size_t len)
        {
            std::unique_lock<std::mutex> lock(mutex_);
            if (looperType_ == AsyncType::ASYNC_SAFE)
            {
                condPro_.wait(lock, [&]()
                              { return proBuf_.writeAbleSize() >= len; });
            }
            proBuf_.push(data, len);
            condCon_.notify_one();
        }

        ~AsyncLooper()
        {
            stop();
        }

        void stop()
        {
            stop_ = true;
            condCon_.notify_all();
            thread_.join(); // 等待工作线程退出
        }

    private:
        // 线程入口函数--对消费缓冲区中的数据进行处理，处理完毕后，初始化缓冲区，交换缓冲区
        void threadEntry()
        {
            while (true)
            {
                {
                    // 1.判断生产缓冲区有没有数据
                    std::unique_lock<std::mutex> lock(mutex_);
                    
                    // 当生产缓冲区为空且标志位被设置的情况下菜退出，否则退出时生产缓冲区仍有数据
                    if (proBuf_.empty() && stop_ == true)
                    {
                        break;
                    }

                    condCon_.wait(lock, [this]()
                                  { return !proBuf_.empty() || stop_; });

                    // 2.唤醒后交换缓冲区
                    conBuf_.swap(proBuf_);
                    if (looperType_ == AsyncType::ASYNC_SAFE)
                        condPro_.notify_all();
                }

                // 3.处理数据并初始化
                callBack_(conBuf_);
                conBuf_.reset();
            }
        }

    private:
        AsyncType looperType_;
        std::atomic<bool> stop_; // 是否工作
        Buffer proBuf_;          // 生产缓冲区
        Buffer conBuf_;          // 消费缓冲区
        std::mutex mutex_;
        std::condition_variable condPro_;
        std::condition_variable condCon_;
        std::thread thread_; // 工作线程
        Functor callBack_;   // 回调函数
    };
};