#pragma once

#include "timeseries/timeseries.hpp"

//namespace timeseries {
class TsAge;
class Age {
public:
  
  enum AgeType {
    Unknow = 0,
    Child = 1,
    Adult = 2,
    Elderly = 3
  };
  using TimeSeriesType = TsAge;
  using AttributeType = AgeType;

  Age() {}
  Age(const AgeType &value, const float &confidence) : value_(value), confidence_(confidence) {}
  //Age(const std::vector<float>& human_attr) {
  //  if (human_attr[0] > 0.f && human_attr[0] >= human_attr[1] && human_attr[0] >= human_attr[2]) {
  //    Age(Child, human_attr[0]);
  //  }
  //  else if (human_attr[1] > 0.f && human_attr[1] > human_attr[0] && human_attr[1] > human_attr[2]) {
  //    Age(Adult, human_attr[1]);
  //  }
  //  else if (human_attr[2] > 0.f && human_attr[2] > human_attr[0] && human_attr[2] >= human_attr[1]) {
  //    Age(Elderly, human_attr[2]);
  //  }
  //}

  std::string toString() const {
    switch (value_) {
    case Unknow:
      return "";
    case Child:
      return "child";
    case Adult:
      return "adult";
    case Elderly:
      return "elderly";
    default:
      return "";
    }
    return "";
  }

  bool isUnknown() {
    return value_ == Unknow;
  }

  Age::AgeType value_{ Unknow };
  float   confidence_{ 0.f };
};

// GENDER
class TsGender;
class Gender {
public:
  enum GenderType {
    Unknow = 0,
    Male = 1,
    Female = 2
  };
  using TimeSeriesType = TsGender;
  using AttributeType = GenderType;

  Gender() {}
  Gender(const GenderType& value, const float& confidence) : value_(value), confidence_(confidence) {}
  //Gender(const std::vector<float> &human_attr) {
  //  if (human_attr[3] > 0.f && human_attr[3] >= human_attr[4]) {
  //    Gender(Female, human_attr[3]);
  //  }
  //  else if (human_attr[4] > 0.f && human_attr[4] > human_attr[3]) {
  //    Gender(Male, human_attr[4]);
  //  }
  //}

  std::string toString() const {
    switch (value_) {
    case Unknow:
      return "";
    case Male:
      return "male";
    case Female:
      return "female";
    default:
      return "";
    }
    return "";
  }

  bool isUnknown() {
    return value_ == Unknow;
  }

  Gender::GenderType value_{ Unknow };
  float   confidence_{ 0.f };
};

template <typename T, typename U= T::AttributeType>
class TsDoubleThreshold : public TimeSeries<T> {
public:
  TsDoubleThreshold(const size_t& max_size) : TimeSeries<T>(max_size) {}

  void setThresholds(const float& t1, const float& t2) {
    threshold_1 = t1;
    threshold_2 = t2;
  }

  bool isClosed() {
    return !best_value_.isUnknown();
  }

  virtual void onRemovingOldestValue(const T& old_value) {
    map_acc_[old_value.value_] -= old_value.confidence_;
  }

  virtual void onAddingNewestValue(const T& new_value) {
    // 1. does it fit over the threshold_1?
    if (new_value.confidence_ >= threshold_1) {
      best_value_ = new_value;
    }
    else {
      // 2. we accumulate confidence
      //TODO:: check that start value at 0.f in RELEASE
      float acc = map_acc_[new_value.value_] += new_value.confidence_;

      // 3. check if accomulated confidence is greater than threshold_2
      if (acc >= threshold_2) {
        best_value_ = new_value;
      }
    }
  }

  float threshold_1 { std::numeric_limits<float>::infinity() };
  float threshold_2 { std::numeric_limits<float>::infinity() };

  T best_value_; //TODO: if we want to stored in Token use ptr or share_ptr¿?
  std::unordered_map<U, float> map_acc_;
};



class TsAge : public TsDoubleThreshold<Age> {
public:
  static const size_t MAX_SIZE = 10;

  TsAge() : TsDoubleThreshold<Age>(MAX_SIZE) {}

  //void updateStatistics() override {
  //  // Aggregation algorithm sample: find two consecutives with one with high confidence at least
  //  const unsigned int MAX_CONSECUTIVES = 2;
  //  if (samples_.size() >= MAX_CONSECUTIVES) {
  //    // iterate backwards
  //    Age prev_age;
  //    unsigned int consecutive_counter = 0;
  //    float max_confidence = 0.f;
  //    for (auto rit = samples_.rbegin(); rit != samples_.rend(); ++rit) {
  //      if (rit->value_.age_ != Age::Unknow) {
  //        if (rit->value_.age_ == prev_age.age_) {
  //          ++consecutive_counter;
  //          if (rit->value_.confidence_ > max_confidence) {
  //            max_confidence = rit->value_.confidence_;
  //          }
  //          if (max_confidence > THRESHOLD && consecutive_counter >= MAX_CONSECUTIVES) {
  //            best_value_ = Age(rit->value_.age_, max_confidence);
  //            break;
  //          }
  //        }
  //        else { // differents, start consecutive counter
  //          prev_age.age_ = rit->value_.age_;
  //          consecutive_counter = 1;
  //          max_confidence = rit->value_.confidence_;
  //        }
  //      }
  //    }
  //  }

  //}

  //Age& best_value_;
};



class TsGender : public TsDoubleThreshold<Gender> {
public:
  static const size_t MAX_SIZE = 10;

  TsGender() : TsDoubleThreshold<Gender>(MAX_SIZE) {}


  //void updateStatistics() override {
  //  // Aggregation algorithm sample: find two consecutives with one with high confidence at least
  //  const unsigned int MAX_CONSECUTIVES = 2;
  //  if (samples_.size() >= MAX_CONSECUTIVES) {
  //    // iterate backwards
  //    Gender prev_gender;
  //    unsigned int consecutive_counter = 0;
  //    float max_confidence = 0.f;
  //    for (auto rit = samples_.rbegin(); rit != samples_.rend(); ++rit) {
  //      if (rit->value_.gender_ != Age::Unknow) {
  //        if (rit->value_.gender_ == prev_gender.gender_) {
  //          ++consecutive_counter;
  //          if (rit->value_.confidence_ > max_confidence) {
  //            max_confidence = rit->value_.confidence_;
  //          }
  //          if (max_confidence > THRESHOLD && consecutive_counter >= MAX_CONSECUTIVES) {
  //            best_value_ = Gender(rit->value_.gender_, max_confidence);
  //            break;
  //          }
  //        }
  //        else { // differents, start consecutive counter
  //          prev_gender.gender_ = rit->value_.gender_;
  //          consecutive_counter = 1;
  //          max_confidence = rit->value_.confidence_;
  //        }
  //      }
  //    }
  //  }

  //}

  //Gender best_value_;
};
//}
