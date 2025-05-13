#ifndef TPC_TPCTIMECLUSTERIZER_H
#define TPC_TPCTIMECLUSTERIZER_H

#include <fun4all/SubsysReco.h>
#include <string>

class PHCompositeNode;
class CDBTTree;
class CDBInterface;
class TrkrClusterContainer;
class TpcRawHitContainer;
class TpcRawHit;

class TpcTimeClusterizer : public SubsysReco
{
public:

  TpcTimeClusterizer(const std::string &name = "TpcTimeClusterizer");
  ~TpcTimeClusterizer() override = default;

  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int End(PHCompositeNode *topNode) override;

private:
  CDBTTree *m_cdbttree{nullptr};
  CDBInterface *m_cdb{nullptr};
  TpcRawHitContainer *m_rawhits = nullptr;
  TrkrClusterContainer *m_clusterlist = nullptr;

  int FEE_map[26]{4, 5, 0, 2, 1, 11, 9, 10, 8, 7, 6, 0, 1, 3, 7, 6, 5, 4, 3, 2, 0, 2, 1, 3, 5, 4};
  int FEE_R[26]{2, 2, 1, 1, 1, 3, 3, 3, 3, 3, 3, 2, 2, 1, 2, 2, 1, 1, 2, 2, 3, 3, 3, 3, 3, 3};

};

#endif
