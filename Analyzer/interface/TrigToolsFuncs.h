#ifndef RHABERLE_TRIGTOOLSFUNCS
#define RHABERLE_TRIGTOOLSFUNCS

#include "DataFormats/Math/interface/LorentzVector.h"
#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"
#include "DataFormats/Math/interface/deltaR.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"

#include "TLorentzVector.h"

#include<vector>
#include<string>


namespace trigger{
  class TriggerEvent;
}

namespace trigtools {
  bool passedHLT(const edm::TriggerNames triggerNames, const edm::Handle<edm::TriggerResults> triggerResults, std::vector<std::string> triggerPaths, std::vector<bool> &triggerDecisions);
  
  //this function takes the trigger event, a filtername + hlt process name (usally HLT but different if 
  //  //HLT was re-run) and returns a vector of the four-momenta of all objects passing the filter
  //
  void getP4sOfObsPassingFilter(std::vector<math::XYZTLorentzVector>& p4s,const trigger::TriggerEvent& trigEvent,const std::string& filterName,const std::string& hltProcess="HLT");
  
  //a TLorentzVector version for my ntuplising needs
  void getP4sOfObsPassingFilter(std::vector<TLorentzVector>& p4s,const trigger::TriggerEvent& trigEvent,const std::string& filterName,const std::string& hltProcess="HLT");
            
  void dumpTriggerEvent(const trigger::TriggerEvent& trigEvt);

  bool passedFilter(const trigger::TriggerEvent& trigEvt,const std::string& givenFilter);

  //template<typename M, typename V>
  bool findBestHLTMuonMatch(std::vector<TLorentzVector> trigObjs, std::vector<reco::Muon>& muons, reco::Vertex highestSumPt2Vertex);

  std::map<std::string, float> getHLTMETOjects(const trigger::TriggerEvent& trigEvent);
  std::map<std::string, float> getHLTMETOjects(const pat::TriggerObjectStandAloneCollection& triggerObjects);
}
  
#endif
