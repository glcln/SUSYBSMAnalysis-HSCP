// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "DataFormats/Common/interface/Handle.h"

#include "AnalysisDataFormats/SUSYBSMObjects/interface/HSCParticle.h"

class  CandidateSelector{
   public:
      inline CandidateSelector(const edm::ParameterSet& iConfig);
      inline bool isSelected(susybsm::HSCParticle& candidate);

      inline bool isSelectedFromMiniAOD(susybsm::HSCParticle& candidate);

      inline static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

      bool  isTrack;
      bool  isMuon;
      bool  isMuonSTA;
      bool  isMuonGB;
      bool  isMuonTK;
      bool  isMTMuon;
      bool  isRpc;
      bool  isEcal;

      int   minTrackHits;
      float minTrackP;
      float minTrackPt;
      float minDedx;

      float minMuonP;
      float minMuonPt;
      float minSAMuonPt;
      float minMTMuonPt;

      float maxMuTimeDtBeta;
      float minMuTimeDtNdof;
      float maxMuTimeCscBeta;
      float minMuTimeCscNdof;
      float maxMuTimeCombinedBeta;
      float minMuTimeCombinedNdof;

      float maxBetaRpc;
      float maxBetaEcal;
};

using namespace edm;
using namespace reco;
using namespace susybsm;




CandidateSelector::CandidateSelector(const edm::ParameterSet& iConfig){
   isTrack               = iConfig.getParameter<bool>   ("onlyConsiderTrack");
   isMuon                = iConfig.getParameter<bool>   ("onlyConsiderMuon");
   isMuonSTA             = iConfig.getParameter<bool>   ("onlyConsiderMuonSTA"); 
   isMuonGB              = iConfig.getParameter<bool>   ("onlyConsiderMuonGB");
   isMuonTK              = iConfig.getParameter<bool>   ("onlyConsiderMuonTK");
   isMTMuon              = iConfig.getParameter<bool>   ("onlyConsiderMTMuon");
   isRpc                 = iConfig.getParameter<bool>   ("onlyConsiderRpc");
   isEcal                = iConfig.getParameter<bool>   ("onlyConsiderEcal");

   minTrackHits          = iConfig.getParameter<int>    ("minTrackHits");
   minTrackP             = iConfig.getParameter<double> ("minTrackP");
   minTrackPt            = iConfig.getParameter<double> ("minTrackPt");

   minDedx               = iConfig.getParameter<double> ("minDedx");

   minMuonP              = iConfig.getParameter<double> ("minMuonP");
   minMuonPt             = iConfig.getParameter<double> ("minMuonPt");
   minSAMuonPt           = iConfig.getParameter<double> ("minMTMuonPt");
   minMTMuonPt           = iConfig.getParameter<double> ("minMTMuonPt");

   maxMuTimeDtBeta       = iConfig.getParameter<double> ("maxMuTimeDtBeta");
   minMuTimeDtNdof       = iConfig.getParameter<double> ("minMuTimeDtNdof");
   maxMuTimeCscBeta      = iConfig.getParameter<double> ("maxMuTimeCscBeta");
   minMuTimeCscNdof      = iConfig.getParameter<double> ("minMuTimeCscNdof");
   maxMuTimeCombinedBeta = iConfig.getParameter<double> ("maxMuTimeCombinedBeta");
   minMuTimeCombinedNdof = iConfig.getParameter<double> ("minMuTimeCombinedNdof");

   maxBetaRpc            = iConfig.getParameter<double> ("maxBetaRpc");
   maxBetaEcal           = iConfig.getParameter<double> ("maxBetaEcal");
}


bool CandidateSelector::isSelectedFromMiniAOD(HSCParticle& candidate)
{
   if(isTrack   && !candidate.hasTrack()){return false;}
   if(isMuon    && !candidate.hasMuon() ){return false;}
   if(isMuonSTA && (!candidate.hasMuon() || candidate.muon()->standAloneMuon().isNull()) ){return false;}
   if(isMuonGB  && (!candidate.hasMuon() || candidate.muon()->combinedMuon  ().isNull()) ){return false;}
   if(isMuonTK  && (!candidate.hasMuon() || candidate.muon()->innerTrack    ().isNull()) ){return false;}
   if(isMTMuon  && !candidate.hasMTMuonRef() ){return false;}
   if(isRpc     && !candidate.hasRpcInfo() ){return false;}
   if(isEcal    && !candidate.hasCaloInfo()){return false;}

   if(candidate.hasTrack()){
      //if(candidate.track()->pseudoTrack().found() < minTrackHits){return false;}
      //if(candidate.track()->p()     < minTrackP   ){return false;}
      //if(candidate.track()->pt()    < minTrackPt  ){return false;}
      const pat::PackedCandidateRef track = candidate.track().packedCandRef();
      if(track->pseudoTrack().found() < minTrackHits){return false;}
      if(track->p()     < minTrackP   ){return false;}
      if(track->pt()    < minTrackPt  ){return false;}

//      Need to be implemented using external dE/dx object
//      if(candidate.hasDedxEstim1()   && minDedxEstimator1>=0     && candidate.dedxEstimator1    ().dEdx()<minDedxEstimator1)    {return false;}
//      if(candidate.hasDedxDiscrim1() && minDedxDiscriminator1>=0 && candidate.dedxDiscriminator1().dEdx()<minDedxDiscriminator1){return false;}
   }

   if(candidate.hasMuon()){
      if(candidate.muon()->p()     < minMuonP   ){return false;}
      if(candidate.muon()->pt()    < minMuonPt  ){return false;}

//      Need to be implemented using external timing object
//      if(maxMuTimeDtBeta      >=0 && 1.0/candidate.muonTimeDt().inverseBeta()       > maxMuTimeDtBeta      ){return false;}
//      if(minMuTimeDtNdof      >=0 && 1.0/candidate.muonTimeDt().nDof()              < minMuTimeDtNdof      ){return false;}
//      if(maxMuTimeCscBeta     >=0 && 1.0/candidate.muonTimeCsc().inverseBeta()      > maxMuTimeCscBeta     ){return false;}
//      if(minMuTimeCscNdof     >=0 && 1.0/candidate.muonTimeCsc().nDof()             < minMuTimeCscNdof     ){return false;}
//      if(maxMuTimeCombinedBeta>=0 && 1.0/candidate.muonTimeCombined().inverseBeta() > maxMuTimeCombinedBeta){return false;}
//      if(minMuTimeCombinedNdof>=0 && 1.0/candidate.muonTimeCombined().nDof()        < minMuTimeCombinedNdof){return false;}
   }

   if(candidate.hasRpcInfo()  && maxBetaRpc>=0  && candidate.rpc ().beta     > maxBetaRpc ){return false;}

   if(candidate.hasMuon() && candidate.muon()->isStandAloneMuon()) {
     if(candidate.muon()->standAloneMuon()->pt() < minSAMuonPt  ){return false;}
   }

   if(candidate.hasMTMuonRef()){
     if(!candidate.MTMuonRef()->standAloneMuon().isNull()){
       if(candidate.MTMuonRef()->standAloneMuon()->pt() < minMTMuonPt  ){return false;}
     }
   }
//      Need to be implemented using external dE/dx object
//   if(candidate.hasCaloInfo() && maxBetaEcal>=0 && candidate.calo().ecalBeta > maxBetaEcal){return false;}

   return true;
}


bool CandidateSelector::isSelected(HSCParticle& candidate)
{
   if(isTrack   && !candidate.hasTrackRef()){return false;}
   if(isMuon    && !candidate.hasMuonRef() ){return false;}
   if(isMuonSTA && (!candidate.hasMuonRef() || candidate.muonRef()->standAloneMuon().isNull()) ){return false;}
   if(isMuonGB  && (!candidate.hasMuonRef() || candidate.muonRef()->combinedMuon  ().isNull()) ){return false;}
   if(isMuonTK  && (!candidate.hasMuonRef() || candidate.muonRef()->innerTrack    ().isNull()) ){return false;}
   if(isMTMuon  && !candidate.hasMTMuonRef() ){return false;}
   if(isRpc     && !candidate.hasRpcInfo() ){return false;}
   if(isEcal    && !candidate.hasCaloInfo()){return false;}

   if(candidate.hasTrackRef()){
      if(candidate.trackRef()->found() < minTrackHits){return false;}
      if(candidate.trackRef()->p()     < minTrackP   ){return false;}
      if(candidate.trackRef()->pt()    < minTrackPt  ){return false;}

//      Need to be implemented using external dE/dx object
//      if(candidate.hasDedxEstim1()   && minDedxEstimator1>=0     && candidate.dedxEstimator1    ().dEdx()<minDedxEstimator1)    {return false;}
//      if(candidate.hasDedxDiscrim1() && minDedxDiscriminator1>=0 && candidate.dedxDiscriminator1().dEdx()<minDedxDiscriminator1){return false;}
   }

   if(candidate.hasMuonRef()){
      if(candidate.muonRef()->p()     < minMuonP   ){return false;}
      if(candidate.muonRef()->pt()    < minMuonPt  ){return false;}

//      Need to be implemented using external timing object
//      if(maxMuTimeDtBeta      >=0 && 1.0/candidate.muonTimeDt().inverseBeta()       > maxMuTimeDtBeta      ){return false;}
//      if(minMuTimeDtNdof      >=0 && 1.0/candidate.muonTimeDt().nDof()              < minMuTimeDtNdof      ){return false;}
//      if(maxMuTimeCscBeta     >=0 && 1.0/candidate.muonTimeCsc().inverseBeta()      > maxMuTimeCscBeta     ){return false;}
//      if(minMuTimeCscNdof     >=0 && 1.0/candidate.muonTimeCsc().nDof()             < minMuTimeCscNdof     ){return false;}
//      if(maxMuTimeCombinedBeta>=0 && 1.0/candidate.muonTimeCombined().inverseBeta() > maxMuTimeCombinedBeta){return false;}
//      if(minMuTimeCombinedNdof>=0 && 1.0/candidate.muonTimeCombined().nDof()        < minMuTimeCombinedNdof){return false;}
   }

   if(candidate.hasRpcInfo()  && maxBetaRpc>=0  && candidate.rpc ().beta     > maxBetaRpc ){return false;}

   if(candidate.hasMuonRef() && candidate.muonRef()->isStandAloneMuon()) {
     if(candidate.muonRef()->standAloneMuon()->pt() < minSAMuonPt  ){return false;}
   }

   if(candidate.hasMTMuonRef()){
     if(!candidate.MTMuonRef()->standAloneMuon().isNull()){
       if(candidate.MTMuonRef()->standAloneMuon()->pt() < minMTMuonPt  ){return false;}
     }
   }
//      Need to be implemented using external dE/dx object
//   if(candidate.hasCaloInfo() && maxBetaEcal>=0 && candidate.calo().ecalBeta > maxBetaEcal){return false;}

   return true;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void CandidateSelector::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setComment("HSCP builder");
  desc.add<bool>("onlyConsiderTrack",  false);
  desc.add<bool>("onlyConsiderMuon",   false);
  desc.add<bool>("onlyConsiderMuonSTA",false);
  desc.add<bool>("onlyConsiderMuonGB", false);
  desc.add<bool>("onlyConsiderMuonTK", false);
  desc.add<bool>("onlyConsiderMTMuon", false);
  desc.add<bool>("onlyConsiderRpc",    false);
  desc.add<bool>("onlyConsiderEcal",   false);
  //
  desc.add<int>("minTrackHits",    -1);
  desc.add<double>("minTrackP",    -1);
  desc.add<double>("minTrackPt",   -1);

  desc.add<double>("minDedx",      -1);

  desc.add<double>("minMuonP",     -1);
  desc.add<double>("minMuonPt",    -1);
  desc.add<double>("minMTMuonPt",  -1);
  desc.add<double>("minSAMuonPt",  -1);

  desc.add<double>("maxMuTimeDtBeta",  -1);
  desc.add<double>("minMuTimeDtNdof",  -1);
  desc.add<double>("maxMuTimeCscBeta", -1);
  desc.add<double>("minMuTimeCscNdof", -1);
  desc.add<double>("maxMuTimeCombinedBeta", -1);
  desc.add<double>("minMuTimeCombinedNdof", -1);

  desc.add<double>("maxBetaRpc",  -1);
  desc.add<double>("maxBetaEcal", -1);

  descriptions.add("CandidateSelector",desc);
}
