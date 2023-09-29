#pragma once

#include <typeindex>
#include <unordered_map>

//#include "timeseries_pose.hpp"
//#include "timeseries_attr_human.hpp"

//namespace timeseries {

  class AttributeTimeSeries {
  public:
    AttributeTimeSeries() {}

    ~AttributeTimeSeries() {}

    template <typename T, typename U = T::TimeSeriesType>
    void pushValue(const uint64_t& timestamp, const T& value) {
      timeSeries<U>()->addSample(timestamp, value);
    }

    template <typename T, typename U = T::TimeSeriesType>
    T newestValue() const {
      return timeSeries<U>()->newestValue();
    }

    // returns the TimeSeries pointer given its attribute type
    template <typename T, typename U = T::TimeSeriesType>
    std::shared_ptr<U> timeSeriesOf() {
      return timeSeries<U>();
    }

    // return the TimeSeries given a TimeSerie type
    template <typename U, typename T = U::ValueType>
    std::shared_ptr<U> timeSeries() {
      auto it = map_.find(typeid(T));
      if (it != map_.end()) {
        return std::static_pointer_cast<U>(it->second);
      }
      // it doesn't exist, let's create it
      std::shared_ptr<U> timeseries = std::make_shared<U>();
      map_[typeid(T)] = timeseries;

      return timeseries;
    }

  private:
    std::unordered_map<std::type_index, std::shared_ptr<void>> map_;
  };
//}