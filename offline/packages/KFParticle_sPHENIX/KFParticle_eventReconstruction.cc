/*
 * This file is part of KFParticle package
 * Copyright ( C ) 2007-2019 FIAS Frankfurt Institute for Advanced Studies
 *               2007-2019 Goethe University of Frankfurt
 *               2007-2019 Ivan Kisel <I.Kisel@compeng.uni-frankfurt.de>
 *               2007-2019 Maksym Zyzak
 *
 * KFParticle is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * ( at your option ) any later version.
 *
 * KFParticle is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/*****************/
/* Cameron Dean  */
/*   LANL 2020   */
/* cdean@bnl.gov */
/*****************/

#include "KFParticle_eventReconstruction.h"
#include "KFParticle_Tools.h"
#include "KFParticle_truthAndDetTools.h"

//sPHENIX stuff
#include <trackbase_historic/SvtxTrack.h>

// KFParticle stuff
#include <KFParticle.h>

#include <algorithm>
#include <cassert>
#include <iterator>  // for begin, distance, end
#include <memory>   // for allocator_traits<>::value_type
#include <string>   // for string
#include <tuple>    // for tie, tuple

#include <iostream>

/// Create necessary objects
KFParticle_Tools kfp_Tools_evtReco;

/// KFParticle constructor
KFParticle_eventReconstruction::KFParticle_eventReconstruction()
  : m_constrain_to_vertex(false)
  , m_constrain_int_mass(false)
  , m_use_fake_pv(false)
{
}

void KFParticle_eventReconstruction::createDecay(PHCompositeNode* topNode, std::vector<KFParticle>& selectedMother, std::vector<KFParticle>& selectedVertex,
                                                 std::vector<std::vector<KFParticle>>& selectedDaughters,
                                                 std::vector<std::vector<KFParticle>>& selectedIntermediates,
                                                 int& nPVs)
{
  std::vector<KFParticle> primaryVertices;
  if (m_use_fake_pv)
  {
    primaryVertices.push_back(createFakePV());
  }
  else
  {
    primaryVertices = makeAllPrimaryVertices(topNode, m_vtx_map_node_name);
  }

  std::vector<KFParticle> daughterParticles = makeAllDaughterParticles(topNode);

  nPVs = primaryVertices.size();

  std::vector<int> goodTrackIndex = findAllGoodTracks(daughterParticles, primaryVertices);

  if (!m_has_intermediates)
  {
    buildBasicChain(selectedMother, selectedVertex, selectedDaughters, daughterParticles, goodTrackIndex, primaryVertices, topNode);
  }
  else
  {
    buildChain(selectedMother, selectedVertex, selectedDaughters, selectedIntermediates, daughterParticles, goodTrackIndex, primaryVertices, topNode);
  }
}

/*
 *  This function is used to build a basic n-body decay without any intermediate particles such as D's or J/psi's
 */
void KFParticle_eventReconstruction::buildBasicChain(std::vector<KFParticle>& selectedMotherBasic,
                                                     std::vector<KFParticle>& selectedVertexBasic,
                                                     std::vector<std::vector<KFParticle>>& selectedDaughtersBasic,
                                                     const std::vector<KFParticle>& daughterParticlesBasic,
                                                     const std::vector<int>& goodTrackIndexBasic,
                                                     const std::vector<KFParticle>& primaryVerticesBasic, PHCompositeNode* topNode)
{
  std::vector<std::vector<int>> goodTracksThatMeet = findTwoProngs(daughterParticlesBasic, goodTrackIndexBasic, m_num_tracks);
  for (int p = 3; p < m_num_tracks + 1; ++p)
  {
    goodTracksThatMeet = findNProngs(daughterParticlesBasic, goodTrackIndexBasic, goodTracksThatMeet, m_num_tracks, p);
  }

  getCandidateDecay(selectedMotherBasic, selectedVertexBasic, selectedDaughtersBasic, daughterParticlesBasic,
                    goodTracksThatMeet, primaryVerticesBasic, 0, m_num_tracks, false, 0, true, topNode);
}

/*
 *  This function is used to build a more complicated decay with intermediate particles such as D's or J/psi's
 */
void KFParticle_eventReconstruction::buildChain(std::vector<KFParticle>& selectedMotherAdv,
                                                std::vector<KFParticle>& selectedVertexAdv,
                                                std::vector<std::vector<KFParticle>>& selectedDaughtersAdv,
                                                std::vector<std::vector<KFParticle>>& selectedIntermediatesAdv,
                                                const std::vector<KFParticle>& daughterParticlesAdv,
                                                const std::vector<int>& goodTrackIndexAdv,
                                                const std::vector<KFParticle>& primaryVerticesAdv, PHCompositeNode* topNode)
{
  int track_start = 0;
  int track_stop = m_num_tracks_from_intermediate[0];
  std::vector<KFParticle> goodCandidates;
  std::vector<KFParticle> goodVertex;
  std::vector<KFParticle> *goodDaughters = new std::vector<KFParticle>[m_num_tracks];
  std::vector<KFParticle> *goodIntermediates = new std::vector<KFParticle>[m_num_intermediate_states];
  std::vector<KFParticle> *potentialIntermediates = new std::vector<KFParticle>[m_num_intermediate_states];
  std::vector<std::vector<KFParticle>> *potentialDaughters = new std::vector<std::vector<KFParticle>>[m_num_intermediate_states];
  for (int i = 0; i < m_num_intermediate_states; ++i)
  {
    std::vector<KFParticle> vertices;
    std::vector<std::vector<int>> goodTracksThatMeet = findTwoProngs(daughterParticlesAdv, goodTrackIndexAdv, m_num_tracks_from_intermediate[i]);
    for (int p = 3; p <= m_num_tracks_from_intermediate[i]; ++p)
    {
      goodTracksThatMeet = findNProngs(daughterParticlesAdv,
                                       goodTrackIndexAdv,
                                       goodTracksThatMeet,
                                       m_num_tracks_from_intermediate[i], p);
    }
    getCandidateDecay(potentialIntermediates[i], vertices, potentialDaughters[i], daughterParticlesAdv,
                      goodTracksThatMeet, primaryVerticesAdv, track_start, track_stop, true, i, m_constrain_int_mass, topNode);
    track_start += track_stop;
    track_stop += m_num_tracks_from_intermediate[i + 1];
  }
  int num_tracks_used_by_intermediates = 0;
  for (int i = 0; i < m_num_intermediate_states; ++i)
  {
    num_tracks_used_by_intermediates += m_num_tracks_from_intermediate[i];
  }

  int num_remaining_tracks = m_num_tracks - num_tracks_used_by_intermediates;
  unsigned int num_pot_inter_a, num_pot_inter_b, num_pot_inter_c, num_pot_inter_d;  // Number of potential intermediates found
  num_pot_inter_a = potentialIntermediates[0].size();
  num_pot_inter_b = m_num_intermediate_states < 2 ? 1 : potentialIntermediates[1].size();  // Ensure the code inside the loop below is executed
  num_pot_inter_c = m_num_intermediate_states < 3 ? 1 : potentialIntermediates[2].size();
  num_pot_inter_d = m_num_intermediate_states < 4 ? 1 : potentialIntermediates[3].size();

  for (unsigned int a = 0; a < num_pot_inter_a; ++a)
  {
    for (unsigned int b = 0; b < num_pot_inter_b; ++b)
    {
      for (unsigned int c = 0; c < num_pot_inter_c; ++c)
      {
        for (unsigned int d = 0; d < num_pot_inter_d; ++d)
        {
          KFParticle candidate;
          bool isGood = false;
          const unsigned int matchIterators[4] = {a, b, c, d};

          int num_mother_decay_products = m_num_intermediate_states + num_remaining_tracks;
          assert(num_mother_decay_products > 0);
          KFParticle *motherDecayProducts = new KFParticle[num_mother_decay_products];
          std::vector<KFParticle> finalTracks = potentialDaughters[0][a];

          for (int i = 0; i < m_num_intermediate_states; ++i)
          {
            motherDecayProducts[i] = potentialIntermediates[i][matchIterators[i]];
          }
          for (int j = 1; j < m_num_intermediate_states; ++j)
          {
            finalTracks.insert(finalTracks.end(), potentialDaughters[j][matchIterators[j]].begin(), potentialDaughters[j][matchIterators[j]].end());
          }

          bool have_duplicate_track = false;

          for (size_t i = 0; i < finalTracks.size(); ++i)
          {
            for (size_t j = i + 1; j < finalTracks.size(); ++j)
            {
              if (finalTracks[i].Id() == finalTracks[j].Id())
              {
                have_duplicate_track = true;
                break;
              }
            }
            if (have_duplicate_track)
            {
              break;
            }
          }

          if (have_duplicate_track)
	  {
	    continue;
	  }

          // If there are daughter tracks coming from the mother not an intermediate, need to ensure that the intermeditate decay tracks aren't used again
          std::vector<int> goodTrackIndexAdv_withoutIntermediates = goodTrackIndexAdv;
          for (int m = 0; m < num_tracks_used_by_intermediates; ++m)
          {
            int trackID_to_remove = finalTracks[m].Id();
            int trackElement_to_remove = -1;

            auto it = std::find_if(daughterParticlesAdv.begin(), daughterParticlesAdv.end(),
                                   [&trackID_to_remove](const KFParticle& obj)
                                   { return obj.Id() == trackID_to_remove; });

            if (it != daughterParticlesAdv.end())
            {
              trackElement_to_remove = std::distance(daughterParticlesAdv.begin(), it);
            }

            goodTrackIndexAdv_withoutIntermediates.erase(remove(goodTrackIndexAdv_withoutIntermediates.begin(),
                                                                goodTrackIndexAdv_withoutIntermediates.end(), trackElement_to_remove),
                                                         goodTrackIndexAdv_withoutIntermediates.end());
          }

          float required_unique_vertexID = 0;
          for (int n = 0; n < m_num_intermediate_states; ++n)
          {
            required_unique_vertexID += m_intermediate_charge[n] * kfp_Tools_evtReco.getParticleMass(m_intermediate_name[n].c_str());
          }

          std::vector<std::vector<int>> uniqueCombinations;
          std::vector<std::vector<int>> listOfTracksToAppend;

          if (num_remaining_tracks != 0)
          {
            for (int i = num_tracks_used_by_intermediates; i < m_num_tracks; ++i)
            {
              required_unique_vertexID += m_daughter_charge[i] * kfp_Tools_evtReco.getParticleMass(m_daughter_name[i].c_str());
            }

            uniqueCombinations = findUniqueDaughterCombinations(num_tracks_used_by_intermediates, m_num_tracks);  // Unique comb of remaining trackIDs

            listOfTracksToAppend = appendTracksToIntermediates(motherDecayProducts, daughterParticlesAdv, goodTrackIndexAdv_withoutIntermediates, num_remaining_tracks);

            for (auto& uniqueCombination : uniqueCombinations)
            {
              for (const auto& element_of_intermediate : m_intermediate_name)
              {
                uniqueCombination.insert(begin(uniqueCombination), kfp_Tools_evtReco.getParticleID(element_of_intermediate));
              }
            }
          }
          else
          {
            std::vector<int> m_intermediate_id;
            m_intermediate_id.clear();
            for (const auto& element_of_intermediate : m_intermediate_name)
            {
              m_intermediate_id.push_back(kfp_Tools_evtReco.getParticleID(element_of_intermediate));
            }
            uniqueCombinations.push_back(m_intermediate_id);
            listOfTracksToAppend.push_back({0});
          }

          for (auto& n_tracks : listOfTracksToAppend)
          {
            for (int n_trackID = 0; n_trackID < num_remaining_tracks; ++n_trackID)
            {
              int mDP_trackElem = m_num_intermediate_states + n_trackID;
              int dP_trackElem = n_tracks[n_trackID];
              motherDecayProducts[mDP_trackElem] = daughterParticlesAdv[dP_trackElem];
            }

            for (auto& uniqueCombination : uniqueCombinations)
            {
              for (const auto& i_pv : primaryVerticesAdv)
              {
                std::tie(candidate, isGood) = getCombination(motherDecayProducts, &uniqueCombination[0], i_pv,
                                                             m_constrain_to_vertex, false, 0, num_mother_decay_products, m_constrain_int_mass, required_unique_vertexID, topNode);
                if (isGood)
                {

                  if (m_require_bunch_crossing_match)
                  {
                    KFParticle_truthAndDetTools toolSet;
                    std::vector<int> crossings;
                    for (int i = 0; i < num_tracks_used_by_intermediates; ++i)
                    {
                      SvtxTrack *thisTrack = toolSet.getTrack(finalTracks[i].Id(), m_dst_trackmap);
                      if (thisTrack)
                      {
                        crossings.push_back(thisTrack->get_crossing());
                      }
                    }
                
                    for (int k = 0; k < num_remaining_tracks; ++k)
                    {
                      int trackArrayID = k + m_num_intermediate_states;
                      SvtxTrack *thisTrack = toolSet.getTrack(motherDecayProducts[trackArrayID].Id(), m_dst_trackmap);
                      if (thisTrack)
                      {
                        crossings.push_back(thisTrack->get_crossing());
                      }
                    }
                
                    removeDuplicates(crossings);
                
                    if (crossings.size() !=1)
                    {
                      continue;
                    }
                  }

                  goodCandidates.push_back(candidate);
                  if (m_constrain_to_vertex)
                  {
                    goodVertex.push_back(i_pv);
                  }
                  for (int k = 0; k < m_num_intermediate_states; ++k)
                  {
                    goodIntermediates[k].push_back(motherDecayProducts[k]);
                  }
                  for (int k = 0; k < num_tracks_used_by_intermediates; ++k)
                  {
                    goodDaughters[k].push_back(finalTracks[k]);
                  }
                  for (int k = 0; k < num_remaining_tracks; ++k)
                  {  // Need to deal with track mass and PID assignment for extra tracks
                    int trackArrayID = k + m_num_intermediate_states;
                    double slowTrackMass=-1;
                    int slowTrackPDG=0;
                    if ((Int_t) motherDecayProducts[trackArrayID].GetQ() != 0)
                    {
                      slowTrackMass = kfp_Tools_evtReco.getParticleMass((Int_t) motherDecayProducts[trackArrayID].GetQ() * uniqueCombination[trackArrayID]);
                      slowTrackPDG = (Int_t) motherDecayProducts[trackArrayID].GetQ() * uniqueCombination[trackArrayID];
                      // pi+ pdgid = 211, pi- pdgid = -211
                      // e+ pdgid = -11, e- pdgid = 11
                      // mu+ pdgid = -13, mu- pdgid = 13
                      // tau+ pdgid = -15, tau- pdgid = 15
                      if (abs(slowTrackPDG) == 11 || abs(slowTrackPDG) == 13 || abs(slowTrackPDG) == 15)
                      {
                        slowTrackPDG *= -1;
                      }
                    }
                    else if ((Int_t) motherDecayProducts[trackArrayID].GetQ() == 0)
                    {
                      slowTrackMass = kfp_Tools_evtReco.getParticleMass(uniqueCombination[trackArrayID]);
                      slowTrackPDG = uniqueCombination[trackArrayID];
                    }
                    KFParticle slowTrack;
                    slowTrack.Create(motherDecayProducts[trackArrayID].Parameters(),
                                     motherDecayProducts[trackArrayID].CovarianceMatrix(),
                                     (Int_t) motherDecayProducts[trackArrayID].GetQ(),
                                     slowTrackMass);
                    slowTrack.NDF() = motherDecayProducts[trackArrayID].GetNDF();
                    slowTrack.Chi2() = motherDecayProducts[trackArrayID].GetChi2();
                    slowTrack.SetId(motherDecayProducts[trackArrayID].Id());
                    slowTrack.SetPDG(slowTrackPDG);
                    goodDaughters[k + num_tracks_used_by_intermediates].push_back(slowTrack);
                  }
                }
              }
            }

            if (goodCandidates.size() != 0)
            {
              int bestCombinationIndex = selectBestCombination(m_constrain_to_vertex, false, goodCandidates, goodVertex);

              selectedMotherAdv.push_back(goodCandidates[bestCombinationIndex]);
              if (m_constrain_to_vertex)
              {
                selectedVertexAdv.push_back(goodVertex[bestCombinationIndex]);
              }
              std::vector<KFParticle> intermediates;
              intermediates.reserve(m_num_intermediate_states);
              for (int i = 0; i < m_num_intermediate_states; ++i)
              {
                intermediates.push_back(goodIntermediates[i][bestCombinationIndex]);
              }
              selectedIntermediatesAdv.push_back(intermediates);
              std::vector<KFParticle> particles;
              particles.reserve(m_num_tracks);
              for (int i = 0; i < m_num_tracks; ++i)
              {
                particles.push_back(goodDaughters[i][bestCombinationIndex]);
              }
              selectedDaughtersAdv.push_back(particles);
            }
            goodCandidates.clear();
            goodVertex.clear();
            for (int j = 0; j < m_num_intermediate_states; ++j)
            {
              goodIntermediates[j].clear();
            }
            for (int j = 0; j < m_num_tracks; ++j)
            {
              goodDaughters[j].clear();
            }
          }
	  delete [] motherDecayProducts;
        }  // Close forth intermediate
      }    // Close third intermediate
    }      // Close second intermediate
  }        // Close first intermediate
  delete [] goodDaughters;
  delete [] goodIntermediates;
  delete [] potentialIntermediates;
}

void KFParticle_eventReconstruction::getCandidateDecay(std::vector<KFParticle>& selectedMotherCand,
                                                       std::vector<KFParticle>& selectedVertexCand,
                                                       std::vector<std::vector<KFParticle>>& selectedDaughtersCand,
                                                       const std::vector<KFParticle>& daughterParticlesCand,
                                                       const std::vector<std::vector<int>>& goodTracksThatMeetCand,
                                                       const std::vector<KFParticle>& primaryVerticesCand,
                                                       int n_track_start, int n_track_stop,
                                                       bool isIntermediate, int intermediateNumber, bool constrainMass, PHCompositeNode* topNode)
{
  int nTracks = n_track_stop - n_track_start;
  std::vector<std::vector<int>> uniqueCombinations = findUniqueDaughterCombinations(n_track_start, n_track_stop);
  std::vector<KFParticle> goodCandidates, goodVertex;
  std::vector<KFParticle> *goodDaughters = new std::vector<KFParticle>[nTracks];
  KFParticle candidate;
  bool isGood;
  bool fixToPV = m_constrain_to_vertex && !isIntermediate;

  float required_unique_vertexID = 0;
  for (int i = n_track_start; i < n_track_stop; ++i)
  {
    required_unique_vertexID += m_daughter_charge[i] * kfp_Tools_evtReco.getParticleMass(m_daughter_name[i].c_str());
  }

  for (auto& i_comb : goodTracksThatMeetCand)  // Loop over all good track combinations
  {
    KFParticle *daughterTracks = new KFParticle[nTracks];

    for (int i_track = 0; i_track < nTracks; ++i_track)
    {
      daughterTracks[i_track] = daughterParticlesCand[i_comb[i_track]];
    }  // Build array of the good tracks in that combination

    for (auto& uniqueCombination : uniqueCombinations)  // Loop over unique track PID assignments
    {
      for (unsigned int i_pv = 0; i_pv < primaryVerticesCand.size(); ++i_pv)  // Loop over all PVs in the event
      {
        int* PDGIDofFirstParticleInCombination = &uniqueCombination[0];
        std::tie(candidate, isGood) = getCombination(daughterTracks, PDGIDofFirstParticleInCombination, primaryVerticesCand[i_pv], m_constrain_to_vertex,
                                                     isIntermediate, intermediateNumber, nTracks, constrainMass, required_unique_vertexID, topNode);
        if (isIntermediate && isGood)
        {
          float min_ip = 0;
          float min_ipchi2 = 0;
          float min_ip_xy = 0;
          float min_ipchi2_xy  = 0;
          calcMinIP(candidate, primaryVerticesCand, min_ip, min_ipchi2);
          calcMinIP(candidate, primaryVerticesCand, min_ip_xy , min_ipchi2_xy, false);
          if (!isInRange(m_intermediate_min_ip[intermediateNumber], min_ip, m_intermediate_max_ip[intermediateNumber])
           || !isInRange(m_intermediate_min_ipchi2[intermediateNumber], min_ipchi2, m_intermediate_max_ipchi2[intermediateNumber])
           || !isInRange(m_intermediate_min_ip_xy[intermediateNumber], min_ip_xy, m_intermediate_max_ip_xy[intermediateNumber])
           || !isInRange(m_intermediate_min_ipchi2_xy[intermediateNumber], min_ipchi2_xy, m_intermediate_max_ipchi2_xy[intermediateNumber]))
          {
            isGood = false;
          }
        }

        if (isGood)
        {
          goodCandidates.push_back(candidate);
          goodVertex.push_back(primaryVerticesCand[i_pv]);
          for (int i = 0; i < nTracks; ++i)
          {
            KFParticle intParticle;
            double intParticleMass=-1;
            int intParticlePDG=0;
            if ((Int_t) daughterTracks[i].GetQ() != 0)
            {
              intParticleMass = kfp_Tools_evtReco.getParticleMass((Int_t) daughterTracks[i].GetQ() * PDGIDofFirstParticleInCombination[i]);
              intParticlePDG = (Int_t) daughterTracks[i].GetQ() * PDGIDofFirstParticleInCombination[i];
              // pi+ pdgid = 211, pi- pdgid = -211
              // e+ pdgid = -11, e- pdgid = 11
              // mu+ pdgid = -13, mu- pdgid = 13
              // tau+ pdgid = -15, tau- pdgid = 15
              if (abs(intParticlePDG) == 11 || abs(intParticlePDG) == 13 || abs(intParticlePDG) == 15)
              {
                intParticlePDG *= -1;
              }
            }
            else if ((Int_t) daughterTracks[i].GetQ() == 0)
            {
              intParticleMass = kfp_Tools_evtReco.getParticleMass(PDGIDofFirstParticleInCombination[i]);
              intParticlePDG = PDGIDofFirstParticleInCombination[i];
            }
            intParticle.Create(daughterTracks[i].Parameters(),
                               daughterTracks[i].CovarianceMatrix(),
                               (Int_t) daughterTracks[i].GetQ(),
                               intParticleMass);
            intParticle.NDF() = daughterTracks[i].GetNDF();
            intParticle.Chi2() = daughterTracks[i].GetChi2();
            intParticle.SetId(daughterTracks[i].Id());
            intParticle.SetPDG(intParticlePDG);
            goodDaughters[i].push_back(intParticle);
          }
        }
      }
    }

    if (goodCandidates.size() != 0)
    {
      int bestCombinationIndex = selectBestCombination(fixToPV, isIntermediate, goodCandidates, goodVertex);

      selectedMotherCand.push_back(goodCandidates[bestCombinationIndex]);
      if (fixToPV)
      {
        selectedVertexCand.push_back(goodVertex[bestCombinationIndex]);
      }
      std::vector<KFParticle> particles;
      particles.reserve(nTracks);
      for (int i = 0; i < nTracks; ++i)
      {
        particles.push_back(goodDaughters[i][bestCombinationIndex]);
      }
      selectedDaughtersCand.push_back(particles);

      goodCandidates.clear();
      goodVertex.clear();
      for (int j = 0; j < nTracks; ++j)
      {
        goodDaughters[j].clear();
      }
    }
    delete [] daughterTracks;
  }
  delete [] goodDaughters;
}

int KFParticle_eventReconstruction::selectBestCombination(bool PVconstraint, bool isAnInterMother,
                                                          std::vector<KFParticle> possibleCandidates,
                                                          std::vector<KFParticle> possibleVertex)
{
  KFParticle smallestMassError = possibleCandidates[0];
  int bestCombinationIndex = 0;
  for (unsigned int i = 1; i < possibleCandidates.size(); ++i)
  {
    if (PVconstraint && !isAnInterMother)// && !m_require_track_and_vertex_match)
    {
      float current_IPchi2 = 0;
      float best_IPchi2 = 0;
   
      if (m_use_2D_matching_tools)
      {
        current_IPchi2 = possibleCandidates[i].GetDeviationFromVertexXY(possibleVertex[i]);
        best_IPchi2 = smallestMassError.GetDeviationFromVertexXY(possibleVertex[bestCombinationIndex]);
      }
      else
      {
        current_IPchi2 = possibleCandidates[i].GetDeviationFromVertex(possibleVertex[i]);
        best_IPchi2 = smallestMassError.GetDeviationFromVertex(possibleVertex[bestCombinationIndex]);
      }

      if (current_IPchi2 < best_IPchi2)
      {
        smallestMassError = possibleCandidates[i];
        bestCombinationIndex = i;
      }
    }
    else
    {
      if (possibleCandidates[i].GetErrMass() < smallestMassError.GetErrMass())
      {
        smallestMassError = possibleCandidates[i];
        bestCombinationIndex = i;
      }
    }
  }
  return bestCombinationIndex;
}

KFParticle KFParticle_eventReconstruction::createFakePV()
{
  float f_vertexParameters[6] = {0};

  float f_vertexCovariance[21] = {0};

  KFParticle kfp_vertex;
  kfp_vertex.Create(f_vertexParameters, f_vertexCovariance, 0, -1);
  kfp_vertex.NDF() = 0;
  kfp_vertex.Chi2() = 0;
  kfp_vertex.SetId(0);
  return kfp_vertex;
}
