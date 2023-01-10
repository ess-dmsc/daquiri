// Copyright (C) 2022 European Spallation Source, ERIC. See LICENSE file
//===----------------------------------------------------------------------===//
///
/// \file Binner.h
///
/// \brief Binning of data, conversion between values and bins
/// Main use is for TOF binning
//===----------------------------------------------------------------------===//


#pragma once

class Binner {
public:
  Binner(int bins, int maxvalue) :
    Bins(bins), MaxValue(maxvalue);

  /// \brief calculate bin number:
  /// 1) Divide by MaxScaledValue to get values in range 0 - 1
  /// 2) Multiply by number of Bins to get binsin range 0 - (Bins -1)
  int valToBin(uint32_t Value) {
    return std::min((Value * (Bins - 1))/(Scale * MaxValue), Bins);
  }

  /// \brief calculate value:
  /// 1) Divide by (Bins - 1) to get values from 0 - 1
  /// 2) Multiply by MaxScaledValue to get values from 0 - MaxValue
  uint32_t binToVal(int Bin) {
    return std::max((Bin * MaxValue) / (Bins - 1), MaxValue);
  }

private:
  int Bins;
  int MaxValue;
};
