#pragma once

#include <typeindex>
#include <map>
#include <unordered_map>

#include "time_series.hpp"


/////////////// POSE stuff
struct Joint {
  int x_;
  int y_;
};

struct Pose {
  static const unsigned int JOINT_TOTAL{ 14 };
  Joint joints_[JOINT_TOTAL];
};


class TimeSeriesPose : public TimeSeries<Pose> {
public:
  const size_t MAX_SIZE{ 10 };
  TimeSeriesPose() : TimeSeries<Pose>(MAX_SIZE) {}
  void updateRemovingOldestValue() override {};
  void updateAddingNewestValue() override {
    // update aggression confidence
    agrression_confidence_ += 0.1f;

    // update aggression confidence
    trip_and_fall_confidence_ += 0.2f;
  };

  float getAgressionConfidence() {
    return agrression_confidence_;
  }
  float getTripAndFallConfidence() {
    return trip_and_fall_confidence_;
  }

  float agrression_confidence_{ 0.f };
  float trip_and_fall_confidence_{ 0.f };
};





class Dashboard {
public:
  Dashboard() {
    // Register all TimeSeries here!!!
    //value_type_ts_map_[typeid(Pose)] = std::make_shared<TimeSeriesPose>();
    //registerTimeSeries<TimeSeriesPose>();
  }
  ~Dashboard() {}

  template <typename T>
  void addSample(const T& value, const uint64_t& timestamp) {
    auto it = value_type_ts_map_.find(typeid(T));
    if (it != value_type_ts_map_.end()) {
      std::shared_ptr<TimeSeries<T>> timeseries = std::static_pointer_cast<TimeSeries<T>>(it->second);
      timeseries->addSample(value, timestamp);
    }
    else {
      std::cout << "TimeSeries with sample of type " << typeid(T).name() << " does not exist in the Dashboard." << std::endl;
    }
  }
  template <typename U>
  std::shared_ptr<U> getTimeSeries() {
    auto it = ts_type_ts_map_.find(typeid(U));
    if (it != ts_type_ts_map_.end()) {
      return std::static_pointer_cast<U>(it->second);
    }
    else {
      std::cout << "TimeSeries of type " << typeid(U).name() << " does not exist in the Dashboard." << std::endl;
    }
    return nullptr;
  }


  template <typename U>
  void registerTimeSeries() {
    try {
      auto timeseries = std::make_shared<TimeSeriesPose>();
      //using T = typename std::remove_reference<decltype(*timeseries)>::type::ValueType;
      //value_type_ts_map_[typeid(T)] = timeseries;
      //ts_type_ts_map_[typeid(U)] = timeseries;
    }
    catch (const std::exception& e) {
      std::cout << "* * * " << e.what() << std::endl;
    }
    
  }
private:
  std::unordered_map<std::type_index, std::shared_ptr<void>> value_type_ts_map_;
  std::unordered_map<std::type_index, std::shared_ptr<void>> ts_type_ts_map_;
};


/*
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, std::vector<float>, AccumulatedConfidences<ATTRIBUTES_SIZE>>,
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, std::vector<data::Detection>, DetectionClassIdAll>,
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, float>,
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, unsigned int, BitMaskOccurrences<4>>,
  private TimeSeriesMap<MAX_TIMESERIES_HISTORY, unsigned int, BitMaskOccurrences<2>>
  */

/*

namespace data {
  struct Rectangle {
    unsigned int x_tl_; // x coordinate of the top left corner
    unsigned int y_tl_; // y coordinate of the top left corner
    unsigned int x_br_; // x coordinate of the bottom right corner
    unsigned int y_br_; // y coordinate of the bottom right corner
  };
  struct Detection {
    float confidence_;
    unsigned int class_id_;
    float threashold_;
  };
}
*/
/**
* @brief DetectionClassIdAll
* It computes Best Value (CLASS_ID) from ALL of occurrences of class_id_ in Time Series of Detections.
* It does not compute Best Confidence (not needed from the time being)


struct DetectionClassIdAll {
  using DataType = std::vector<data::Detection>;;

  void clear() {
    count_map_.clear();
    best_value_ = 0;
  }

  void removeOldValue(const DataType& value) {
    // remove old detection class_id from the counter map
    for (const auto& d : value) {
      count_map_[d.class_id_]--;
    }
  }

  void addNewValue(const DataType& value) {
    // count the class_id occurrence
    for (const auto& d : value) {
      count_map_[d.class_id_]++;
    }

    // compute best_value_
    best_value_ = 0;
    for (const auto& [class_id, count] : count_map_) {
      if (count > 0) {
        best_value_ |= (1 << class_id);
      }
    }
  }

  std::unordered_map<unsigned int, unsigned int> count_map_;
  unsigned int best_value_ = 0;
}; 
*/