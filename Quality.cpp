#define __STDC_FORMAT_MACROS 1  // we want PRIu64 and friends

#include <gflags/gflags.h>
#include "ts_timestamp.h"
#include <random>
#include <thread>
#include <atomic>
#include <ctime>
#include <future>
#include <unistd.h>
#include <inttypes.h>
#include "util/allocation.h"
#include "util/malloc-compat.h"
#include <vector>
#include <algorithm>


const int C = 1; //ncounters/nthreads

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

struct pairVT
{
  uint64_t val;
  uint64_t ts[2];
};




void threadCounter(uint64_t thread_id, uint64_t n, uint64_t m, std::promise<std::vector<pairVT>> &&res)
{
  pairVT cur;
  std::atomic<uint64_t> ts[2];
  std::vector<pairVT> curRes;
  curRes.clear();
  uint64_t val,cnt=0; 
  std::mt19937 rng;
  rng.seed(thread_id);
  std::uniform_int_distribution<uint64_t> rnd_stC(0, C-1);
  std::uniform_int_distribution<uint64_t> rnd_st(0, m-1);

  uint64_t k, i, j, tempi, tempj;
  
  while (!startBarrier.load(std::memory_order_relaxed)) {}
  
  while (!endBarrier.load(std::memory_order_relaxed))
  {
    //    printf("%d %d\n", (int) cnt, (int) thread_id);
    cnt++;
    if (cnt % 2 != 0)
    {
      i = rnd_stC(rng)+thread_id*C;
      cur.val = CounterValue[i]->load(std::memory_order_relaxed);
      
      timestamping_->set_timestamp(ts);
      timestamping_->load_timestamp(cur.ts, ts);
      curRes.push_back(cur);
    }
    else
    {
      i = rnd_st(rng);
      j = rnd_st(rng);
      tempi = CounterValue[i]->load(std::memory_order_relaxed);
      tempj = CounterValue[j]->load(std::memory_order_relaxed);
      if (tempi > tempj)
      {
	  k = i;
	  i = j;
	  j = k;
	  k = tempi;
	  tempi = tempj;
	  tempj = k;
      }
      if (CounterValue[i]->compare_exchange_strong(tempi, tempi + 1, std::memory_order_relaxed))
      {
	cur.val = -1;
	
	timestamping_->set_timestamp(ts);
	timestamping_->load_timestamp(cur.ts, ts);
	curRes.push_back(cur);
      }
    }
   
  }
  res.set_value(curRes);
}
 
bool comp (pairVT i, pairVT j)
{
  return timestamping_->is_later(j.ts, i.ts);
}


int main(int argc, const char **argv)
{
    uint64_t nthreads_, ncounters_, sleep_time;
    
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
 
    
    for (uint64_t i = 0; i < nthreads_; i++)
    {
        threads[i] = memalloc::get<std::thread>(memalloc::kCachePrefetch * 4);
    }

    
    for (uint64_t i = 0; i < ncounters_; i++)
    {
        CounterValue[i] = memalloc::get<std::atomic<uint64_t>>(memalloc::kCachePrefetch * 4);
    }
    
    
    timestamping_ = memalloc::get<AtomicCounterTimestamp>(memalloc::kCachePrefetch * 4);
 
    timestamping_->initialize(0, nthreads_);
    
    
    
    //how to make std::future cache alligned
    
    
    
    std::future<std::vector<pairVT>> * futures =  new std::future<std::vector<pairVT>> [nthreads_];

    for (uint64_t i = 0; i < ncounters_; i++)
    {
      CounterValue[i] = static_cast<std::atomic<uint64_t>*>(scal::get<std::atomic<uint64_t>>(scal::kCachePrefetch * 4));
      CounterValue[i]->store(0, std::memory_order_relaxed);
    }
    for (uint64_t i = 0; i < nthreads_; i++) 
    {
         
         std::promise<std::vector<pairVT>> prom;
         futures[i] = prom.get_future();
	 
         threads[i] = std::thread(threadCounter, i, nthreads_, ncounters_, std::move(prom));
    }    
    struct timespec start, end;


    startBarrier.store(true, std::memory_order_relaxed);
   
    usleep(1000000 * sleep_time);

    endBarrier.store(true, std::memory_order_relaxed);
    

    for (uint64_t i=0; i < nthreads_; i++) threads[i].join();
    std::vector<pairVT> finalRes;
    finalRes.clear();
    uint64_t totalCnt = 0;
    for (uint64_t i=0; i < nthreads_; i++)
    { 
      std::vector<pairVT> curRes = futures[i].get();
      for (uint64_t j=0; j < curRes.size(); j++) finalRes.push_back(curRes[j]);
    }
    
    std::sort(finalRes.begin(), finalRes.end(), comp);
    double ans=0.0;

    uint64_t cntInc, cntR = 0;
    for (uint64_t i = 0; i < finalRes.size(); i++)
    {  
      if (finalRes[i].val==-1) cntInc++;
      else
      {
	cntR++;
	if (finalRes[i].val * ncounters_ > cntInc) ans += (1.0*finalRes[i].val - 1.0*cntInc/ncounters_);
	else ans -= (1.0*finalRes[i].val - 1.0*cntInc/ncounters_); 
      }
    }
    
    printf("%f\n", ans / cntR);
    delete[] threads;
    delete[] futures;
    return 0;
  }
