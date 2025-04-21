#ifndef DIODERECO__H
#define DIODERECO__H

#include <fun4all/SubsysReco.h>

#include <string>
#include <iostream>
#include <vector>

class PHCompositeNode;
class TH2F;
class TCanvas;
//class TpcDiode;
//class packet;

class DiodeReco : public SubsysReco
{
 public:
  DiodeReco(const std::string &name = "DiodeReco");
  ~DiodeReco() override = default;
  
  int Init(PHCompositeNode *topNode) override;
  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int End(PHCompositeNode *topNode) override;

  std::vector<double> adc;

  double MaxAdc(int avg, int low_bin=0,int high_bin=9999); // Signal is averaged over "avg" number of bins interatively within the bin range [low_bin,high_bin]
  int MaxBin(int avg);
  double Integral(int low_bin,int high_bin);
  // int NAboveThreshold(double upper_thr,double lower_thr);
  // double PulseWidth(double upper_thr,double lower_thr);
  // void PedestalCorrection(int low_bin,int high_bin);

private:
  TCanvas *c_waveforms;
  TH2F *waveforms;
};

#endif
