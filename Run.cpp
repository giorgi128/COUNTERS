#define __STDC_FORMAT_MACROS 1  // we want PRIu64 and friends

#include "allocation.h"
#include "ts_timestamp.h"
#include <random>
#include <thread>
#include <atomic>
#include <ctime>
#include <future>
#include <unistd.h>
#include <inttypes.h>
#include <iostream>



double
timediff_in_s(const struct timespec &start,
              const struct timespec &end)
{
  struct timespec tmp;
  if (end.tv_nsec < start.tv_nsec) {
    tmp.tv_sec = end.tv_sec - start.tv_sec - 1;
    tmp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
  } else {
    tmp.tv_sec = end.tv_sec - start.tv_sec;
    tmp.tv_nsec = end.tv_nsec - start.tv_nsec;
  }

  return tmp.tv_sec + (double)tmp.tv_nsec / 1000000000.0;
}

std::atomic<bool> startBarrier(false), endBarrier(false);
AtomicCounterTimestamp *timestamping_;


void threadCounter(uint64_t thread_id, uint64_t m, std::atomic<uint64_t>** CounterValue, uint64_t** opCnt)
{
  uint64_t val;
  uint64_t cnt=0;
  std::mt19937 rng;
  rng.seed(thread_id);
  std::uniform_int_distribution<uint64_t> rnd_st(0, m-1);
  uint64_t k, i, j, tempi, tempj;
  
  while (!startBarrier.load(std::memory_order_relaxed)) {}
  
  while (!endBarrier.load(std::memory_order_relaxed))
  {
    cnt++;
    if (cnt % 2 == 0)
    {
      val = CounterValue[thread_id]->load();
    }
    else
    {
      i = rnd_st(rng);
      j = rnd_st(rng);
      tempi = CounterValue[i]->load();
      tempj = CounterValue[j]->load();
      if (tempi > tempj)
      {
          k = i;
          i = j;
          j = k;
      }
      CounterValue[i]->fetch_add(1);
  }
  opCnt[thread_id]* =cnt;
}

int main(int argc, const char **argv)
{
      
   uint64_t nthreads_, sleep_time, ncounters_;
      
   int opt;
      while ((opt = getopt(argc, argv, "n:s:")) != -1) {
          switch (opt) {
              case 'n':
                  errno = 0;
                  nthreads_ = strtol(optarg, NULL, 0);
                  if (errno != 0);
                  break;
              case 's':
                  errno = 0;
                  sleep_time = strtol(optarg, NULL, 0);
                  if (errno != 0);
                  break;
              default:
                  
          }
      }
    
    ncounters_=C*nthreads_;
    
    std::thread** threads = static_cast<std::thread**> memalloc::CallocAligned(nthreads_,
                                                sizeof(std::threads*), memalloc::kCachePrefetch * 4));
   
    std::atomic<uint64_t>** CounterValue = static_cast<std::atomic<uint64_t>**> memalloc::CallocAligned(ncounters_,
                            sizeof(std::atomic<uint64_t>*), memalloc::kCachePrefetch * 4));
    
    uint64_t** opCnt = static_cast<uint64_t**> memalloc::CallocAligned(nthreads_,sizeof(uint64_t*), memalloc::kCachePrefetch * 4));

    for (uint64_t i = 0; i < nthreads_; i++)
    {
        threads[i] = memalloc::get<std::thread>(memalloc::kCachePrefetch * 4);
        opCnt[i] = memalloc::get<uint64_t>(memalloc::kCachePrefetch * 4);
    }
    
    for (uint64_t i = 0; i < ncounters_; i++)
    {
        CounterValue[i] = memalloc::get<std::atomic<uint64_t>>(memalloc::kCachePrefetch * 4);
    }

    for (uint64_t i = 0; i < nthreads_; i++) 
    {
         
         std::promise<uint64_t> prom;
         opCnt[i]* = 0;
	 
         threads[i]* = std::thread(threadCounter, i, ncounters_, CounterValue, opCnt);
    }    
    struct timespec start, end;

    clock_gettime(CLOCK_MONOTONIC, &start);

    startBarrier.store(true, std::memory_order_relaxed);
    
    usleep(1000000 * sleep_time);
    
    endBarrier.store(true, std::memory_order_relaxed);
    clock_gettime(CLOCK_MONOTONIC, &end);

    for (uint64_t i=0; i < nthreads_; i++) threads[i]->join();
    
    uint64_t totalCnt = 0;
    for (uint64_t i=0; i < nthreads_; i++)
    { 
      totalCnt += opCnt[i]*;
    }
    
    const double elapsed = timediff_in_s(start, end);
    
    uint64_t ops_per_s = (uint64_t)((double) totalCnt / elapsed);

    printf("%" PRIu64 " %" PRIu64 "\n", ops_per_s, nthreads_);
    
    return 0;
  }
