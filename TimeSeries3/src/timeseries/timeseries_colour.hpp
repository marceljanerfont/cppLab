#pragma once

#include "timeseries.hpp"

///// shared code: TsTopFrequencyBitmask
class TsTopFrequencyBitmask:
  public TimeSeries<unsigned int> {
public:
  TsTopFrequencyBitmask(const unsigned int& max_size, const unsigned int& max_flags): 
    TimeSeries<unsigned int>(max_size), max_flags_(max_flags) {}
  
  void clear() override {
    TimeSeries::clear();
    count_map_.clear();
  }

  void updateRemovingOldestValue(const unsigned int& old_value) override {
    // remove old value bit mask from the counter map
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((old_value & (1 << k)) != 0) {
        count_map_[k]--;
      }
    }
  }

  void updateAddingNewestValue(const unsigned int& new_value) override {
    // count the bit position
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((new_value & (1 << k)) != 0) {
        count_map_[k]++;
      }
    }
  }
  unsigned int getBestValue() const override {
    // compute best_value_
    unsigned int best_value = 0;
    using Pair = std::pair<unsigned int, unsigned int>;
    struct CompareCount {
      bool operator() (const Pair& l, const Pair& r) const { return r.second > l.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, CompareCount> queue(count_map_.begin(), count_map_.end());

    for (unsigned int i = 0; (i < max_flags_ && !queue.empty()); ++i) {
      best_value |= (1 << queue.top().first);
      queue.pop();
    }

    return best_value;
  }

private:
  unsigned int max_flags_{ 1 };
  const unsigned int MAX_BITS_{ 32 };
  std::unordered_map<unsigned int, unsigned int> count_map_;
};
////////////////////////////////////////

class TsColour : 
  public TsTopFrequencyBitmask {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_COLOURS = 1;
  TsColour() : TsTopFrequencyBitmask(MAX_SIZE, MAX_COLOURS) {}
};
////////////////////////////////////////

class TsColourTop :
  public TsTopFrequencyBitmask {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_COLOURS = 2;
  TsColourTop() : TsTopFrequencyBitmask(MAX_SIZE, MAX_COLOURS) {}
};

////////////////////////////////////////

class TsColourBottom :
  public TsTopFrequencyBitmask {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_COLOURS = 2;
  TsColourBottom() : TsTopFrequencyBitmask(MAX_SIZE, MAX_COLOURS) {}
};

