//
// Original Author:  Emery Nibigira @2024

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDFilter.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Framework/interface/ESHandle.h"

#include "AnalysisDataFormats/SUSYBSMObjects/interface/HSCParticle.h"
#include "DataFormats/PatCandidates/interface/IsolatedTrack.h"
#include "DataFormats/PatCandidates/interface/Muon.h"

#include "CommonTools/UtilAlgos/interface/DeltaR.h"
#include "SUSYBSMAnalysis/HSCP/interface/CandidateSelector.h"


class HSCParticleProducer : public edm::one::EDFilter<edm::one::SharedResources> {
  public:
    explicit HSCParticleProducer(const edm::ParameterSet& iConfig)
      :Filter_         (iConfig.getParameter<bool>          ("filter")),
       // the input collections
       trackToken_      {consumes<edm::View<pat::IsolatedTrack>>(iConfig.getParameter<edm::InputTag>("tracks"))},
       trackIsoToken_   {consumes<edm::View<pat::IsolatedTrack>>(iConfig.getParameter<edm::InputTag>("tracksIsolation"))},
       muonsToken_      {consumes<std::vector<pat::Muon>>(iConfig.getParameter<edm::InputTag>("slimmedMuons"))},
       dedxHitInfoToken_{consumes<reco::DeDxHitInfoAss>(iConfig.getParameter<edm::InputTag>("dedxHitInfo"))},
       // the parameters
       minTkP          (iConfig.getParameter<double>  ("minTkP")), 
       maxTkChi2       (iConfig.getParameter<double>  ("maxTkChi2")),
       minTkHits       (iConfig.getParameter<uint32_t>("minTkHits")),
       minMuP          (iConfig.getParameter<double>  ("minMuP")),
       minSAMuPt       (iConfig.getParameter<double>  ("minSAMuPt")),
       minMTMuPt       (iConfig.getParameter<double>  ("minMTMuPt")),
       minDR           (iConfig.getParameter<double>  ("minDR")),
       minMTDR         (iConfig.getParameter<double>  ("minMTDR")),
       maxInvPtDiff    (iConfig.getParameter<double>  ("maxInvPtDiff"))
    {
      // Load all the selections
      std::vector<edm::ParameterSet> SelectionParameters = iConfig.getParameter<std::vector<edm::ParameterSet> >("SelectionParameters");
      for(unsigned int i=0;i<SelectionParameters.size();i++){
        Selectors.push_back(new CandidateSelector(SelectionParameters[i]) );
      }
      produces<susybsm::HSCParticleCollection>();
    }

    static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

    ~HSCParticleProducer() override {}

  private:
    virtual bool filter(edm::Event&, const edm::EventSetup&);

    std::vector<susybsm::HSCParticle> getHSCPSeedCollection(edm::Handle<edm::View<pat::IsolatedTrack>>& trackCollectionHandle,  
                                                           edm::Handle<reco::DeDxHitInfoAss> dedxHitInfoHandle,
                                                           edm::Handle<std::vector<pat::Muon>>& muonCollectionHandle);
                                                

    bool isGoodTrack(const pat::PackedCandidateRef track);

    // ----------member data ---------------------------
    bool          Filter_;

    edm::EDGetTokenT<edm::View<pat::IsolatedTrack>> trackToken_;
    edm::EDGetTokenT<edm::View<pat::IsolatedTrack>> trackIsoToken_;
    edm::EDGetTokenT<std::vector<pat::Muon>> muonsToken_;
    edm::EDGetTokenT<reco::DeDxHitInfoAss> dedxHitInfoToken_;

    bool         useBetaFromTk;
    bool         useBetaFromMuon;
    bool         useBetaFromRpc;
    bool         useBetaFromEcal;

    float        minTkP;
    float        maxTkChi2;
    uint32_t     minTkHits;
    float        minMuP;
    float        minSAMuPt;
    float        minMTMuPt;
    float        minDR;
    float        minMTDR;
    float        maxInvPtDiff;

    std::vector<CandidateSelector*> Selectors;
};


// ------------ method called to produce the data  ------------
bool HSCParticleProducer::filter(edm::Event& iEvent, const edm::EventSetup& iSetup) {

  using namespace edm;
  using namespace std;

  // information from the muons
  //edm::Handle<pat::Muon> muonCollectionHandle = iEvent.getHandle(muonsToken_);
  auto muonCollectionHandle = iEvent.getHandle(muonsToken_);

  // information from the tracks
  auto trackCollectionHandle = iEvent.getHandle(trackToken_);

  // information from the tracks iso
  auto trackIsoCollectionHandle = iEvent.getHandle(trackIsoToken_);

  //const edm::Handle<reco::DeDxHitInfoAss> dedxHitInfoHandle = iEvent.getHandle(dedxHitInfoToken_);
  auto dedxHitInfoHandle = iEvent.getHandle(dedxHitInfoToken_);

  // creates the output collection
  //susybsm::HSCParticleCollection* hscp = new susybsm::HSCParticleCollection;
  //std::unique_ptr<susybsm::HSCParticleCollection> result(hscp);
  ////auto hscp = std::make_unique<std::vector<susybsm::HSCParticle>>();
  auto hscp = std::make_unique<susybsm::HSCParticleCollection>();

  // Fill the output collection with HSCP Candidate (the candiate only contains ref to muon AND/OR track object)
  *hscp = getHSCPSeedCollection(trackCollectionHandle, dedxHitInfoHandle, muonCollectionHandle);

  // cleanup the collection based on the input selection
  for(int i=0;i<(int)hscp->size();i++) {
     susybsm::HSCParticleCollection::iterator hscpcandidate = hscp->begin() + i;
     bool decision = false;
     for(unsigned int s=0;s<Selectors.size();s++){decision |= Selectors[s]->isSelectedFromMiniAOD(*hscpcandidate);}
     if(!decision){
        hscp->erase(hscpcandidate);
        //if(useBetaFromEcal)caloInfoColl->erase(caloInfoColl->begin() + i);
        i--;
     }
  }
  
  bool filterResult = !Filter_ || (Filter_ && hscp->size()>=1);

  // output result
  edm::OrphanHandle<std::vector<susybsm::HSCParticle>> putHandle = iEvent.put(std::move(hscp));

  return filterResult;
}


std::vector<susybsm::HSCParticle> HSCParticleProducer::getHSCPSeedCollection(edm::Handle<edm::View<pat::IsolatedTrack>>& trackCollectionHandle,  
                                                                    edm::Handle<reco::DeDxHitInfoAss> dedxHitInfoHandle,
                                                                    edm::Handle<std::vector<pat::Muon>>& muonCollectionHandle)
{
  std::vector<susybsm::HSCParticle> HSCPCollection;

  // Store a local vector of track ref (that can be modified if matching)
  std::vector<pat::IsolatedTrack> tracks;
  std::vector<const reco::DeDxHitInfo*> dedxHitInfo;

  for(unsigned int it = 0; it < trackCollectionHandle->size(); it++){
    auto isotrack = trackCollectionHandle->ptrAt(it);
    const pat::PackedCandidateRef track = isotrack->packedCandRef();

    if (track.isNull() || !track.isAvailable()) continue; // resolve a null or invalid reference
    
    if (!isGoodTrack(track)) continue;

    //If track is from muon always keep it
    bool isMuon=false;
    for (size_t im = 0; im < muonCollectionHandle->size(); ++im) {
      const pat::MuonRef muon(muonCollectionHandle, im);
      
      if (muon->innerTrack().isNull()) continue;
      if( fabs( (1.0/muon->innerTrack()->pt())-(1.0/track->pt())) > maxInvPtDiff) continue;
      //float dR = deltaR(muon->innerTrack()->momentum(), track->p());
      float dR = deltaR(muon->innerTrack()->eta(), muon->innerTrack()->phi(), track->eta(), track->phi());
      if(dR <= minDR) isMuon=true;
    }

    if(!isMuon) continue;
    tracks.push_back( *isotrack );
    const reco::DeDxHitInfo* dedxInfo = (*dedxHitInfoHandle)[isotrack].get();
    dedxHitInfo.push_back( dedxInfo );
  }


  /*for (size_t im = 0; im < muonCollectionHandle->size(); ++im) {
    const pat::MuonRef muon(muonCollectionHandle, im);
  
    double SApt=-1;
    if(muon->isStandAloneMuon()) SApt=muon->standAloneMuon()->pt();
    if(muon->p()<minMuP && SApt<minSAMuPt)continue;

    // Check if the inner track match any track in order to create a Muon+Track HSCP Candidate
    // Matching is needed because input track collection and muon inner track may lightly differs due to track refit
    float dRMin=1000; int found = -1;
    for(unsigned int it = 0; it<tracks.size(); it++) {
      //const pat::PackedCandidateRef track  = tracks[it];
      auto isotrack = tracks[it];
      const pat::PackedCandidateRef track = isotrack.packedCandRef();

      //if( fabs( (1.0/muon_pt)-(1.0/track->pt())) > maxInvPtDiff) continue;
      //float dR = deltaR(muon_p, track->p());
      //foundMatch
      bool foundMatch = (muon->innerTrack().isNonnull()) ? ( fabs( (1.0/muon->innerTrack()->pt())-(1.0/track->pt())) <= maxInvPtDiff) 
                                                          : ( fabs( (1.0/muon->pt())-(1.0/track->pt())) <= maxInvPtDiff);
      if(!foundMatch) continue; 
      float dR = (muon->innerTrack().isNonnull()) ? deltaR(muon->innerTrack()->eta(), muon->innerTrack()->phi(), track->eta(), track->phi())
                                                  : deltaR(muon->eta(), muon->phi(),track->eta(), track->phi());;
      if(dR <= minDR && dR < dRMin){ dRMin=dR; found = it;}
    }

    susybsm::HSCParticle candidate;
    candidate.setMuon(muon);
    if(found>=0){
      candidate.setTrack(tracks[found]);  candidate.setDeDxHitInfo(dedxHitInfo[found]);
      tracks.erase(tracks.begin()+found); dedxHitInfo.erase(dedxHitInfo.begin()+found);
    }
    HSCPCollection.push_back(candidate);
  }*/

  //EMERY-1//// Loop on muons with inner track ref and create Muon HSCP Candidate
  for (size_t im = 0; im < muonCollectionHandle->size(); ++im) {
    const pat::MuonRef muon(muonCollectionHandle, im);
  
    double SApt=-1;
    if(muon->isStandAloneMuon()) SApt=muon->standAloneMuon()->pt();
    if(muon->p()<minMuP && SApt<minSAMuPt)continue;
    if (muon->innerTrack().isNull()) continue;

    // Check if the inner track match any track in order to create a Muon+Track HSCP Candidate
    // Matching is needed because input track collection and muon inner track may lightly differs due to track refit
    float dRMin=1000; int found = -1;
    for(unsigned int it = 0; it<tracks.size(); it++) {
      //const pat::PackedCandidateRef track  = tracks[it];
      auto isotrack = tracks[it];
      const pat::PackedCandidateRef track = isotrack.packedCandRef();

      //if( fabs( (1.0/muon_pt)-(1.0/track->pt())) > maxInvPtDiff) continue;
      //float dR = deltaR(muon_p, track->p());
      //foundMatch
      bool foundMatch = ( fabs( (1.0/muon->innerTrack()->pt())-(1.0/track->pt())) <= maxInvPtDiff );
      if(!foundMatch) continue; 
      float dR = deltaR(muon->innerTrack()->eta(), muon->innerTrack()->phi(), track->eta(), track->phi());
      if(dR <= minDR && dR < dRMin){ dRMin=dR; found = it;}
    }

    susybsm::HSCParticle candidate;
    candidate.setMuon(muon);
    if(found>=0){
      candidate.setTrack(tracks[found]);  candidate.setDeDxHitInfo(dedxHitInfo[found]);
      tracks.erase(tracks.begin()+found); dedxHitInfo.erase(dedxHitInfo.begin()+found);
    }
    HSCPCollection.push_back(candidate);
  }

  //EMERY-2//
  for (size_t im = 0; im < muonCollectionHandle->size(); ++im) {
    const pat::MuonRef muon(muonCollectionHandle, im);
  
    double SApt=-1;
    if(muon->isStandAloneMuon()) SApt=muon->standAloneMuon()->pt();
    if(muon->p()<minMuP && SApt<minSAMuPt)continue;

    // Check if the inner track match any track in order to create a Muon+Track HSCP Candidate
    // Matching is needed because input track collection and muon inner track may lightly differs due to track refit
    float dRMin=1000; int found = -1;
    for(unsigned int it = 0; it<tracks.size(); it++) {
      //const pat::PackedCandidateRef track  = tracks[it];
      auto isotrack = tracks[it];
      const pat::PackedCandidateRef track = isotrack.packedCandRef();

      //if( fabs( (1.0/muon_pt)-(1.0/track->pt())) > maxInvPtDiff) continue;
      //float dR = deltaR(muon_p, track->p());
      //foundMatch
      bool foundMatch = (muon->innerTrack().isNonnull()) ? ( fabs( (1.0/muon->innerTrack()->pt())-(1.0/track->pt())) <= maxInvPtDiff) 
                                                          : ( fabs( (1.0/muon->pt())-(1.0/track->pt())) <= maxInvPtDiff);
      if(!foundMatch) continue; 
      float dR = (muon->innerTrack().isNonnull()) ? deltaR(muon->innerTrack()->eta(), muon->innerTrack()->phi(), track->eta(), track->phi())
                                                  : deltaR(muon->eta(), muon->phi(),track->eta(), track->phi());;
      if(dR <= minDR && dR < dRMin){ dRMin=dR; found = it;}
    }

    susybsm::HSCParticle candidate;
    candidate.setMuon(muon);
    if(found>=0){
      candidate.setTrack(tracks[found]);  candidate.setDeDxHitInfo(dedxHitInfo[found]);
      tracks.erase(tracks.begin()+found); dedxHitInfo.erase(dedxHitInfo.begin()+found);
    }
    HSCPCollection.push_back(candidate);
  }

  // Loop on tracks not matching muon and create Track HSCP Candidate
  for(unsigned int it=0; it<tracks.size(); it++){
    susybsm::HSCParticle candidate;
    candidate.setTrack(tracks[it]); candidate.setDeDxHitInfo(dedxHitInfo[it]);
    HSCPCollection.push_back(candidate); 
  }

   return HSCPCollection;
}

bool HSCParticleProducer::isGoodTrack(const pat::PackedCandidateRef track){
  if (!track->hasTrackDetails()) return false; // ignore candidates without track
  if (track->charge() == 0) return false; // ignore neutral candidates
  if (track->p()<minTkP) return false;
  if (track->pseudoTrack().normalizedChi2()>maxTkChi2) return false;
  if (track->pseudoTrack().found()<minTkHits) return false;
  return true;
}


// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void HSCParticleProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setComment("HSCP builder");
  desc.add<bool>("filter", false);
  // TAG OF THE REQUIRED INPUT COLLECTION
  desc.add("tracks",           edm::InputTag("isolatedTracks"));
  desc.add("tracksIsolation",  edm::InputTag("isolatedTracks"));
  desc.add("slimmedMuons",     edm::InputTag("slimmedMuons"));
  desc.add("dedxHitInfo",      edm::InputTag("isolatedTracks"));
  // TRACK SELECTION FOR THE HSCP SEED
  desc.add<double>("minTkP",       30);
  desc.add<double>("maxTkChi2",    5);
  desc.add<uint32_t>("minTkHits",    9);
  desc.add<double>("minMuP",       30);
  desc.add<double>("minSAMuPt",    70);
  desc.add<double>("minMTMuPt",    70);
  // MUON/TRACK MATCHING THRESHOLDS (ONLY IF NO MUON INNER TRACK)
  desc.add<double>("minDR",        0.1);
  desc.add<double>("minMTDR",      0.3);
  desc.add<double>("maxInvPtDiff", 0.005);

  std::vector<edm::ParameterSet> HSCPSelections;
  //
  edm::ParameterSetDescription cand;           edm::ParameterSet HSCPSelection;
  cand.add<bool>("onlyConsiderTrack",  false); HSCPSelection.addParameter<bool>("onlyConsiderTrack",  false);
  cand.add<bool>("onlyConsiderMuon",   false); HSCPSelection.addParameter<bool>("onlyConsiderMuon",   false);
  cand.add<bool>("onlyConsiderMuonSTA",false); HSCPSelection.addParameter<bool>("onlyConsiderMuonSTA",false);
  cand.add<bool>("onlyConsiderMuonGB", false); HSCPSelection.addParameter<bool>("onlyConsiderMuonGB", false);
  cand.add<bool>("onlyConsiderMuonTK", false); HSCPSelection.addParameter<bool>("onlyConsiderMuonTK", false);
  cand.add<bool>("onlyConsiderRpc",    false); HSCPSelection.addParameter<bool>("onlyConsiderRpc",    false);
  cand.add<bool>("onlyConsiderEcal",   false); HSCPSelection.addParameter<bool>("onlyConsiderEcal",   false);
  //
  cand.add<int>("minTrackHits",    2); HSCPSelection.addParameter<int>("minTrackHits",    -1);
  cand.add<double>("minTrackP", 45.0); HSCPSelection.addParameter<double>("minTrackP",    -1);
  cand.add<double>("minTrackPt", 5.0); HSCPSelection.addParameter<double>("minTrackPt",   -1);

  cand.add<double>("minDedx",      -1); HSCPSelection.addParameter<double>("minDedx",      -1);

  cand.add<double>("minMuonP",     -1); HSCPSelection.addParameter<double>("minMuonP",     -1);
  cand.add<double>("minMuonPt",    -1); HSCPSelection.addParameter<double>("minMuonPt",    -1);
  cand.add<double>("minSAMuonPt",  -1); HSCPSelection.addParameter<double>("minSAMuonPt",  -1);

  cand.add<double>("maxMuTimeDtBeta",  -1); HSCPSelection.addParameter<double>("maxMuTimeDtBeta",  -1);
  cand.add<double>("minMuTimeDtNdof",  -1); HSCPSelection.addParameter<double>("minMuTimeDtNdof",  -1);
  cand.add<double>("maxMuTimeCscBeta", -1); HSCPSelection.addParameter<double>("maxMuTimeCscBeta", -1);
  cand.add<double>("minMuTimeCscNdof", -1); HSCPSelection.addParameter<double>("minMuTimeCscNdof", -1);
  cand.add<double>("maxMuTimeCombinedBeta", -1); HSCPSelection.addParameter<double>("maxMuTimeCombinedBeta", -1);
  cand.add<double>("minMuTimeCombinedNdof", -1); HSCPSelection.addParameter<double>("minMuTimeCombinedNdof", -1);

  cand.add<double>("maxBetaRpc",  -1); HSCPSelection.addParameter<double>("maxBetaRpc",  -1);
  cand.add<double>("maxBetaEcal", -1); HSCPSelection.addParameter<double>("maxBetaEcal", -1);
  //
  HSCPSelections.push_back(HSCPSelection);
  //HSCPSelectionHighdEdx//onlyConsiderTrack       = cms.bool(True)//minDedxEstimator1       = cms.double(3.5)
  desc.addVPSet("SelectionParameters", cand, HSCPSelections);
  //desc.add<std::vector<edm::ParameterSet>>("SelectionParameters", HSCPSelections);

  descriptions.add("HSCParticleProducer",desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HSCParticleProducer);