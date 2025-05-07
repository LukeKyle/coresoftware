#include "TpcTimeClusterizer.h"

#include <phool/PHCompositeNode.h>

TpcTimeClusterizer::TpcTimeClusterizer(const std::string &name)
  : SubsysReco(name)
{
}


int TpcTimeClusterizer::InitRun(PHCompositeNode *)
{
  return 0;
}

int TpcTimeClusterizer::process_event(PHCompositeNode *)
{
  return 0;
}

int TpcTimeClusterizer::End(PHCompositeNode *)
{
  return 0;
}
