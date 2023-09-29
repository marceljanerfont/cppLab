#pragma once

#include "timeseries.hpp"

struct Colour {
  Colour(unsigned int colour) : colour_(colour) {}
  unsigned int colour_;
};
////////////////////////////////////////

//TopFrequencyBitmask
class TsColourBase :
  public TimeSeries<Colour> {
public:
  TsColourBase(const std::size_t& max_size, const unsigned int& max_flags):
    TimeSeries<Colour>(max_size), max_flags_(max_flags) {}

  void updateRemovingOldestValue(const Colour& old_value) override {
    // remove old value bit mask from the counter map
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((old_value.colour_ & (1 << k)) != 0) {
        count_map_[k]--;
      }
    }
  }

  void updateAddingNewestValue(const Colour& new_value) override {
    // count the bit position
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((new_value.colour_ & (1 << k)) != 0) {
        count_map_[k]++;
      }
    }
  }

  Colour getBestValue() {
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

    return Colour(best_value);
  }
private:
  const unsigned int MAX_BITS_{ 32 };
  unsigned int max_flags_{ 1 };
  std::unordered_map<unsigned int, unsigned int> count_map_;
};


class TsColour : public TsColourBase {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_COLOURS = 1;
  TsColour() : TsColourBase(MAX_SIZE, MAX_COLOURS) {}
};

class TsColourTop : public TsColourBase {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_COLOURS = 2;
  TsColourTop() : TsColourBase(MAX_SIZE, MAX_COLOURS) {}
};

class TsColourBottom : public TsColourBase {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_COLOURS = 2;
  TsColourBottom() : TsColourBase(MAX_SIZE, MAX_COLOURS) {}
};
