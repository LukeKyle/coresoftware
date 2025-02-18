// Preprocessor directives "include guard" that make sure this header file is
// only included once during compiling... Useful if header file is included in
// other files referenced by this code
#ifndef TRACKRESIDUALS_KSHORT_H
#define TRACKRESIDUALS_KSHORT_H

// inclusion of inherited class, SubsysReco
#include <fun4all/SubsysReco.h>

// Includes for referenced classes
#include <tpc/TpcClusterMover.h>
#include <tpc/TpcGlobalPositionWrapper.h>

// include Root classes
#include <TFile.h>
#include <TTree.h>

// forward delcaration of classes defined in other header files
class PHCompositeNode;

// derived class inherited from SubsysReco
class TrackResiduals_kshort : public SubsysReco
{
 public: // public members of TrackResiduals_kshort class

  // class constructor with optional string parameter with default value
  TrackResiduals_kshort(const std::string &name = "TrackResiduals_kshort");
  
  // class destructor that ensures allocated resources are let go when object
  // goes out of scope or is deleted. Overwritten if it exists in base class
  ~TrackResiduals_kshort() override;

  // overwritten functions from inherited class, SubsysReco
  int Init(PHCompositeNode *topNode) override;
  int InitRun(PHCompositeNode *topNode) override;
  int process_event(PHCompositeNode *topNode) override;
  int End(PHCompositeNode *topNode) override;

  // function to set name of output file
  void outfileName(const std::string &name) { m_outfileName = name; }

  // Public member functions for setting parameters in reconstruction
  void trackmapName(const std::string &name) { m_trackMapName = name; }
  void setTrkrClusterContainerName(std::string &name){ m_clusterContainerName = name; }

  //Private members of TrackResiduals_kshort class
 private:
  // Private member functions
  void clearClusterStateVectors();
  void createBranches();
  void fillClusterBranchesKF(TrkrDefs::cluskey ckey, SvtxTrack *track,const std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>> &global_moved,PHCompositeNode *topNode);
  void fillResidualTreeKF(PHCompositeNode *topNode);
  

  // Private member instances of a class
  TpcClusterMover m_clusterMover;
  TpcGlobalPositionWrapper m_globalPositionWrapper;

  // Private member variables for output data
  std::string m_outfileName = "";
  TFile *m_outfile = nullptr;
  TTree *m_tree = nullptr;

  // Private member variables for TTree variables
  int m_charge = std::numeric_limits<int>::quiet_NaN();
  float m_chisq = std::numeric_limits<float>::quiet_NaN();
  std::vector<float> m_clusAdc;
  std::vector<float> m_clusgx;
  std::vector<float> m_clusgy;
  std::vector<float> m_clusgz;
  std::vector<float> m_clusgxunmoved;
  std::vector<float> m_clusgyunmoved;
  std::vector<float> m_clusgzunmoved;
  std::vector<uint32_t> m_clushitsetkey;
  std::vector<uint64_t> m_cluskeys;
  std::vector<int> m_cluslayer;
  std::vector<float> m_clusMaxAdc;
  std::vector<int> m_clusphisize;
  std::vector<int> m_clsector;
  std::vector<int> m_clside;
  std::vector<int> m_clussize;
  std::vector<int> m_cluszsize;
  int m_crossing = std::numeric_limits<int>::quiet_NaN();
  float m_dcaxy = std::numeric_limits<float>::quiet_NaN();
  float m_dcaz = std::numeric_limits<float>::quiet_NaN();
  float m_eta = std::numeric_limits<float>::quiet_NaN();
  int m_event = 0;
  float m_ndf = std::numeric_limits<float>::quiet_NaN();
  int m_nhits = std::numeric_limits<int>::quiet_NaN();
  int m_nintt = std::numeric_limits<int>::quiet_NaN();
  int m_ninttstate = std::numeric_limits<int>::quiet_NaN();
  int m_nmaps = std::numeric_limits<int>::quiet_NaN();
  int m_nmapsstate = std::numeric_limits<int>::quiet_NaN();
  int m_nmms = std::numeric_limits<int>::quiet_NaN();
  int m_nmmsstate = std::numeric_limits<int>::quiet_NaN();
  int m_ntpc = std::numeric_limits<int>::quiet_NaN();
  int m_ntpcstate = std::numeric_limits<int>::quiet_NaN();
  float m_phi = std::numeric_limits<float>::quiet_NaN();
  float m_pt = std::numeric_limits<float>::quiet_NaN();
  float m_px = std::numeric_limits<float>::quiet_NaN();
  float m_py = std::numeric_limits<float>::quiet_NaN();
  float m_pz = std::numeric_limits<float>::quiet_NaN();
  float m_quality = std::numeric_limits<float>::quiet_NaN();
  int m_runnumber = -1;
  int m_tileid = std::numeric_limits<int>::quiet_NaN();

  // Private member variables for function arguments
  std::string m_clusterContainerName = "TRKR_CLUSTER";
  std::string m_trackMapName = "SvtxTrackMap";
};

#endif  // TRACKRESIDUALS_KSHORT_H
