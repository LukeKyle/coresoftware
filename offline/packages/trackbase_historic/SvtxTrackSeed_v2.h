#ifndef TRACKBASEHISTORIC_SVTXTRACKSEED_V2_H
#define TRACKBASEHISTORIC_SVTXTRACKSEED_V2_H

#include <phool/PHObject.h>

#include "TrackSeed.h"

#include <climits>
#include <cmath>
#include <iostream>

class SvtxTrackSeed_v2 : public TrackSeed
{
 public:
  SvtxTrackSeed_v2();
  ~SvtxTrackSeed_v2() override;

  SvtxTrackSeed_v2(const SvtxTrackSeed_v2&);
  SvtxTrackSeed_v2& operator=(const SvtxTrackSeed_v2& seed);

  void identify(std::ostream& os = std::cout) const override;
  void Reset() override { *this = SvtxTrackSeed_v2(); }
  int isValid() const override { return 1; }
  void CopyFrom(const TrackSeed&) override;
  void CopyFrom(TrackSeed* seed) override { CopyFrom(*seed); }
  PHObject* CloneMe() const override { return new SvtxTrackSeed_v2(*this); }

  unsigned int get_silicon_seed_index() const override { return m_silicon_seed; }
  unsigned int get_tpc_seed_index() const override { return m_tpc_seed; }
  short int get_crossing_estimate() const override { return m_crossing_estimate; }
  void set_silicon_seed_index(const unsigned int index) override { m_silicon_seed = index; }
  void set_tpc_seed_index(const unsigned int index) override { m_tpc_seed = index; }
  void set_crossing_estimate(const short int cross) override { m_crossing_estimate = cross; }

 private:
  unsigned int m_silicon_seed = std::numeric_limits<unsigned int>::max();
  unsigned int m_tpc_seed = std::numeric_limits<unsigned int>::max();
  short int m_crossing_estimate = SHRT_MAX;

  ClassDefOverride(SvtxTrackSeed_v2, 1);
};

#endif
