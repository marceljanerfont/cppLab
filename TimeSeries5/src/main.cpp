#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <cmath>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/circular_buffer.hpp>


#include "gtest/gtest.h"

#include "timeseries/timeseries.hpp"
#include "timeseries/dashboard.hpp"
#include "../include/utils.h"

//////////////////////////////////////////
// Attribute base class
class Attribute {
public:
  static const int MULTI_ID = -1;
  static const int UNIQUE_ID = 0; // there is only one subtype

  Attribute(const int& id = UNIQUE_ID): id_(id) {};

  // check their similiraty, if true then they can have same subtype 'id_ -> other.id_'
  virtual bool checkSimilarity(const Attribute& other) const { return true; };
  // it called BEFORE new attribute is added into the TimeSeries
  virtual void updateStatistics(const std::shared_ptr<void>& timeseries) {};

  // attribute subtype
  int id_{ -1 };
};

//////////////////////////////////////////
class Gender : public Attribute {
public:
  //static const std::size_t MAX_SIZE = 10;
  enum GenderType { Unknown, Male, Female };

  Gender() : Attribute(UNIQUE_ID), gender_(Unknown) {}
  Gender(const GenderType& gender) : Attribute(UNIQUE_ID), gender_(gender) {}

  GenderType gender_{ Unknown };
  double confidence_{ 0. };
  bool done_{ false };
};
//////////////////////////////////////////
class Age : public Attribute {
public:
  enum AgeType { Unknown, Child, Adult, Elderly };

  Age() : Attribute(UNIQUE_ID), age_(Unknown) {}
  Age(const AgeType& age) : Attribute(UNIQUE_ID), age_(age) {}

  AgeType age_{ Unknown };
  double confidence_{ 0. };
};

//////////////////////////////////////////
class Bag : public Attribute {
public:
  Bag(const int& size, const int& colour) : Attribute(MULTI_ID), size_(size), colour_(colour) {}

  bool checkSimilarity(const Attribute& other) const override {
    const Bag* otherBag = dynamic_cast<const Bag*>(&other);
    if (otherBag) {
      return 
        std::abs(size_ - otherBag->size_) <= 1 && 
        std::abs(colour_ - otherBag->colour_) <= 1;
    }
    return false;
  }

  int colour_;
  int size_;
};
//////////////////////////////////////////
class Pose : public Attribute {
public:
  static const unsigned int JOINT_TOTAL{ 14 };
  struct Joint {
    int x_;
    int y_;
  };
  Pose() : Attribute(UNIQUE_ID) {}

  // data aggregation from previous observation
  void updateStatistics(const std::shared_ptr<void>& timeseries) override {
    const std::shared_ptr<TimeSeries<Pose>> ts_poses = std::static_pointer_cast<TimeSeries<Pose>>(timeseries);
    tripandfall_confidence_ = ts_poses->size() / 10.f;
    aggression_confidence_ = (ts_poses->size() + 5.f) / 10.f;;
  }
 
  // current observation
  Joint joints_[JOINT_TOTAL];

  // aggrregated data computed: updateStatistics()
  float tripandfall_confidence_ { 0.f };
  float aggression_confidence_{ 0.f };
};


//////////////////////////////////////////
//////////////////////////////////////////
//////////////////////////////////////////

TEST(Dashboard, TimeSeries) {

  Dashboard dashboard;

  Gender gender_1(Gender::Male);
  Gender gender_2(Gender::Female);
  Age age_1(Age::Child);
  Age age_2(Age::Adult);

  dashboard.addSample(1, gender_1);
  dashboard.addSample(1, age_1);

  dashboard.addSample(2, gender_2);
  dashboard.addSample(2, age_2);

  std::shared_ptr<TimeSeries<Age>> ts_age = dashboard.getTimeSeries<Age>();
  EXPECT_EQ(age_1.age_, ts_age->oldestValue().age_);


  /// BAGS
  Bag bag_1_1(1, 1);
  Bag bag_1_2(1, 2);
  Bag bag_5_5(5, 5);
  Bag bag_6_5(6, 5);

  dashboard.addSample(1, bag_1_1);
  dashboard.addSample(1, bag_6_5);

  dashboard.addSample(2, bag_1_2);
  dashboard.addSample(2, bag_5_5);

  std::vector<std::shared_ptr<TimeSeries<Bag>>> all_timeseries = dashboard.getAllTimeSeries<Bag>();

  EXPECT_EQ(2, all_timeseries.size());
  EXPECT_EQ(2, all_timeseries[0]->size());
  EXPECT_EQ(2, all_timeseries[1]->size());



  // POSES
  dashboard.addSample(1, Pose());
  dashboard.addSample(2, Pose());
  //std::shared_ptr<TimeSeries<Pose>> ts_poses = dashboard.getTimeSeries<Pose>();

  float current_tripandfall_confidence = dashboard.getNewestValue<Pose>().tripandfall_confidence_;
  float current_agression_confidence = dashboard.getNewestValue<Pose>().aggression_confidence_;

  EXPECT_EQ(1 / 10.f, current_tripandfall_confidence);
  EXPECT_EQ((1 + 5.f) / 10.f, current_agression_confidence);


}


int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  //::testing::GTEST_FLAG(filter) = "TaskScheduler.many_calendar_tasks";
  return RUN_ALL_TESTS();

  return 0;
}
