#pragma once


#include "timeseries_attr_human.hpp"

void TsHumanAttr::clear() {
  TsAccumulatedConfidences::clear();
  age_ = AUnknow;
  gender_ = GUnknow;
  upper_body_ = UBUnknow;
  lower_body_ = LBUnknow;
  head_ = HUnknow;
  head_protection_ = HPUnknow;
  head_coverage_ = HCUnknow;
  most_domminat_upper_colour_ = AICUnknow;
  most_domminat_lower_colour_ = AICUnknow;
  age_fixed_ = false;
  gender_fixed_ = false;
  head_fixed_ = false;
}

void TsHumanAttr::updateAddingNewestValue(const std::vector<float>& new_value) {
  TsAccumulatedConfidences::updateAddingNewestValue(new_value);
  const double MIN_ACC = 0.025 * static_cast<double>(size());

  // Age
  if (!age_fixed_) {
    if (acc_confidences_[0] > MIN_ACC && acc_confidences_[0] >= acc_confidences_[1] && acc_confidences_[0] >= acc_confidences_[2]) {
      age_ = Age::Child;
    }
    else if (acc_confidences_[1] > MIN_ACC && acc_confidences_[1] > acc_confidences_[0] && acc_confidences_[1] > acc_confidences_[2]) {
      age_ = Age::Adult;
    }
    else if (acc_confidences_[2] > MIN_ACC && acc_confidences_[2] > acc_confidences_[0] && acc_confidences_[2] >= acc_confidences_[1]) {
      age_ = Age::Elderly;
    }
  }
  // Gender
  if (!gender_fixed_) {
    if (acc_confidences_[3] > MIN_ACC && acc_confidences_[3] > acc_confidences_[4]) {
      gender_ = Gender::Female;
    }
    else if (acc_confidences_[4] > MIN_ACC && acc_confidences_[4] >= acc_confidences_[3]) {
      gender_ = Gender::Male;
    }
  }
  // Upper body
  if (acc_confidences_[5] > MIN_ACC && acc_confidences_[5] > acc_confidences_[6] && acc_confidences_[7] >= acc_confidences_[8]) {
    upper_body_ = UpperBody::ShortCasual;
  }
  else if (acc_confidences_[5] > MIN_ACC && acc_confidences_[5] > acc_confidences_[6] && acc_confidences_[8] > acc_confidences_[7]) {
    upper_body_ = UpperBody::LongCasual;
  }
  else if (acc_confidences_[6] > MIN_ACC && acc_confidences_[6] >= acc_confidences_[5] && acc_confidences_[7] >= acc_confidences_[8]) {
    upper_body_ = UpperBody::PPEVest;
  }
  else if (acc_confidences_[6] > MIN_ACC && acc_confidences_[6] >= acc_confidences_[5] && acc_confidences_[8] > acc_confidences_[7]) {
    upper_body_ = UpperBody::PPEVest;
  }

  // Lower body
  if (acc_confidences_[9] > MIN_ACC && acc_confidences_[9] >= acc_confidences_[10] && acc_confidences_[11] >= acc_confidences_[12]) {
    lower_body_ = LowerBody::ShortSkirt;
  }
  else if (acc_confidences_[9] > MIN_ACC && acc_confidences_[9] >= acc_confidences_[10] && acc_confidences_[12] > acc_confidences_[11]) {
    lower_body_ = LowerBody::LongSkirt;
  }
  else if (acc_confidences_[10] > MIN_ACC && acc_confidences_[10] > acc_confidences_[9] && acc_confidences_[11] >= acc_confidences_[12]) {
    lower_body_ = LowerBody::ShortTrousers;
  }
  else if (acc_confidences_[10] > MIN_ACC && acc_confidences_[10] > acc_confidences_[9] && acc_confidences_[12] > acc_confidences_[11]) {
    lower_body_ = LowerBody::LongTrousers;
  }

  // HEAD
  // Head protection
  if (
    acc_confidences_[19] > MIN_ACC &&
    acc_confidences_[19] >= acc_confidences_[13] &&
    acc_confidences_[19] >= acc_confidences_[14] &&
    acc_confidences_[19] >= acc_confidences_[15] &&
    acc_confidences_[19] >= acc_confidences_[16] &&
    acc_confidences_[19] >= acc_confidences_[17] &&
    acc_confidences_[19] >= acc_confidences_[18]) {
    head_protection_ = PHelmet;
  }
  else {
    head_protection_ = NoHelmet;
    // Head coverage
    if (
      acc_confidences_[18] > MIN_ACC &&
      acc_confidences_[18] >= acc_confidences_[13] &&
      acc_confidences_[18] >= acc_confidences_[14] &&
      acc_confidences_[18] >= acc_confidences_[15] &&
      acc_confidences_[18] >= acc_confidences_[16] &&
      acc_confidences_[18] >= acc_confidences_[17]) {
      head_coverage_ = CCover;
    }
    else {
      head_coverage_ = NoCover;
      // Head
      if (!head_fixed_) {
        if (
          acc_confidences_[17] > MIN_ACC &&
          acc_confidences_[17] >= acc_confidences_[13] &&
          acc_confidences_[17] >= acc_confidences_[14] &&
          acc_confidences_[17] >= acc_confidences_[15] &&
          acc_confidences_[17] >= acc_confidences_[16]) {
          head_ = Head::Bold;
        }
        else if (acc_confidences_[13] > MIN_ACC && acc_confidences_[13] > acc_confidences_[14] && acc_confidences_[15] >= acc_confidences_[16]) {
          head_ = Head::ShortLightHair;
        }
        else if (acc_confidences_[13] > MIN_ACC && acc_confidences_[13] > acc_confidences_[14] && acc_confidences_[16] > acc_confidences_[15]) {
          head_ = Head::ShortDarkHair;
        }
        else if (acc_confidences_[14] > MIN_ACC && acc_confidences_[14] >= acc_confidences_[13] && acc_confidences_[15] >= acc_confidences_[16]) {
          head_ = Head::LongLightHair;
        }
        else if (acc_confidences_[14] > MIN_ACC && acc_confidences_[14] >= acc_confidences_[13] && acc_confidences_[16] > acc_confidences_[15]) {
          head_ = Head::LongDarkHair;
        }
      }
    }
  }
  // Upper dominant colour
  std::size_t start_idx = 20;
  std::size_t end_idx = 30;
  auto max_elem_iter = std::max_element(acc_confidences_.begin() + start_idx, acc_confidences_.begin() + end_idx);
  if (*max_elem_iter > 0.5 * size()) {
    most_domminat_upper_colour_ = static_cast<AIColour>(std::distance(acc_confidences_.begin() + start_idx, max_elem_iter) - start_idx);
  }
  // Lower dominant colour
  start_idx = 31;
  end_idx = 41;
  max_elem_iter = std::max_element(acc_confidences_.begin() + start_idx, acc_confidences_.begin() + end_idx);
  if (*max_elem_iter > 0.5 * size()) {
    most_domminat_lower_colour_ = static_cast<AIColour>(std::distance(acc_confidences_.begin() + start_idx, max_elem_iter) - start_idx);
  }
}

double TsHumanAttr::getAgeConfidence() const {
  if (age_ == AUnknow) {
    return 0.0;
  }
  return acc_confidences_[AGE_IDX.at(age_)] / static_cast<double>(acc_confidences_.size());
}
double TsHumanAttr::getGenderConfidence() const {
  if (gender_ == GUnknow) {
    return 0.0;
  }
  return acc_confidences_[GENDER_IDX.at(gender_)] / static_cast<double>(acc_confidences_.size());
}
double TsHumanAttr::getUpperBodyConfidence() const {
  if (upper_body_ == UBUnknow) {
    return 0.0;
  }
  if (upper_body_ == PPEVest) { // PPEVest uses 2 attributes indices: 7 and 8
    const std::size_t idx = UPPER_BODY_IDX.at(PPEVest);
    return (acc_confidences_[idx] + acc_confidences_[idx + 1]) / static_cast<double>(acc_confidences_.size());
  }
  return acc_confidences_[UPPER_BODY_IDX.at(upper_body_)] / static_cast<double>(acc_confidences_.size());
}
double TsHumanAttr::getLowerBodyConfidence() const {
  if (lower_body_ == LBUnknow) {
    return 0.0;
  }
  return acc_confidences_[LOWER_BODY_IDX.at(lower_body_)] / static_cast<double>(acc_confidences_.size());
}
double TsHumanAttr::getHeadConfidence() const {
  if (head_ == HUnknow) {
    return 0.0;
  }
  return acc_confidences_[HEAD_IDX.at(head_)] / static_cast<double>(acc_confidences_.size());
}
