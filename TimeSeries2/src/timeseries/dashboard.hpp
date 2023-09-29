#pragma once

#include <typeindex>
#include <map>
#include <unordered_map>

#include "timeseries_pose.hpp"
#include "timeseries_colour.hpp"


class Dashboard {
public:
  Dashboard() {
    // Register all TimeSeries here!!!
    registerTimeSeries<TimeSeriesPose>();
    registerTimeSeries<TimeSeriesColour>();
    registerTimeSeries<TimeSeriesColourTop>();
    registerTimeSeries<TimeSeriesColourBottom>();
  }

  ~Dashboard() {}

  template <typename T>
  void addSample(const uint64_t& timestamp, const T& value) {
    getTimeSerieBase<T>()->addSample(timestamp, value);
  }

  template <typename T>
  T newestValue() const {
    return getTimeSerieBase<T>()->newestValue();
  }

  template <typename T>
  T oldestValue() const {
    return getTimeSerieBase<T>()->oldestValue();
  }
  
  template <typename U>
  std::shared_ptr<U> getTimeSeries() {
    auto it = ts_type_ts_map_.find(typeid(U));
    if (it == ts_type_ts_map_.end()) {
      throw std::logic_error("The Dasboard has not contain any TimeSeries of type: " + std::string(typeid(U).name()));
    }
    return std::static_pointer_cast<U>(it->second);
  }

  template <typename U>
  void registerTimeSeries() {
    auto timeseries = std::make_shared<U>();
    using T = typename std::remove_reference<decltype(*timeseries)>::type::ValueType;
    value_type_ts_map_[typeid(T)] = timeseries;
    ts_type_ts_map_[typeid(U)] = timeseries;
  }

private:
  template <typename T> 
  std::shared_ptr<TimeSeries<T>> getTimeSerieBase() const {
    // check if we have TimeSeries for this Type
    auto it = value_type_ts_map_.find(typeid(T));
    if (it == value_type_ts_map_.end()) {
      throw std::logic_error("The Dasboard has not contain any TimeSeries for sample's type: " + std::string(typeid(T).name()));
    }
    // we have TimeSeries
    return std::static_pointer_cast<TimeSeries<T>>(it->second);
  }



  std::unordered_map<std::type_index, std::shared_ptr<void>> value_type_ts_map_;
  std::unordered_map<std::type_index, std::shared_ptr<void>> ts_type_ts_map_;
};