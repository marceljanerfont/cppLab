#pragma once

#include <typeindex>
#include <map>
#include <unordered_map>

#include "timeseries_pose.hpp"
#include "timeseries_colour.hpp"
#include "timeseries_attr.hpp"
#include "timeseries_attr_human.hpp"
#include "timeseries_attr_face.hpp"

class Dashboard {
public:
  Dashboard() {
    // Register all TimeSeries here!
    registerTimeSeries<TsPose>();
    registerTimeSeries<TsColour>();
    registerTimeSeries<TsColourTop>();
    registerTimeSeries<TsColourBottom>();
    registerTimeSeries<TsHumanAttr>();
    registerTimeSeries<TsFaceAttr>();
  }

  ~Dashboard() {}

  template <typename U, typename T=U::ValueType>
  void addSample(const uint64_t& timestamp, const T& value) {
    getTimeSeries<U>()->addSample(timestamp, value);
  }

  template <typename U, typename T=U::ValueType>
  T newestValue() const {
    return getTimeSeries<U>()->newestValue();
  }

  template <typename U, typename T=U::ValueType>
  T oldestValue() const {
    return getTimeSeries<U>()->oldestValue();
  }

  template <typename U, typename T=U::ValueType>
  T getBestValue() const {
    return getTimeSeries<U>()->getBestValue();
  }
  
  template <typename U>
  std::shared_ptr<U> getTimeSeries() const {
    auto it = map_.find(typeid(U));
    if (it == map_.end()) {
      throw std::logic_error("The Dasboard has not contain any TimeSeries of type: " + std::string(typeid(U).name()));
    }
    return std::static_pointer_cast<U>(it->second);
  }

private:

  template <typename U>
  void registerTimeSeries() {
    auto timeseries = std::make_shared<U>();
    map_[typeid(U)] = timeseries;
  }

  std::unordered_map<std::type_index, std::shared_ptr<void>> map_;
};