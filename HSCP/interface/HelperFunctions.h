#ifndef HELPERFUNCTIONS_H
#define HELPERFUNCTIONS_H

#include "boost/date_time/posix_time/posix_time.hpp"
#include <sys/time.h>
#include <regex>

//=============================================================
//     Get HSCP Type Name
//=============================================================
inline std::string getTypeName(int t){
  std::string typeName = "";
  if      (t == 0) typeName = "globalMuon";
  else if (t == 1) typeName = "trackerMuon";
  else if (t == 2) typeName = "matchedStandAloneMuon";
  else if (t == 3) typeName = "standAloneMuon";
  else if (t == 4) typeName = "innerTrack";
  else if (t == 5) typeName = "unknown";
  return typeName;
}

//=============================================================
//     Check if the GenIDs are for a HSCP
//=============================================================
template<typename G>
bool isHSCPgenID(G& gen) {
  int thePDGidForCandidate = abs(gen->pdgId());
// R-hadrons
  if (   thePDGidForCandidate == 1000993 || thePDGidForCandidate == 1009113
      || thePDGidForCandidate == 1009223 || thePDGidForCandidate == 1009313
      || thePDGidForCandidate == 1009333 || thePDGidForCandidate == 1092114
      || thePDGidForCandidate == 1093214 || thePDGidForCandidate == 1093324
      || thePDGidForCandidate == 1000622 || thePDGidForCandidate == 1000642
      || thePDGidForCandidate == 1006113 || thePDGidForCandidate == 1006311
      || thePDGidForCandidate == 1006313 || thePDGidForCandidate == 1006333) {
    return true;
  }
// Single-charged HSCP
  else if (   thePDGidForCandidate == 1009213 || thePDGidForCandidate == 1009323
       || thePDGidForCandidate == 1091114 || thePDGidForCandidate == 1092214
       || thePDGidForCandidate == 1093114 || thePDGidForCandidate == 1093224
       || thePDGidForCandidate == 1093314 || thePDGidForCandidate == 1093334
       || thePDGidForCandidate == 1000612 || thePDGidForCandidate == 1000632
       || thePDGidForCandidate == 1000652 || thePDGidForCandidate == 1006211
       || thePDGidForCandidate == 1006213 || thePDGidForCandidate == 1006321
       || thePDGidForCandidate == 1006323 || thePDGidForCandidate == 1000015) {
    return true;
  }
// Double-charged R-hadrons
  else if (thePDGidForCandidate == 1092224 || thePDGidForCandidate == 1006223) {
    return true;
  }
// tau prime, could be single or multiple charged
  else if (thePDGidForCandidate == 17) {
    return true;
  } else {
    return false;
  }
}

template <typename G, typename T>
int findBestHSCPMatch(G& gen, T& tk, float maxDeltaR=0.015) {
  float minDeltaR = 999.0;
  int idx = -1;
  for (auto g = gen.begin(); g != gen.end(); g++) {
    if (!isHSCPgenID(g)) continue;
    if (g->status() != 1) continue;
    if (g->pt() < 5) continue;
    const auto tmp = reco::deltaR(g->eta(), g->phi(), tk->eta(), tk->phi());
    if (tmp < minDeltaR) {
      minDeltaR = tmp;
      idx = g - gen.begin();
    }
  }
  if (idx > -1  && minDeltaR > maxDeltaR) idx = -2;
  return idx;
}


template<typename F, typename M, typename V>
bool findBestHLTMuonMatch(std::vector<F> trigObjs, std::vector<M>& muons, V highestSumPt2Vertex) {
    bool matchedMuonWasFound = false;

    float globalMaxEta_ = 1;

    //[-Wunused-variable]int closestTrigMuIndex = -1;
    //[-Wunused-variable]int closestTrigMuPt25Index = -1;
    //[-Wunused-variable]int closestTrigObjIndex = -1;
    int closestTrigObjIndexLoose = -1;
    //[-Wunused-variable]float dr_min_hltMuon_hscpCand_inEvent = 9999.0;
    float dr_min_hltMuon_hscpCandPt25_inEvent = 9999.0;
    float dr_min_hltMuonLoose_hscpCand_inEvent = 9999.0;
    //[-Wunused-variable]float dr_minGlobally_hltMuon_hscpCand_inEvent = 9999.0;
    //[-Wunused-variable]int numPassedMatchingTrigObj = 0;
    //[-Wunused-variable]int numPassedMatchingTrigObjEtaCut = 0;
    //[-Wunused-variable]bool doNotMatchThis = false;
    bool doNotMatchThisLoose = false;

    for(size_t objNr=0; objNr<trigObjs.size(); objNr++){
        auto trigObj = trigObjs[objNr];
        if (trigObj.Pt() < 50) continue;

        //float minDeltaR = 9999.0;
        //float dr_min_hltMuon_trigObj = 9999.0;
        //[-Wunused-variable]int idx = -1;
        for (unsigned int i=0; i<muons.size(); i++){
            auto *mu = &(muons)[i];
            const auto tmp = reco::deltaR(trigObj.Eta(),trigObj.Phi(),mu->eta(),mu->phi());
            if (muon::isLooseMuon(*mu) && mu->pt() > 50){
                if (tmp < dr_min_hltMuonLoose_hscpCand_inEvent){
                    if (closestTrigObjIndexLoose > -1){
                        if ((trigObjs[closestTrigObjIndexLoose].Eta() < globalMaxEta_) && (dr_min_hltMuonLoose_hscpCand_inEvent < 0.015)){
                            doNotMatchThisLoose = true;
                        }
                    }
                    if (!doNotMatchThisLoose){
                        dr_min_hltMuonLoose_hscpCand_inEvent = tmp;
                        closestTrigObjIndexLoose = objNr;
                    }
                    //minDeltaR = tmp;
                    //idx = i;
                }
            }// end condition on loose mu ID

            if(!muon::isTightMuon(*mu, highestSumPt2Vertex)) continue;

            // For checking the trigger pT let's consider tracks with pT > 25 GeV
            if (mu->pt() < 25)  continue;

            const auto drTemp = reco::deltaR(trigObj.Eta(),trigObj.Phi(),mu->eta(),mu->phi());
            if (drTemp < dr_min_hltMuon_hscpCandPt25_inEvent) {
                dr_min_hltMuon_hscpCandPt25_inEvent = drTemp;
                //\\closestTrigMuPt25Index = i;
            }
        }// end loop on muon objects
    }// end loop on trigger objects

    if (dr_min_hltMuon_hscpCandPt25_inEvent < 0.15) matchedMuonWasFound = true;

  return matchedMuonWasFound;
}

// matchPFCandToTrack
template <typename P, typename T>
int findBestPFCandMatch(P& pfCands, T& tk){
  int idx = -1;
  for (auto cand = pfCands.begin(); cand != pfCands.end(); cand++) {
    idx+=1;
    if (cand->trackRef().isNull())
      continue;
    //int type = cand->particleId();
    // only charged hadrons and leptons can be asscociated with a track
    // if (!(type == PFCandidate::h ||type == PFCandidate::e ||type == PFCandidate::mu))
    //   continue;
    if(cand->trackRef().key() == tk.key()){
      idx = cand - pfCands.begin();
      break;
    }
  }
  return idx;
}

// matchPFCandToTrack
template <typename C, typename T>
float getTrackMiniIso(C tc, T& tk){
  float track_genTrackMiniIsoSumPt = 0;
  float track_genTrackMiniIsoSumPtFix = 0;
  for(unsigned int i=0; i<tc->size(); i++){
    T tr = T( tc, i );
    if (tr.isNonnull() && tr.key() != tk.key()){
      float drForMiniIsoFix = 0.3;

      float drForMiniIso = 0.0;
      if (tk->pt() < 50 ) {
        drForMiniIso = 0.2;
      } else if (tk->pt() < 200) {
        drForMiniIso = 10/tk->pt();
      } else {
        drForMiniIso = 0.05;
      }
      float pt = tr->pt();
      float dr = reco::deltaR(tr->eta(),tr->phi(),tk->eta(),tk->phi());
      if(dr < drForMiniIsoFix) {
        track_genTrackMiniIsoSumPtFix+=pt;
      }

      if (dr<drForMiniIso) {
        track_genTrackMiniIsoSumPt+=pt;
      }
    }
  }
  return track_genTrackMiniIsoSumPt;
}

// template <typename P, typename T>
// void getMiniPFIsolation(std::vector<P>& pfCands, T& tk, float dz, float &miniRelIsoChg, float &miniRelIsoAll){
//   miniRelIsoChg = 0; miniRelIsoAll = 0;
//   float track_PFMiniIso_sumCharHadPt = 0;
//   float track_PFMiniIso_sumNeutHadPt = 0;
//   float track_PFMiniIso_sumLeptonPt  = 0;
//   float track_PFMiniIso_sumPhotonPt  = 0;
//   float track_PFMiniIso_sumPUPt      = 0
//   float track_PFMiniIso_otherPt      = 0;

//   float pf_energy = 0;
//   float pf_ecal_energy = 0;
//   float pf_hcal_energy = 0;

//   std::vector<double> miniIsoParams = {0.05, 0.2, 10.0};
//   float miniDR = std::max(miniIsoParams[0], std::min(miniIsoParams[1], miniIsoParams[2] / tk->pt()));
//   for (auto cand = pfCands.begin(); cand != pfCands.end(); cand++) {

//     if (cand->trackRef().isNull())
//       continue;
//     if (cand->trackRef().key() != tk.key())
//       continue;
//     const float pfIsolation_DZ = 0.1;
//     bool fromPV = (fabs(dz) < pfIsolation_DZ);
//     //bool pf_isChHadron = fabs(cand->pdgId()) == 211;
//     bool pf_isChHadron   = cand->translatePdgIdToType(cand->pdgId()) == P::ParticleType::h;
//     bool pf_isNeutHadron = cand->translatePdgIdToType(cand->pdgId()) == P::ParticleType::h0;
//     bool pf_isElectron   = cand->translatePdgIdToType(cand->pdgId()) == P::ParticleType::e;
//     bool pf_isMuon       = cand->translatePdgIdToType(cand->pdgId()) == P::ParticleType::mu;
//     bool pf_isPhoton     = cand->translatePdgIdToType(cand->pdgId()) == P::ParticleType::gamma;

//     pf_energy = cand->ecalEnergy() + pfCand->hcalEnergy();
//     pf_ecal_energy = cand->ecalEnergy();
//     pf_hcal_energy = cand->hcalEnergy();

//     float dr = deltaR(cand->eta(),cand->phi(),tk->eta(),tk->phi());

//     if (dr<miniDR){
//       if(pf_isChHadron && fromPV) track_PFMiniIso_sumCharHadPt+=cand->pt();//cand->p4().pt();
//     }

//   }
//   miniRelIsoChg = track_PFMiniIso_sumCharHadPt/tk->pt();
//   miniRelIsoAll = (track_PFMiniIso_sumLeptonPt + track_PFMiniIso_otherPt + track_PFMiniIso_sumCharHadPt + std::max(0.0, track_PFMiniIso_sumNeutHadPt + track_PFMiniIso_sumPhotonPt - 0.5* track_PFMiniIso_sumPUPt))/track->pt();
// }

// template <typename P, typename T>
// void getMiniPFIsolation(std::vector<P>& pfCands, T& tk, float dz, float &miniRelIsoChg){
//   miniRelIsoChg = 0;
//   float track_PFMiniIso_sumCharHadPt = 0;
//   for (auto cand = pfCands.begin(); cand != pfCands.end(); cand++) {

//     if (cand->trackRef().isNull())
//       continue;
//     if (cand->trackRef().key() == tk.key())
//       continue;
//     bool pf_isChHadron = cand->translatePdgIdToType(cand->pdgId()) == P::ParticleType::h;

//     const float pfIsolation_DZ = 0.1;
//     bool fromPV = (fabs(dz) < pfIsolation_DZ);

//     float pt = cand->p4().pt();
//     float drForMiniIso = 0.0;
//     if      (tk->pt() < 50 ) drForMiniIso = 0.2;
//     else if (tk->pt() < 200) drForMiniIso = 10/tk->pt();
//     else                     drForMiniIso = 0.05;

//     float dr = deltaR(cand->eta(),cand->phi(),tk->eta(),tk->phi());
//     if (dr<drForMiniIso){
//       if(pf_isChHadron && fromPV) track_PFMiniIso_sumCharHadPt+=pt;
//     }

//     //int type = cand->particleId();
//     // only charged hadrons and leptons can be asscociated with a track
//     // if (!(type == PFCandidate::h ||type == PFCandidate::e ||type == PFCandidate::mu))
//     //   continue;

//   }
//   miniRelIsoChg = track_PFMiniIso_sumCharHadPt/tk->pt();
// }

//=============================================================
//     Time stamp
//=============================================================
template<typename EVT>
inline std::string getLocalTime(EVT& event){
  time_t t(event.time().value() >> 32);
  std::string text(asctime(localtime(&t)));
  size_t pos = text.find('\n');
  if (pos != std::string::npos) text = text.substr(0, pos);
  text += " ";
  if (daylight)
    text += tzname[1];
  else
    text += tzname[0];
  return text;
}
template<typename EVT>
inline std::string getYear(EVT& event){
  std::string ts = getLocalTime(event);//std::cout<<"DEBUG TimeStamp: "<<ts<<std::endl;
  vector<std::string> v;
  std::string s;
  for (uint i = 0; i < ts.length(); i++){
    if (ts[i] == ' '){
      v.push_back(s);
      s = "";
    } else {
      s += ts[i];
    }
  }
  if (s != "")
    v.push_back(s);
  return v[4];
}

/*const std::string removeTriggerVersion(const std::string& trigger) {
  const std::regex regexp("_v[0-9]+$");
  return std::regex_replace(trigger, regexp, "");
}*/


#endif /* HELPERFUNCTIONS_H */