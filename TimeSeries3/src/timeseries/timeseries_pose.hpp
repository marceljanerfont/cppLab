#pragma once

#include "timeseries.hpp"


/////////////// POSE stuff
struct Joint {
  int x_;
  int y_;
};

struct Pose {
  static const unsigned int JOINT_TOTAL{ 14 };
  Joint joints_[JOINT_TOTAL];
};


class TsPose : public TimeSeries<Pose> {
public:
  static const size_t MAX_SIZE = 10;
  
  TsPose() : TimeSeries<Pose>(MAX_SIZE) {}
  
  void updateRemovingOldestValue(const Pose& old_value) override {}
  
  void updateAddingNewestValue(const Pose& new_value) override {
    // update aggression confidence
    agrression_confidence_ += 0.1f;

    // update aggression confidence
    trip_and_fall_confidence_ += 0.2f;
  }

  float getAgressionConfidence() {
    return agrression_confidence_;
  }
  float getTripAndFallConfidence() {
    return trip_and_fall_confidence_;
  }

  float agrression_confidence_{ 0.f };
  float trip_and_fall_confidence_{ 0.f };
};
