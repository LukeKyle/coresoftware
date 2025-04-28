#include "DiodeReco.h"

#include <fun4all/Fun4AllReturnCodes.h>
#include <phool/PHIODataNode.h>    // for PHIODataNode
#include <phool/PHNodeIterator.h>  // for PHNodeIterator
#include <phool/PHObject.h>        // for PHObject
#include <phool/getClass.h>


//#include <ffarawobjects/TpcDiodeContainerv1.h>
#include <ffarawobjects/TpcDiodev1.h>

#include <Event/Event.h>
#include <Event/EventTypes.h>
#include <Event/packet.h>
#include "Event/caen_correction.h"

#include <TH2.h>
#include <TCanvas.h>

#include <algorithm>

using namespace std;

DiodeReco::DiodeReco(const std::string& name)
  : SubsysReco(name)
{
  for(int c=0;c<32;c++)
    {
      adc.clear();
    }
}

int DiodeReco::Init(PHCompositeNode*)
{
  //std::cout << "Init" << std::endl;
  return Fun4AllReturnCodes::EVENT_OK;
}

int DiodeReco::InitRun(PHCompositeNode*)
{
  // char name[100];
  // char title[100];
  
  // c_persistency_N = new TCanvas("c_persistency_N","c_persistency_N",800,600);
  // c_persistency_N->Divide(4,4);
  // c_persistency_S = new TCanvas("c_persistency_S","c_persistency_S",800,600);
  // c_persistency_S->Divide(4,4);
  // c_waveforms = new TCanvas("c_waveforms","c_waveforms",800,600);

  // for(int c=0;c<32;c++){
  //   sprintf(name,"CAEN Persistency Waveform (Channel %d)",c);
  //   sprintf(title,"CAEN Persistency Waveform (Channel %d);time bins [ns];ADC",c);
  //   persistency[c] = new TH2F(name,title,1024,-0.5,1023.5,256,-250,4250);
  // }
  // waveforms = new TH2F("CAEN Waveforms","CAEN Waveforms;time bins [ns];channels",1024,0,1024,32,0,32);
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
  caen_correction *cc = 0;
  Packet *p = _event->getPacket(2000);
  if (p)
    {
      if(!cc)
	{
	  switch ( p->iValue(0,"FREQUENCY") )
	    {
	    case 0:   // 5GS
	      cc = new caen_correction("/sphenix/user/llegnosky/calib/calib_24974_5G.dat");
	      break;
	    case 1:   // 2.5GS
	      cc = new caen_correction("/sphenix/user/llegnosky/calib/calib_24974_2G.dat");
	      break;
	    case 2:   // 5GS
	      cc = new caen_correction("/sphenix/user/llegnosky/calib/calib_24974_1G.dat");
	      break;
	    }
	}

      if(cc)
	{
	  cc->init(p);
	}
      
      for(int c=0;c<32;c++)
	{
	  for(int t=0;t<1024;t++)
	    {
	      adc.push_back(cc->caen_corrected(t,c));
	    }
      
	  PedestalCorrected(0,200);
	  uint16_t maxadc = MaxAdc(2,150,300);
	  uint16_t maxbin = MaxBin(2);
	  double integral = Integral(0,1024);
	  uint16_t nabovethreshold = NAboveThreshold(200,100);
	  double pulsewidth = PulseWidth(200,100);
	  const uint16_t samples = adc.size();

	  //auto newdiode = std::make_unique<TpcDiodev1>();
	  TpcDiodev1 *newdiode = new TpcDiodev1();

	  newdiode->set_packetid(2000);
	  newdiode->set_channel(c);
	  newdiode->set_maxadc(maxadc);
	  newdiode->set_maxbin(maxbin);
	  newdiode->set_integral(integral);
	  newdiode->set_nabovethreshold(nabovethreshold);
	  newdiode->set_pulsewidth(pulsewidth);
	  newdiode->set_samples(samples);

	  for(uint16_t s=0;s<samples;s++)
	    {
	      uint16_t adcval = adc[s];
	      newdiode->set_adc(s,adcval);
	    }
	  
	  //cout << newdiode->get_channel() << endl;
	}
      
      // int laser=-1;
      // int nlaser=0;
      
      // for(int c=0;c<32;c++){
      // 	for(int t=0;t<1024;t++){
      // 	  adc.push_back(cc->caen_corrected(t,c));
      // 	}

      // 	PedestalCorrected(0,200);

      // 	if(c<4||(c>15&&c<20))
      // 	  {
      // 	    int maxadc = MaxAdc(2,200,300);
      // 	    if(maxadc>200)
      // 	      {
      // 		laser=c;
      // 		cout << laser << endl;
      // 		cout << maxadc << endl;
      // 		nlaser++;
      // 	      }
      // 	  }
      // 	adc.clear();
      // }

      // cout << "Event: " << event << " " << "Laser: " << laser << endl;

      // if(event==2){
      // 	  for(int c=0;c<32;c++)
      // 	    {
      // 	      for(int t=0;t<1024;t++){
      // 		adc.push_back(cc->caen_corrected(t,c));
      // 	      }
	      
      // 	      PedestalCorrected(0,200);
	      
      // 	      if(nlaser>1)
      // 		{
      // 		  cout << "More than one laser fired! Stopping code!" << endl;
      // 		  return -1;
      // 		}
	      
      // 	      if(laser<0)
      // 		{
      // 		  cout << "No laser fired in this event!" << endl;
      // 		}
      // 	      for(int s=0;s<static_cast<int>(adc.size());s++){
      // 		persistency[c]->Fill(s,adc[s]);
      // 		waveforms->Fill(s,c,adc[s]);
      // 	      }
      // 	      if(c<16){
      // 		c_persistency_N->cd(c+1);
      // 		persistency[c]->Draw("colz");
      // 	      }
      // 	      else
      // 		{
      // 		  c_persistency_S->cd(c-15);
      // 		  persistency[c]->Draw("colz");
      // 		}
      // 	      adc.clear();
      // 	    }
      // }
      //p->dump();
      delete p;
    }
  
  // c_waveforms->cd();
  // waveforms->Draw("LEGO");
  
  //std::cout << "process_event" << std::endl;
  // event++;
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

int DiodeReco::NAboveThreshold( double upper_thr, double lower_thr)
{
  int nAbove = 0;

  bool belowThreshold = true;

  for(int i=0; i<static_cast<int>(adc.size()); i++)
    {
      if (belowThreshold && adc[i] >= upper_thr)
        {
          nAbove++;
          belowThreshold = false;
        }

      else if (!belowThreshold && adc[i] < lower_thr)
        {
          belowThreshold = true;
        }
    }

  return nAbove;
}

double DiodeReco::PulseWidth(double upper_thr, double lower_thr)
{

  //  The results of this routine are ONLY valid 
  //  if NAbove is one.

  bool belowThreshold = true;

  int left = 0;
  int right = 0;

  for(int i=0; i<static_cast<int>(adc.size()); i++)
    {
      if ( belowThreshold && adc[i] >= upper_thr)
        {
          left = i;
          belowThreshold = false;
        }

      else if ( !belowThreshold && adc[i] < lower_thr)
        {
          right = i;
          belowThreshold = true;
        }
    }

  return right-left;
}

void DiodeReco::PedestalCorrected(int low_bin=-1, int high_bin=9999)
{
  if (low_bin<0) low_bin=0;
  if (high_bin>static_cast<int>(adc.size())) high_bin=adc.size();

  // Copy all voltages in the pedestal region & sort
  vector<double> sam;
  for(int i=low_bin; i<high_bin; i++)
    {
      sam.push_back(adc[i]);
    }
  sort(sam.begin(), sam.end());

  // Assign the pedestal as the median of this distribution
  int n=sam.size();
  double PEDESTAL;
  if(n%2 != 0) PEDESTAL = sam[n/2];
  else PEDESTAL = (sam[(n-1)/2] + sam[n/2])/2.0;
  
  for(int i=0; i<static_cast<int>(adc.size()); i++)
    {
      adc[i] = adc[i]-PEDESTAL;
    }
}
