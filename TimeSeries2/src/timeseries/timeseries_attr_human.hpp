#pragma once

#include "timeseries.hpp"
#include "timeseries_attr.hpp"


/////////////// HUMAN attr stuff
struct HumanAttr : public std::vector<float> {};
///
class TimeSeriesHumanAttr : public TimeSeries<HumanAttr> {
public:
  static const size_t MAX_SIZE = 10;
  static const size_t ATTR_SIZE = 42;
  
  TimeSeriesHumanAttr() : TimeSeries<HumanAttr>(MAX_SIZE) {}
  
  void updateRemovingOldestValue(const HumanAttr& old_value) override {
    acc_confidences_.removeOldValue(old_value);
  }
  
  void updateAddingNewestValue(const HumanAttr& new_value) override {
    acc_confidences_.addNewValue(new_value);
  }

private:
  AccumulatedConfidences<ATTR_SIZE> acc_confidences_;
};
