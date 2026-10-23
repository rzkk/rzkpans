#ifndef     PANS_INCLUDE_PANS_MUTEX_H    // 路径 + _ + 文件名
#define     PANS_INCLUDE_PANS_MUTEX_H

#include<atomic>
#include<mutex>


#if defined(__x86_64__) || defined(__i386__)
#include<immintrin.h>
#elif  defined(__aarch64__) || defined(__arm__)
#include<arm_acle.h>
#endif

#if defined(__linux) || defined(__GLIBC__)
#include<pthread.h>
#endif

inline void cpu_relax() noexcept{
#if defined(__x86_64__) || defined(__i386__)
    _mm_pause();
#elif defined(__aarch64) || #defined(__arm__)
    __yield();
#endif
}

namespace name
{


#if defined(__linux) || defined(__GLIBC__)
// pthread 版本 : 直接封装
//noexcept : 承诺不抛异常 
class Spinlock{
public:
    Spinlock() noexcept{
        pthread_spin_init(&m_mutex , PTHREAD_PROCESS_PRIVATE);
    }
    ~Spinlock() noexcept{
        pthread_spin_destroy(&m_mutex);
    }
    Spinlock(const Spinlock &) = delete;
    Spinlock& operator= (const Spinlock &) = delete;
    
    void lock() noexcept{
        pthread_spin_lock(&m_mutex);
    }
    void unlock() noexcept{
        pthread_spin_unlock(&m_mutex);
    }

private:
    pthread_spinlock_t m_mutex{};
};
#else 
class Spinlock{
public:
    using Lock = std::lock_guard<Spinlock>;

    Spinlock() noexcept = default;
    ~Spinlock() noexcept = default;

    Spinlock(const Spinlock &) = delete;
    Spinlock& operator= (const Spinlock &) = delete;

    void lock() noexcept{
        while(m_mutex.test_and_set(std::memory_order_acquire)){  // 抢锁
            while( m_mutex.test(std::memory_order_relaxed) ){   // 看看锁释放没 ， 要不要去抢一下
                cpu_relax();  // 告诉cpu , 在忙等 ， 
            }
        }
    }
    void unlock() noexcept{
       m_mutex.clear(std::memory_order_release);
    }
    [[nodiscard]] bool try_lock() noexcept{
        return !m_mutex.test_and_set( std::memory_order_acquire );
    }
private:
    std::atomic_flag m_mutex = ATOMIC_FLAG_INIT;
};



#endif

} // namespace name






#endif
