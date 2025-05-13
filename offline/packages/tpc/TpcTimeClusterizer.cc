#include "TpcTimeClusterizer.h"

#include <phool/PHCompositeNode.h>
#include <phool/PHIODataNode.h>  // for PHIODataNode
#include <phool/PHNodeIterator.h>
#include <phool/PHObject.h>  // for PHObject
#include <phool/getClass.h>

#include <cdbobjects/CDBTTree.h>
#include <ffamodules/CDBInterface.h>

#include <trackbase/TpcDefs.h>
#include <trackbase/TrkrDefs.h>  // for hitkey, getLayer

#include <trackbase/TrkrClusterContainerv4.h>
#include <ffarawobjects/TpcRawHitContainer.h>
#include <ffarawobjects/TpcRawHit.h>
#include <trackbase/TrkrClusterv5.h>

#include <vector>
#include <algorithm>
#include <math.h>

TpcTimeClusterizer::TpcTimeClusterizer(const std::string &name)
  : SubsysReco(name)
{
}


int TpcTimeClusterizer::InitRun(PHCompositeNode *topNode)
{
  std::cout << "TpcCombinedRawDataUnpacker::Init(PHCompositeNode *topNode) Initializing" << std::endl;
  
  m_cdb = CDBInterface::instance();
  std::string calibdir = m_cdb->getUrl("TPC_FEE_CHANNEL_MAP");
  
  if (calibdir[0] == '/')
    {
    // use generic CDBTree to load
      m_cdbttree = new CDBTTree(calibdir);
      m_cdbttree->LoadCalibrations();
    }
  else
    {
      std::cout << "TpcRawDataDecoder::::InitRun No calibration file found" << std::endl;
      exit(1);
    }
  
  PHNodeIterator iter(topNode);

  // Looking for the DST node
  PHCompositeNode *dstNode = dynamic_cast<PHCompositeNode *>(iter.findFirst("PHCompositeNode", "DST"));
  if (!dstNode)
  {
    std::cout << PHWHERE << "DST Node missing, doing nothing." << std::endl;
    //return Fun4AllReturnCodes::ABORTRUN;
    return -1;
  }

  // Create the Cluster node if required
  auto trkrclusters = findNode::getClass<TrkrClusterContainer>(dstNode, "TRKR_CLUSTER");
  if (!trkrclusters)
  {
    PHNodeIterator dstiter(dstNode);
    PHCompositeNode *DetNode =
        dynamic_cast<PHCompositeNode *>(dstiter.findFirst("PHCompositeNode", "TRKR"));
    if (!DetNode)
    {
      DetNode = new PHCompositeNode("TRKR");
      dstNode->addNode(DetNode);
    }

    trkrclusters = new TrkrClusterContainerv4;
    PHIODataNode<PHObject> *TrkrClusterContainerNode =
        new PHIODataNode<PHObject>(trkrclusters, "TRKR_CLUSTER", "PHObject");
    DetNode->addNode(TrkrClusterContainerNode);
  }
  
  return 0;
}

int TpcTimeClusterizer::process_event(PHCompositeNode *topNode)
{
  // get node containing the raw digitized hits
  m_rawhits = findNode::getClass<TpcRawHitContainer>(topNode, "TPCRAWHIT");
  if (!m_rawhits)
    {
      std::cout << PHWHERE << "ERROR: Can't find node TPCRAWHIT" << std::endl;
      //return Fun4AllReturnCodes::ABORTRUN;
      return -1;
    }

  int index = 0;

  TrkrClusterv5 *cluster = new TrkrClusterv5();

  int nhits = m_rawhits->get_nhits();
  
  for(int l=0;l<48;l++){
    for(int t=0;t<1024;t++){
      std::vector<int> AdcSamples;
      std::vector<double> RSamples;
      std::vector<double> PhiSamples;
      std::vector<int> SectorSamples;
      std::vector<int> SideSamples;
      std::vector<int> LayerSamples;
      for(int h=0;h<nhits;h++){
	TpcRawHit *m_rawhit = m_rawhits->get_hit(h);

	int adc = m_rawhit->get_adc(t);
	AdcSamples.push_back(adc);
	
	int fee = m_rawhit->get_fee();
	int channel = m_rawhit->get_channel();

	int feeM = FEE_map[fee];
	if (FEE_R[fee] == 2)
	  {
	    feeM += 6;
	  }
	if (FEE_R[fee] == 3)
	  {
	    feeM += 14;
	  }
	
	int side = 1;
	int32_t packet_id = m_rawhit->get_packetid();
	int ep = (packet_id - 4000) % 10;
	int sector = (packet_id - 4000 - ep) / 10;
	if (sector > 11)
	  {
	    side = 0;
	  }
	
	unsigned int key = 256 * (feeM) + channel;

	std::string varname = "layer";
	int layer = m_cdbttree->GetIntValue(key, varname);
	// antenna pads will be in 0 layer
	if (layer <= 6)
	  {
	    continue;
	  }
	
	varname = "phi";  // + std::to_string(key);
	double phi = (side == 1 ? 1 : -1) * ( m_cdbttree->GetDoubleValue(key, varname) - M_PI/2.) + (sector % 12) * M_PI / 6;

	varname = "R";  // + std::to_string(key);
	double R = m_cdbttree->GetDoubleValue(key, varname);

	RSamples.push_back(R);
	PhiSamples.push_back(phi);
	SectorSamples.push_back(sector);
	SideSamples.push_back(side);
	LayerSamples.push_back(layer);
      }
      auto maxadc_it = max_element(AdcSamples.begin(),AdcSamples.end());
      int maxadc = *maxadc_it;

      int maxadc_index = distance(AdcSamples.begin(),maxadc_it);
      double maxR = RSamples[maxadc_index];
      double maxphi = PhiSamples[maxadc_index];

      double X = maxR*cos(maxphi);
      double Y = maxR*sin(maxphi);

      cluster->setMaxAdc(maxadc);
      cluster->setX(X);
      cluster->setY(Y);

      const auto hitsetkey = TpcDefs::genHitSetKey(LayerSamples[maxadc_index], SectorSamples[maxadc_index], SideSamples[maxadc_index]);

      const auto ckey = TrkrDefs::genClusKey(hitsetkey, index);

      m_clusterlist->addClusterSpecifyKey(ckey, cluster);

      AdcSamples.clear();
      PhiSamples.clear();
      SectorSamples.clear();
      SideSamples.clear();
      LayerSamples.clear();
      index++;
    }
  }
  return 0;
}

int TpcTimeClusterizer::End(PHCompositeNode *)
{
  return 0;
}
