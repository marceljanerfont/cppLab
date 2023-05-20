#pragma once

#include "timeseries.hpp"
#include "timeseries_attr.hpp"


/////////////// FACE attr stuff
struct FaceAttr : public std::vector<float> {};
///
class TimeSeriesFaceAttr : public TimeSeries<FaceAttr> {
public:
  static const size_t MAX_SIZE = 10;
  static const size_t ATTR_SIZE = 18;

  TimeSeriesFaceAttr() : TimeSeries<FaceAttr>(MAX_SIZE) {}

  void updateRemovingOldestValue(const FaceAttr& old_value) override {
    acc_confidences_.removeOldValue(old_value);
  }

  void updateAddingNewestValue(const FaceAttr& new_value) override {
    acc_confidences_.addNewValue(new_value);
  }

private:
  AccumulatedConfidences<ATTR_SIZE> acc_confidences_;
};
