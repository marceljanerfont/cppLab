#pragma once

#include "timeseries.hpp"

///// shared code: TopFrequencyBitmask
template <unsigned int MAX_FLAGS>
class TopFrequencyBitmask {
public:
  TopFrequencyBitmask() {}
  void clear() {
    count_map_.clear();
  }
  void removeOldValue(const unsigned int& value) {
    // remove old value bit mask from the counter map
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((value & (1 << k)) != 0) {
        count_map_[k]--;
      }
    }
  }
  void addNewValue(const unsigned int& value) {
    // count the bit position
    for (unsigned int k = 0; k < MAX_BITS_; ++k) {
      if ((value & (1 << k)) != 0) {
        count_map_[k]++;
      }
    }
  }
  unsigned int getBestValue() const {
    // compute best_value_
    unsigned int best_value = 0;
    using Pair = std::pair<unsigned int, unsigned int>;
    struct CompareCount {
      bool operator() (const Pair& l, const Pair& r) const { return r.second > l.second; }
    };
    std::priority_queue<Pair, std::vector<Pair>, CompareCount> queue(count_map_.begin(), count_map_.end());

    for (unsigned int i = 0; (i < MAX_FLAGS && !queue.empty()); ++i) {
      best_value |= (1 << queue.top().first);
      queue.pop();
    }

    return best_value;
  }

private:
  const unsigned int MAX_BITS_{ 32 };
  std::unordered_map<unsigned int, unsigned int> count_map_;
};
////////////////////////////////////////

struct Colour {
  Colour(unsigned int colour) : colour_(colour) {}
  unsigned int colour_;
};
//
class TimeSeriesColour : 
  public TimeSeries<Colour> {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_COLOURS = 1;
  TimeSeriesColour() : TimeSeries<Colour>(MAX_SIZE) {}

  void clear() override {
    TimeSeries<Colour>::clear();
    top_freq_bitmask_.clear();
  }

  void updateRemovingOldestValue(const Colour& old_value) override {
    top_freq_bitmask_.removeOldValue(old_value.colour_);
  }

  void updateAddingNewestValue(const Colour& new_value) override {
    top_freq_bitmask_.addNewValue(new_value.colour_);
  }

  Colour getBestValue() {
    return Colour(top_freq_bitmask_.getBestValue());
  }
private:
  TopFrequencyBitmask<MAX_COLOURS> top_freq_bitmask_;
};
////////////////////////////////////////

struct ColourTop {
  ColourTop(unsigned int colour) : colour_(colour) {}
  unsigned int colour_;
};
//
class TimeSeriesColourTop :
  public TimeSeries<ColourTop> {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_COLOURS = 2;
  TimeSeriesColourTop() : TimeSeries<ColourTop>(MAX_SIZE) {}

  void clear() override {
    TimeSeries<ColourTop>::clear();
    top_freq_bitmask_.clear();
  }

  void updateRemovingOldestValue(const ColourTop& old_value) override {
    top_freq_bitmask_.removeOldValue(old_value.colour_);
  }

  void updateAddingNewestValue(const ColourTop& new_value) override {
    top_freq_bitmask_.addNewValue(new_value.colour_);
  }

  ColourTop getBestValue() {
    return ColourTop(top_freq_bitmask_.getBestValue());
  }
private:
  TopFrequencyBitmask<MAX_COLOURS> top_freq_bitmask_;
};
////////////////////////////////////////

typedef unsigned int ColourBottom;
class TimeSeriesColourBottom : public TimeSeries<ColourBottom> {
public:
  TimeSeriesColourBottom() : TimeSeries<ColourBottom>(10) {}

  unsigned int bestValue() {
    return 8;
  }
};
