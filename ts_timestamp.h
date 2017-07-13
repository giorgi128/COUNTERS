#ifndef SCAL_DATASTRUCTURES_TS_TIMESTAMP_H_
#define SCAL_DATASTRUCTURES_TS_TIMESTAMP_H_

#define __STDC_LIMIT_MACROS
#include <inttypes.h>
#include <assert.h>
#include <atomic>
#include <stdio.h>

#include "allocation.h"


//////////////////////////////////////////////////////////////////////
// An timestamp class based on an atomic counter.
//////////////////////////////////////////////////////////////////////
class AtomicCounterTimestamp {
  private:
    // Memory for the atomic counter.
    std::atomic<uint64_t> *clock_;

  public:
    inline void initialize(uint64_t delay, uint64_t num_threads) {
      clock_ = memalloc::get<std::atomic<uint64_t>>(memalloc::kCachePrefetch * 4);
      clock_->store(1);
    }

    inline void init_sentinel(uint64_t *result) {
      result[0] = 0;
    }

    inline void init_sentinel_atomic(std::atomic<uint64_t> *result) {
      result[0].store(0);
    }

    inline void init_top_atomic(std::atomic<uint64_t> *result) {
      result[0].store(UINT64_MAX);
    }

    inline void init_top(uint64_t *result) {
      result[0] = UINT64_MAX;			
    }

    inline void load_timestamp(uint64_t *result, std::atomic<uint64_t> *source) {
      result[0] = source[0].load();
    }

    inline void set_timestamp(std::atomic<uint64_t> *result) {
      result[0].store(clock_->fetch_add(1));
    }

    inline void set_timestamp_local(uint64_t *result) {
        result[0] = clock_->fetch_add(1);
    }


    inline void read_time(uint64_t *result) {
      result[0] = clock_->load();
    }

    // Compares two timestamps, returns true if timestamp1 is later than
    // timestamp2.
    inline bool is_later(uint64_t *timestamp1, uint64_t *timestamp2) {
      return timestamp2[0] < timestamp1[0];
    }
};



#endif // SCAL_DATASTRUCTURES_TS_TIMESTAMP_H_
