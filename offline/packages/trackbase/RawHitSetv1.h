#ifndef TRACKBASE_RAWHITSETV1_H
#define TRACKBASE_RAWHITSETV1_H

/**
 * @file trackbase/RawHitSetv1.h
 * @author D. McGlinchey, H. PEREIRA DA COSTA
 * @date 4 June 2018
 * @brief Container for storing RawHit's
 */
#include "RawHitSet.h"
#include "TrkrDefs.h"

#include <iostream>
#include <map>
#include <utility>  // for pair

// forward declaration
class RawHit;

class RawHitSetv1 : public RawHitSet
{
 public:
  RawHitSetv1() = default;

  ~RawHitSetv1() override
  {
    RawHitSetv1::Reset();
  }

  void identify(std::ostream& os = std::cout) const override;

  void Reset() override;

  void setHitSetKey(const TrkrDefs::hitsetkey key) override
  {
    m_hitSetKey = key;
  }

  TrkrDefs::hitsetkey getHitSetKey() const override
  {
    return m_hitSetKey;
  }

  void addHit(RawHit*) override;
  //  void addTpcHit(unsigned short phibin, RawHit*) override;
  void setTpcPhiBins(unsigned short phibins) override;
 
  //  void removeHit(TrkrDefs::hitkey) override;

  // RawHit* getHit(const TrkrDefs::hitkey) const override;

  ConstRange getHits() const override;
  //  ConstRange getTpcHits(unsigned short phibin) const override;

  unsigned int size() const override
  {
    return m_hits.size();
  }
  unsigned int tpcphibins() const override
  {
    return m_tpchits.size();
  }
  
  ConstVecIterator getHits(int phibin) override
    {
      return m_tpchits.begin()+phibin;
    }
  unsigned int size(int phibin) const override
  {
    return (*(m_tpchits.begin()+phibin)).size();
  }

 private:
  /// unique key for this object
  TrkrDefs::hitsetkey m_hitSetKey = TrkrDefs::HITSETKEYMAX;

  /// storage for RawHit objects
  Vector m_hits;
  
  VectorTpc2D m_tpchits;
  ClassDefOverride(RawHitSetv1, 1);
};

#endif  // TRACKBASE_RawHitSetv1_H
