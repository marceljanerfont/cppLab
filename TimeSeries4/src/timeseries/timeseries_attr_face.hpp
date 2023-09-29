#pragma once

//#include "timeseries.hpp"
#include "timeseries_attr.hpp"


/////////////// 
class TsFaceAttr :
  public TsAccumulatedConfidences {
public:
  static const unsigned int MAX_SIZE = 10;
  static const unsigned int MAX_ATTR = 18;
  TsFaceAttr() : TsAccumulatedConfidences(MAX_SIZE, MAX_ATTR) {}
};
