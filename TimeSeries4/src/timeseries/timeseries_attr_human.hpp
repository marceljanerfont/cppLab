#pragma once

#include <map>

#include "timeseries_attr.hpp"

//https://ipsotek.atlassian.net/wiki/spaces/RD/pages/2876243969/AI+Model+Integration
enum Age {
  AUnknow = 0,
  Child = 1,
  Adult = 2,
  Elderly = 3
};
static const std::map<Age, std::size_t> AGE_IDX{ {Child, 0}, {Adult, 1},  {Elderly, 2} };

enum Gender {
  GUnknow = 0,
  Male = 1,
  Female = 2
};
static const std::map<Gender, std::size_t> GENDER_IDX{ {Female, 3}, {Male, 4} };

enum UpperBody {
  UBUnknow = 0,
  ShortCasual = 1,
  LongCasual = 2,
  PPEVest = 3
};
static const std::map<UpperBody, std::size_t> UPPER_BODY_IDX{ {ShortCasual, 5}, {LongCasual, 6},  {PPEVest, 7} };

enum LowerBody {
  LBUnknow = 0,
  ShortSkirt = 1,
  LongSkirt = 2,
  ShortTrousers = 3,
  LongTrousers = 4
};
static const std::map<LowerBody, std::size_t> LOWER_BODY_IDX{ {ShortSkirt, 9}, {LongSkirt, 10},  {ShortTrousers, 11}, {LongTrousers, 12} };

enum HeadProtection {
  HPUnknow = 0,
  PHelmet = 1,
  NoHelmet = 2
};

enum HeadCoverage {
  HCUnknow = 0,
  CCover = 1,
  NoCover = 2
};

enum Head {
  HUnknow = 0,
  Helmet = 1, // HeadProtection
  Cover = 2, // HeadCoverage
  Bold = 3,
  ShortLightHair = 4,
  ShortDarkHair = 5,
  LongLightHair = 6,
  LongDarkHair = 7
};
static const std::map<Head, std::size_t> HEAD_IDX{ {Helmet, 19}, {Cover, 18},  {Bold, 17}, {ShortLightHair, 16}, {ShortDarkHair, 15}, {LongLightHair, 14}, {LongDarkHair, 13} };

enum AIColour {
  Black = 0,
  Blue = 1,
  Brown = 2,
  Green = 3,
  Grey = 4,
  Orange = 5,
  Pink = 6,
  Purple = 7,
  Red = 8,
  White = 9,
  Yellow = 10,
  AICUnknow
};

class TsHumanAttr :
  public TsAccumulatedConfidences {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_ATTR = 42;
  TsHumanAttr() : TsAccumulatedConfidences(MAX_SIZE, MAX_ATTR) {}


  // TimeSeries virtual functions
  void clear() override;
  void updateAddingNewestValue(const std::vector<float>& new_value) override;
  ///

  double getAgeConfidence() const;
  double getGenderConfidence() const;
  double getUpperBodyConfidence() const;
  double getLowerBodyConfidence() const;
  double getHeadConfidence() const;

  std::string getAgeString() const;
  std::string getGenderString() const;
  std::string getUpperBodyStr() const;
  std::string getLowerBodyStr() const;
  std::string getHeadString() const;
  unsigned int getUpperColourMask() const;
  unsigned int getLowerColourMask() const;
  static std::string getAIColourString(AIColour ai_colour);
  static unsigned int getIntColour(AIColour c);

  // last computed attribute according to the accumulated of the last samples
  Age age_{ AUnknow };
  Gender gender_{ GUnknow };
  UpperBody upper_body_{ UBUnknow };
  LowerBody lower_body_{ LBUnknow };
  Head head_{ HUnknow };
  HeadProtection head_protection_{ HPUnknow };
  HeadCoverage head_coverage_{ HCUnknow };
  AIColour most_domminat_upper_colour_{ AICUnknow };
  AIColour most_domminat_lower_colour_{ AICUnknow };

  bool age_fixed_{ false };
  bool gender_fixed_{ false };
  bool head_fixed_{ false }; // it can be fixed only if is not helmet or cover
};