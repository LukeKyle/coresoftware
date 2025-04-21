#include "DiodeReco.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>


//#include <ffarawobjects/TpcDiodeContainerv1.h>
//#include <ffarawobjects/TpcDiodev1.h>

#include <Event/Event.h>
#include <Event/EventTypes.h>
#include <Event/packet.h>

#include <TH2.h>
#include <TCanvas.h>

#include <algorithm>

using namespace std;

DiodeReco::DiodeReco(const std::string& name)
  : SubsysReco(name)
{
  adc.clear();
}

int DiodeReco::Init(PHCompositeNode*)
{
  //std::cout << "Init" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int DiodeReco::InitRun(PHCompositeNode*)
{
  c_waveforms = new TCanvas("c_waveforms","c_waveforms",800,600);
  waveforms = new TH2F("CAEN Waveforms","CAEN WAveforms;time bins [ns];channels",1024,0,1024,32,0,32);
  //std::cout << "InitRun" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int DiodeReco::process_event(PHCompositeNode *topNode)
{

  Event *_event = findNode::getClass<Event>(topNode, "PRDF");
  if (!_event)
    {
      std::cout << "RawdataUnpacker::Process_Event - Event not found" << std::endl;
      return Fun4AllReturnCodes::DISCARDEVENT;
    }

  _event->identify();
  Packet *p = _event->getPacket(2000);
  if ( p)
    {
      for(int c=0;c<32;c++){
	for(int t=0;t<1024;t++){
	  waveforms->Fill(t,c,p->iValue(t,c));
	}
      }
      //p->dump();
      delete p;
    }

  c_waveforms->cd();
  waveforms->Draw("LEGO");
  
  //std::cout << "process_event" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int DiodeReco::End(PHCompositeNode*)
{
  //std::cout << "End" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}


double DiodeReco::MaxAdc(int n,int low_bin,int high_bin)
{
  double MaxSum = -99999; // Maximum sum over n bins within the bin range 
  int MaxMid = -1; // Bin number of the middle bin used to calculate MaxSum

  // Bracket the limits safely...
  int start = max(1+n,low_bin+n);
  int end   = min(int(adc.size())-n,high_bin-n);
  for (int mid=start; mid<end; mid++)
    {
      double Sum=0;
      for (int bin=mid-n; bin<=mid+n; bin++)
        {
          Sum += adc[bin];
        }
      if (Sum > MaxSum)
        {
          MaxSum = Sum;
          MaxMid = mid;
        }
    }

  if(MaxMid<0)
    {
      cout<<"Error: Maximum ADC could not be found!"<<endl;
    }
  return MaxSum/(2.0*n+1.0);
}

int DiodeReco::MaxBin(int n)
{
  double MaxSum = -99999;
  int MaxMid = -1;
  for (int mid=1+n; mid<static_cast<int>(adc.size())-n; mid++)
    {
      double Sum=0;
      for (int bin=mid-n; bin<=mid+n; bin++)
        {
          Sum += adc[bin];
        }
      if (Sum > MaxSum)
        {
          MaxSum = Sum;
          MaxMid = mid;
        }
    }
  if(MaxMid<0)
    {
      cout<<"Error: Maximum ADC could not be found!"<<endl;
    }
  return MaxMid;
}

double DiodeReco::Integral(int low_bin,int high_bin)
{
if (low_bin<0) low_bin=0;
 if (high_bin>static_cast<int>(adc.size())) high_bin=static_cast<int>(adc.size());

  double SUM = 0;
  for(int i=low_bin; i<high_bin; i++)
    {
      SUM += adc[i];
    }
  return SUM;
}
