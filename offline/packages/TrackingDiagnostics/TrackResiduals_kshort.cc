#include "TrackResiduals_kshort.h"

//Includes for reference classes
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllServer.h>
#include <g4detectors/PHG4TpcCylinderGeomContainer.h>
#include <micromegas/MicromegasDefs.h>
#include <phool/getClass.h>
#include <trackbase/TpcDefs.h>
#include <trackbase/TrkrCluster.h>
#include <trackbase/TrkrClusterContainer.h>
#include <trackbase/TrkrDefs.h>
#include <trackbase_historic/ActsTransformations.h>
#include <trackbase_historic/SvtxTrack.h>
#include <trackbase_historic/SvtxTrackState.h>
#include <trackbase_historic/SvtxTrackMap.h>
#include <globalvertex/SvtxVertexMap.h>
#include <trackbase_historic/TrackAnalysisUtils.h>

// create anonymous namespace with functions that can only be used inside this
// source file
namespace
{
  // Function to square two numbers
  template <class T>
  inline T square(const T& t)
  {
    return t * t;
  }
  // Function to compute radius of two numbers
  template <class T>
  inline T r(const T& x, const T& y)
  {
    return std::sqrt(square(x) + square(y));
  }

  std::vector<TrkrDefs::cluskey> get_cluster_keys(SvtxTrack* track)
  {
    std::vector<TrkrDefs::cluskey> out;
    for (const auto& seed : {track->get_silicon_seed(), track->get_tpc_seed()})
    {
      if (seed)
      {
        std::copy(seed->begin_cluster_keys(), seed->end_cluster_keys(), std::back_inserter(out));
      }
    }

    return out;
  }
}

//____________________________________________________________________________..
TrackResiduals_kshort::TrackResiduals_kshort(const std::string& name)
  : SubsysReco(name)
{
}

//____________________________________________________________________________..
TrackResiduals_kshort::~TrackResiduals_kshort() = default;

//____________________________________________________________________________..
int TrackResiduals_kshort::Init(PHCompositeNode* /*unused*/)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int TrackResiduals_kshort::InitRun(PHCompositeNode* topNode)
{
  // create output file
  m_outfile = new TFile(m_outfileName.c_str(), "RECREATE");
  // create branches for ttrees that will be filled later on
  createBranches(); 

  // global position wrapper
  m_globalPositionWrapper.loadNodes(topNode);

  // clusterMover needs the correct radii of the TPC layers
  auto tpccellgeo = findNode::getClass<PHG4TpcCylinderGeomContainer>(topNode, "CYLINDERCELLGEOM_SVTX");
  m_clusterMover.initialize_geometry(tpccellgeo);
  m_clusterMover.set_verbosity(0);

  auto se = Fun4AllServer::instance();
  m_runnumber = se->RunNumber();

  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int TrackResiduals_kshort::process_event(PHCompositeNode* topNode)
{
  auto clustermap = findNode::getClass<TrkrClusterContainer>(topNode, m_clusterContainerName);
  auto trackmap = findNode::getClass<SvtxTrackMap>(topNode, m_trackMapName);

  // Check to make sure nodes are present and, if not, abort process_event
  if (!trackmap or !clustermap)
  {
    std::cout << "Missing node, can't continue" << std::endl;
    return Fun4AllReturnCodes::ABORTEVENT;
  }
  
  fillResidualTreeKF(topNode);
  m_event++;
  clearClusterStateVectors();
  return Fun4AllReturnCodes::EVENT_OK;
}

//____________________________________________________________________________..
int TrackResiduals_kshort::End(PHCompositeNode* /*unused*/)
{
  return Fun4AllReturnCodes::EVENT_OK;
}

// Clears data from vectors
void TrackResiduals_kshort::clearClusterStateVectors()
{
  m_clusAdc.clear();
  m_clusgx.clear();
  m_clusgy.clear();
  m_clusgz.clear();
  m_clusgxunmoved.clear();
  m_clusgyunmoved.clear();
  m_clusgzunmoved.clear();
  m_clushitsetkey.clear();
  m_cluskeys.clear();
  m_cluslayer.clear();
  m_clusMaxAdc.clear();
  m_clusphisize.clear();
  m_clsector.clear();
  m_clside.clear();
  m_clussize.clear();
  m_cluszsize.clear();
}

// Define branches for TTrees
void TrackResiduals_kshort::createBranches()
{
  m_tree = new TTree("residualtree","A tree with track, cluster, and state info");
  m_tree->Branch("charge", &m_charge, "m_charge/I");
  m_tree->Branch("chisq", &m_chisq, "m_chisq/F");
  m_tree->Branch("clusAdc", &m_clusAdc);
  m_tree->Branch("clusgr", &m_clusgr);
  m_tree->Branch("clusgx", &m_clusgx);
  m_tree->Branch("clusgy", &m_clusgy);
  m_tree->Branch("clusgz", &m_clusgz);
  m_tree->Branch("clusgxunmoved", &m_clusgxunmoved);
  m_tree->Branch("clusgyunmoved", &m_clusgyunmoved);
  m_tree->Branch("clusgzunmoved", &m_clusgzunmoved);
  m_tree->Branch("clushitsetkey", &m_clushitsetkey);
  m_tree->Branch("cluskeys", &m_cluskeys);
  m_tree->Branch("cluslayer", &m_cluslayer);
  m_tree->Branch("cluslx", &m_cluslx);
  m_tree->Branch("cluslz", &m_cluslz);
  m_tree->Branch("cluselx", &m_cluselx);
  m_tree->Branch("cluselz", &m_cluselz);
  m_tree->Branch("clusMaxAdc", &m_clusMaxAdc);
  m_tree->Branch("clusphisize", &m_clusphisize);
  m_tree->Branch("clussector", &m_clsector);
  m_tree->Branch("clusside", &m_clside);
  m_tree->Branch("clussize", &m_clussize);
  m_tree->Branch("cluszsize", &m_cluszsize);
  m_tree->Branch("crossing", &m_crossing, "m_crossing/I");
  m_tree->Branch("dcaxy", &m_dcaxy, "m_dcaxy/F");
  m_tree->Branch("dcaz", &m_dcaz, "m_dcaz/F");
  m_tree->Branch("eta", &m_eta, "m_eta/F");
  m_tree->Branch("event", &m_event, "m_event/I");
  m_tree->Branch("ndf", &m_ndf, "m_ndf/F");
  m_tree->Branch("nhits", &m_nhits, "m_nhits/I");
  m_tree->Branch("nintt", &m_nintt, "m_nintt/I");
  m_tree->Branch("ninttstate", &m_ninttstate, "m_ninttstate/I");
  m_tree->Branch("nmaps", &m_nmaps, "m_nmaps/I");
  m_tree->Branch("nmapsstate", &m_nmapsstate, "m_nmapsstate/I");
  m_tree->Branch("nmms", &m_nmms, "m_nmms/I");
  m_tree->Branch("nmmsstate", &m_nmmsstate, "m_nmmsstate/I");
  m_tree->Branch("ntpc", &m_ntpc, "m_ntpc/I");
  m_tree->Branch("ntpcstate", &m_ntpcstate, "m_ntpcstate/I");
  m_tree->Branch("phi", &m_phi, "m_phi/F");
  m_tree->Branch("pt", &m_pt, "m_pt/F");
  m_tree->Branch("px", &m_px, "m_px/F");
  m_tree->Branch("py", &m_py, "m_py/F");
  m_tree->Branch("pz", &m_pz, "m_pz/F");
  m_tree->Branch("quality", &m_quality, "m_quality/F");
  m_tree->Branch("run", &m_runnumber, "m_runnumber/I");
  m_tree->Branch("tile", &m_tileid, "m_tileid/I");
  m_tree->Branch("trackid", &m_trackid, "m_trackid/I");
  m_tree->Branch("vertexid", &m_vertexid, "m_vertexid/I");
  m_tree->Branch("vx", &m_vx, "m_vx/F");
  m_tree->Branch("vy", &m_vy, "m_vy/F");
  m_tree->Branch("vz", &m_vz, "m_vz/F");
}


// Store cluster information in variables for ttree
void TrackResiduals_kshort::fillClusterBranchesKF(TrkrDefs::cluskey ckey, SvtxTrack* track,const std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>>& global,PHCompositeNode* topNode)
{
  auto clustermap = findNode::getClass<TrkrClusterContainer>(topNode, m_clusterContainerName);
  auto geometry = findNode::getClass<ActsGeometry>(topNode, "ActsGeometry");

  // move the corrected cluster positions back to the original readout surface
  auto global_moved = m_clusterMover.processTrack(global);

  TrkrCluster* cluster = clustermap->findCluster(ckey);

  // loop over global vectors and get this cluster
  Acts::Vector3 clusglob(0, 0, 0);
  for (const auto& pair : global)
  {
    auto thiskey = pair.first;
    clusglob = pair.second;
    if (thiskey == ckey)
    {
      break;
    }
  }

  Acts::Vector3 clusglob_moved(0, 0, 0);
  for (const auto& pair : global_moved)
  {
    auto thiskey = pair.first;
    clusglob_moved = pair.second;
    if (thiskey == ckey)
    {
      break;
    }
  }

  unsigned int layer = TrkrDefs::getLayer(ckey);

  if (Verbosity() > 1)
  {
    std::cout << "Called :fillClusterBranchesKF for ckey " << ckey << " layer " << layer << " trackid " << track->get_id() << " clusglob x " << clusglob(0) << " y " << clusglob(1) << " z " << clusglob(2) << std::endl;
  }

  switch (TrkrDefs::getTrkrId(ckey))
  {
  case TrkrDefs::mvtxId:
    m_nmaps++;
    break;
  case TrkrDefs::inttId:
    m_nintt++;
    break;
  case TrkrDefs::tpcId:
    m_ntpc++;
    m_clsector.push_back(TpcDefs::getSectorId(ckey));
    m_clside.push_back(TpcDefs::getSide(ckey));
    break;
  case TrkrDefs::micromegasId:
    m_nmms++;
    m_tileid = MicromegasDefs::getTileId(ckey);
    break;
  }
  
  SvtxTrackState* state = nullptr;

  // the track states from the Acts fit are fitted to fully corrected clusters, and are on the surface
  for (auto state_iter = track->begin_states();
       state_iter != track->end_states();
       ++state_iter)
  {
    SvtxTrackState* tstate = state_iter->second;
    auto stateckey = tstate->get_cluskey();
    if (stateckey == ckey)
    {
      state = tstate;
      break;
    }
  }

  if (!state)
  {
    if (Verbosity() > 1)
    {
      std::cout << "   no state for cluster " << ckey << "  in layer " << layer << std::endl;
    }
  }
  else
  {
    switch (TrkrDefs::getTrkrId(ckey))
    {
    case TrkrDefs::mvtxId:
      m_nmapsstate++;
      break;
    case TrkrDefs::inttId:
      m_ninttstate++;
      break;
    case TrkrDefs::tpcId:
      m_ntpcstate++;
      break;
    case TrkrDefs::micromegasId:
      m_nmmsstate++;
      break;
    }
  }

  m_cluskeys.push_back(ckey);

  // get new local coords from moved cluster
  Surface surf = geometry->maps().getSurface(ckey, cluster);
  Surface surf_ideal = geometry->maps().getSurface(ckey, cluster); //Unchanged by distortion corrections
  // if this is a TPC cluster, the crossing correction may have moved it across the central membrane, check the surface
  auto trkrid = TrkrDefs::getTrkrId(ckey);
  if (trkrid == TrkrDefs::tpcId)
  {
    TrkrDefs::hitsetkey hitsetkey = TrkrDefs::getHitSetKeyFromClusKey(ckey);
    TrkrDefs::subsurfkey new_subsurfkey = 0;
    surf = geometry->get_tpc_surface_from_coords(hitsetkey, clusglob_moved, new_subsurfkey);
  }
  if (!surf)
  {
    if (Verbosity() > 2)
    {
      std::cout << " Failed to find surface for cluskey " << ckey << std::endl;
    }
    return;
  }

  // get local coordinates
  Acts::Vector2 loc;
  clusglob_moved *= Acts::UnitConstants::cm;  // we want mm for transformations
  Acts::Vector3 normal = surf->normal(geometry->geometry().getGeoContext());
  auto local = surf->globalToLocal(geometry->geometry().getGeoContext(),
                                   clusglob_moved, normal);
  if (local.ok())
  {
    loc = local.value() / Acts::UnitConstants::cm;
  }
  else
  {
    // otherwise take the manual calculation for the TPC
    // doing it this way just avoids the bounds check that occurs in the surface class method
    Acts::Vector3 loct = surf->transform(geometry->geometry().getGeoContext()).inverse() * clusglob_moved;  // global is in mm
    loct /= Acts::UnitConstants::cm;

    loc(0) = loct(0);
    loc(1) = loct(1);
  }

  clusglob_moved /= Acts::UnitConstants::cm;  // we want cm for the tree

  m_cluslx.push_back(loc.x());
  m_cluslz.push_back(loc.y());

  float clusr = r(clusglob_moved.x(), clusglob_moved.y());
  auto para_errors = m_clusErrPara.get_clusterv5_modified_error(cluster,
                                                                clusr, ckey);

  m_cluselx.push_back(sqrt(para_errors.first));
  m_cluselz.push_back(sqrt(para_errors.second));
  m_clusgx.push_back(clusglob_moved.x());
  m_clusgy.push_back(clusglob_moved.y());
  m_clusgz.push_back(clusglob_moved.z());
  m_clusgr.push_back(clusglob_moved.y() > 0 ? clusr : -1 * clusr);
  m_clusgxunmoved.push_back(clusglob.x());
  m_clusgyunmoved.push_back(clusglob.y());
  m_clusgzunmoved.push_back(clusglob.z());
  m_clusAdc.push_back(cluster->getAdc());
  m_clusMaxAdc.push_back(cluster->getMaxAdc());
  m_cluslayer.push_back(TrkrDefs::getLayer(ckey));
  m_clusphisize.push_back(cluster->getPhiSize());
  m_cluszsize.push_back(cluster->getZSize());
  m_clussize.push_back(cluster->getPhiSize() * cluster->getZSize());
  m_clushitsetkey.push_back(TrkrDefs::getHitSetKeyFromClusKey(ckey));
}

// Fills residual tree with track information
void TrackResiduals_kshort::fillResidualTreeKF(PHCompositeNode* topNode)
{
  auto clustermap = findNode::getClass<TrkrClusterContainer>(topNode, m_clusterContainerName);
  auto trackmap = findNode::getClass<SvtxTrackMap>(topNode, m_trackMapName);
  auto vertexmap = findNode::getClass<SvtxVertexMap>(topNode, "SvtxVertexMap");
 
 // Begin loop over tracks
 for (const auto& [key, track] : *trackmap)
   {
     if (!track)
       {
	 continue;
       }

     m_trackid = track->get_id();
     
     m_crossing = track->get_crossing();
    m_px = track->get_px();
    m_py = track->get_py();
    m_pz = track->get_pz();

    m_pt = std::sqrt(square(m_px) + square(m_py));
    m_eta = atanh(m_pz / std::sqrt(square(m_pt) + square(m_pz)));
    m_phi = atan2(m_py, m_px);

    m_charge = track->get_charge();
    m_quality = track->get_quality();
    m_chisq = track->get_chisq();
    m_ndf = track->get_ndf();

    m_nmaps = 0;
    m_nmapsstate = 0;
    m_nintt = 0;
    m_ninttstate = 0;
    m_ntpc = 0;
    m_ntpcstate = 0;
    m_nmms = 0;
    m_nmmsstate = 0;

    m_dcaxy = std::numeric_limits<float>::quiet_NaN();
    m_dcaz = std::numeric_limits<float>::quiet_NaN();

    m_vertexid = track->get_vertex_id();
    if (vertexmap)
    {
      auto vertexit = vertexmap->find(m_vertexid);
      if (vertexit != vertexmap->end())
      {
        auto vertex = vertexit->second;
        m_vx = vertex->get_x();
        m_vy = vertex->get_y();
        m_vz = vertex->get_z();
        Acts::Vector3 v(m_vx, m_vy, m_vz);
        auto dcapair = TrackAnalysisUtils::get_dca(track, v);
        m_dcaxy = dcapair.first.first;
        m_dcaz = dcapair.second.first;
      }
    }

    clearClusterStateVectors();
    if (Verbosity() > 1)
    {
      std::cout << "Track " << key << " has cluster/states"
                << std::endl;
    }

    // get the fully corrected cluster global positions
    std::vector<std::pair<TrkrDefs::cluskey, Acts::Vector3>> global_raw;
    for (const auto& ckey : get_cluster_keys(track))
    {
      auto cluster = clustermap->findCluster(ckey);

      // Fully correct the cluster positions for the crossing and all distortions
      Acts::Vector3 global = m_globalPositionWrapper.getGlobalPositionDistortionCorrected(ckey, cluster, m_crossing );

      // add the global positions to a vector to give to the cluster mover
      global_raw.emplace_back(std::make_pair(ckey, global));
    }

    // move the cluster positions back to the original readout surface in the fillClusterBranchesKF method

    if (!m_doAlignment)
    {
      for (const auto& ckey : get_cluster_keys(track))
      {
        fillClusterBranchesKF(ckey, track, global_raw, topNode);
      }
    }

    if (m_nmms > 0 || !m_doMicromegasOnly)
    {
      m_tree->Fill();
    }

      // pick up edits from here ////////////////////////

    // Acts::Vector3 zero = Acts::Vector3::Zero();
    // auto dcapair = TrackAnalysisUtils::get_dca(track, zero);
    // m_dcaxy = dcapair.first.first;
    // m_dcaz = dcapair.second.first;

    // clearClusterStateVectors();
    // m_tree->Fill();
    
  }  // end loop over tracks
}
