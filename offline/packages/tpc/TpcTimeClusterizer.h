#ifndef TPC_TPCTIMECLUSTERIZER_H
#define TPC_TPCTIMECLUSTERIZER_H

#include <fun4all/SubsysReco.h>
#include <string>

class PHCompositeNode;

class TpcTimeClusterizer : public SubsysReco
{
public:

  TpcTimeClusterizer(const std::string &name = "TpcTimeClusterizer");
  ~TpcTimeClusterizer() override = default;

  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int End(PHCompositeNode *topNode) override;

};

#endif
