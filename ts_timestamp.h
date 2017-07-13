#ifndef SCAL_DATASTRUCTURES_TS_TIMESTAMP_H_
#define SCAL_DATASTRUCTURES_TS_TIMESTAMP_H_

#define __STDC_LIMIT_MACROS
#include <inttypes.h>
#include <assert.h>
#include <atomic>
#include <stdio.h>

#include "allocation.h"


//////////////////////////////////////////////////////////////////////
// An timestamp class based on a hardware instruction.
//////////////////////////////////////////////////////////////////////
class HardwareTimestamp {
  private:
  public:
    inline void initialize(uint64_t delay, uint64_t num_threads) {
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
      result[0].store(get_hwptime());
    }

    inline void read_time(uint64_t *result) {
      result[0] = get_hwptime();
    }

    // Compares two timestamps, returns true if timestamp1 is later than
    // timestamp2.
    inline bool is_later(uint64_t *timestamp1, uint64_t *timestamp2) {
      return timestamp2[0] < timestamp1[0];
    }
};

//////////////////////////////////////////////////////////////////////
// An interval timestamp class based on a hardware instruction.
//////////////////////////////////////////////////////////////////////
class HardwareIntervalTimestamp {
  private:

    // Length of the interval.
    uint64_t delay_;

  public:
  
    inline void initialize(uint64_t delay, uint64_t num_threads) {
      delay_ = delay;
    }

    inline void init_sentinel(uint64_t *result) {
      result[0] = 0;
      result[1] = 0;
    }

    inline void init_sentinel_atomic(std::atomic<uint64_t> *result) {
      result[0].store(0);
      result[1].store(0);
    }

    inline void init_top_atomic(std::atomic<uint64_t> *result) {
      result[0].store(UINT64_MAX);
      result[1].store(UINT64_MAX);
    }

    inline void init_top(uint64_t *result) {
      result[0] = UINT64_MAX;
      result[1] = UINT64_MAX;
    }

    inline void load_timestamp(uint64_t *result, std::atomic<uint64_t> *source) {
      result[0] = source[0].load();
      result[1] = source[1].load();
    }

    // Acquires a new timestamp and stores it in result.
    inline void set_timestamp(std::atomic<uint64_t> *result) {
      // Set the first timestamp.
      result[0].store(get_hwptime());
      // Wait for delay_ time.
      uint64_t wait = get_hwtime() + delay_;
      while (get_hwtime() < wait) {}
      // Set the second timestamp.
      result[1].store(get_hwptime());

    }

    inline void read_time(uint64_t *result) {
      result[0] = get_hwptime();
      result[1] = result[0];
    }

    // Compares two timestamps, returns true if timestamp1 is later than
    // timestamp2.
    inline bool is_later(uint64_t *timestamp1, uint64_t *timestamp2) {
      return timestamp2[1] < timestamp1[0];
    }
};

//////////////////////////////////////////////////////////////////////
// An interval timestamp class based on a simulated interval.
//////////////////////////////////////////////////////////////////////
class SimulatedIntervalTimestamp {
  private:

    // Length of the interval.
    uint64_t delay_;

  public:
  
    inline void initialize(uint64_t delay, uint64_t num_threads) {
      delay_ = delay;
    }

    inline void init_sentinel(uint64_t *result) {
      result[0] = 0;
      result[1] = 0;
    }

    inline void init_sentinel_atomic(std::atomic<uint64_t> *result) {
      result[0].store(0);
      result[1].store(0);
    }

    inline void init_top_atomic(std::atomic<uint64_t> *result) {
      result[0].store(UINT64_MAX);
      result[1].store(UINT64_MAX);
    }

    inline void init_top(uint64_t *result) {
      result[0] = UINT64_MAX;
      result[1] = UINT64_MAX;
    }

    inline void load_timestamp(uint64_t *result, std::atomic<uint64_t> *source) {
      result[0] = source[0].load();
      result[1] = source[1].load();
    }

    // Acquires a new timestamp and stores it in result.
    inline void set_timestamp(std::atomic<uint64_t> *result) {
      uint64_t timestamp = get_hwptime();
      // Set the first timestamp.
      result[0].store(timestamp);
      // Set the second timestamp.
      result[1].store(timestamp + delay_);

    }

    inline void read_time(uint64_t *result) {
      result[0] = get_hwptime();
      result[1] = result[0];
    }

    // Compares two timestamps, returns true if timestamp1 is later than
    // timestamp2.
    inline bool is_later(uint64_t *timestamp1, uint64_t *timestamp2) {
      return timestamp2[1] < timestamp1[0];
    }
};

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
      result[0].store(clock_->_add(1));
    }

    inline void set_timestamp_local(uint64_t *result) {
        result[0] = clock_->_add(1);
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

//////////////////////////////////////////////////////////////////////
// A stutter-interval-cas timestamp class based.
//////////////////////////////////////////////////////////////////////
class CASTimestamp {
 
  private:
    // Memory for the atomic counter.
    std::atomic<uint64_t> *clock_;

    // An array of thread-local clocks.
    uint64_t delay_;

  public:
    inline void initialize(uint64_t delay, uint64_t num_threads) {
      delay_ = delay;
      clock_ = memalloc::get<std::atomic<uint64_t>>(memalloc::kCachePrefetch * 4);
      clock_->store(1);
    }

    inline void init_sentinel(uint64_t *result) {
      result[0] = 0;
      result[1] = 0;
    }

    inline void init_sentinel_atomic(std::atomic<uint64_t> *result) {
      result[0].store(0);
      result[1].store(0);
    }

    inline void init_top_atomic(std::atomic<uint64_t> *result) {
      result[0].store(UINT64_MAX);
      result[1].store(UINT64_MAX);
    }

    inline void init_top(uint64_t *result) {
      result[0] = UINT64_MAX;
      result[1] = UINT64_MAX;
    }

    inline void load_timestamp(uint64_t *result, std::atomic<uint64_t> *source) {
      result[0] = source[0].load();
      result[1] = source[1].load();
    }

    // Acquires a new timestamp and stores it in result.
    inline void set_timestamp(std::atomic<uint64_t> *result) {
      uint64_t timestamp = clock_->load();
      // Set the first timestamp.
      result[0].store(timestamp + 1);
      uint64_t delay = delay_;

      uint64_t wait = get_hwtime() + delay;
      while (get_hwtime() < wait) {}

      uint64_t timestamp2 = clock_->load();
      if (timestamp != timestamp2) {
        result[1].store(timestamp2);
      } else {
        if (clock_->compare_exchange_weak(timestamp, timestamp + 1)) {
          result[1].store(timestamp + 1);
        } else {
          result[1].store(timestamp);
        }
      }
    }

    inline void read_time(uint64_t *result) {
      result[0] = clock_->load();
      result[1] = result[0];
    }

    // Compares two timestamps, returns true if timestamp1 is later than
    // timestamp2.
    inline bool is_later(uint64_t *timestamp1, uint64_t *timestamp2) {
      return timestamp2[1] < timestamp1[0];
    }
};


#endif // SCAL_DATASTRUCTURES_TS_TIMESTAMP_H_
