#include <iostream>
#include <climits>
#include <cstdlib>
#include <chrono>
#include <thread>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/circular_buffer.hpp>


#include "gtest/gtest.h"

#include "timeseries/attribute_timeseries.hpp"
#include "timeseries/timeseries_attr_human.hpp"
#include "../include/utils.h"

void printMsg(const std::string &msg) {
  std::cout << "msg: " << msg << std::endl;
}


///  POSE TIMESERIE EXAMPLE
TEST(Attributes, attributes) {

  AttributeTimeSeries attr;

  //attr.pushValue(1, Pose());
  //attr.pushValue(2, Pose());
  //attr.pushValue(3, Pose());

  //EXPECT_EQ(0.3f, attr.timeSeriesOf<Pose>()->getAgressionConfidence());
  //EXPECT_EQ(0.6f, attr.timeSeriesOf<Pose>()->getTripAndFallConfidence());

  attr.timeSeriesOf<Age>()->setThresholds(0.9f, 1.5f);

  attr.pushValue(1, Age(Age::Adult, 0.3f));
  attr.pushValue(2, Age(Age::Child, 0.4f));
  attr.pushValue(3, Age(Age::Child, 0.8f));
  attr.pushValue(4, Age(Age::Adult, 0.2f));
  attr.pushValue(4, Age(Age::Child, 0.4f));
  attr.pushValue(5, Age(Age::Adult, 0.9f));

  EXPECT_EQ(Age::Child, attr.timeSeriesOf<Age>()->best_value_.value_);
  //EXPECT_EQ(0.8f, attr.timeSeriesOf<Age>()->best_value_.confidence_);

}
/*
/////
TEST(Dashboard, COLOUR) {

  Dashboard dashboard;

  dashboard.addSample<TsColour>(1, Colour(0x00000001 << 1));
  dashboard.addSample<TsColour>(2, Colour(0x00000001 << 2));
  dashboard.addSample<TsColour>(3, Colour(0x00000001 << 2));

  dashboard.addSample<TsColourTop>(1, Colour(0x00000001 << 2));
  dashboard.addSample<TsColourTop>(2, Colour(0x00000001 << 5));
  dashboard.addSample<TsColourTop>(3, Colour(0x00000001 << 5));
  dashboard.addSample<TsColourTop>(4, Colour(0x00000001 << 3));
  dashboard.addSample<TsColourTop>(5, Colour(0x00000001 << 2));

  EXPECT_EQ((0x00000001 << 1), dashboard.oldestValue<TsColour>().colour_);
  EXPECT_EQ((0x00000001 << 2), dashboard.newestValue<TsColour>().colour_);
  EXPECT_EQ(0x00000001 << 2, dashboard.getBestValue<TsColour>().colour_);
  EXPECT_EQ((0x00000001 << 2) | (0x00000001 << 5), dashboard.getBestValue<TsColourTop>().colour_);

  //there is no TsColourBottom data
  EXPECT_THROW(dashboard.newestValue<TsColourBottom>(), std::logic_error);
}

TEST(Dashboard, ASSOCIATED_OBJECT) {

}



TEST(Dashboard, HUMAN_FACE_ATTRIBUTES) {
  Dashboard dashboard;

  std::vector<float> human_attr_det(42, 0.1f);
  human_attr_det[3] = 0.8f; // female

  dashboard.addSample<TsHumanAttr>(1, human_attr_det);
  dashboard.addSample<TsFaceAttr>(1, std::vector<float>(18, 0.2f));

  dashboard.addSample<TsHumanAttr>(2, std::vector<float>(42, 0.9f));
  dashboard.addSample<TsFaceAttr>(2, std::vector<float>(18, 0.8f));

  float humman_attr_0 = dashboard.getBestValue<TsHumanAttr>()[0];
  float humman_face_0 = dashboard.getBestValue<TsFaceAttr>()[0];

  EXPECT_EQ(1.f, humman_attr_0);
  EXPECT_EQ(1.f, humman_face_0);

  Gender gender = dashboard.getTimeSeries<TsHumanAttr>()->gender_;
  EXPECT_EQ(Gender::Female, gender);

}
*/

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  //::testing::GTEST_FLAG(filter) = "TaskScheduler.many_calendar_tasks";
  return RUN_ALL_TESTS();

  return 0;
}
