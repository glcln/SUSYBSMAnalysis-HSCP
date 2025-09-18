#ifndef ANALYZER_H
#define ANALYZER_H

// system include files
#include <map>

#include "TFile.h"
#include "TTree.h"
#include "TH1.h"
#include "TLorentzVector.h"

// FWCore include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/Registry.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "DataFormats/TrackReco/interface/HitPattern.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"
#include "DataFormats/TrackReco/interface/DeDxHitInfo.h"

#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/Common/interface/Ref.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "math.h"

#include "DataFormats/MuonReco/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonFwd.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"

#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/VertexReco/interface/VertexFwd.h"

//#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
//#include "DataFormats/HepMCCandidate/interface/GenParticleFwd.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"

#include "DataFormats/Math/interface/deltaR.h"

#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "FWCore/Common/interface/TriggerResultsByName.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"

#include "DataFormats/L1Trigger/interface/EtSum.h"
#include "DataFormats/L1Trigger/interface/EtSumHelper.h"
#include "DataFormats/METReco/interface/PFMET.h"
//#include "DataFormats/METReco/interface/PFMETFwd.h"
#include "DataFormats/METReco/interface/CaloMET.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "AnalysisDataFormats/SUSYBSMObjects/interface/HSCParticle.h"

/// MINI-AOD
#include "DataFormats/PatCandidates/interface/IsolatedTrack.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

///TRACKER Low Level
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include "Geometry/Records/interface/TrackerTopologyRcd.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "RecoLocalTracker/Records/interface/TkPixelCPERecord.h"
#include "RecoLocalTracker/ClusterParameterEstimator/interface/PixelClusterParameterEstimator.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"

///Timing from Muon detectors
#include "DataFormats/MuonReco/interface/MuonTimeExtra.h"
#include "DataFormats/MuonReco/interface/MuonTimeExtraMap.h"

////////
#define FWCORE
#include "AnalysisDataFormats/SUSYBSMObjects/interface/HSCParticle.h"
#include "SUSYBSMAnalysis/HSCP/interface/TreeManager.h"
#include "SUSYBSMAnalysis/HSCP/interface/HSCPVertexSelector.h"
#include "SUSYBSMAnalysis/Analyzer/interface/TrigToolsFuncs.h"
#include "SUSYBSMAnalysis/HSCP/interface/HSCPDeDxTool.h"
#include "SUSYBSMAnalysis/HSCP/interface/HelperFunctions.h"
///////

namespace HSCPType {
  enum Type { globalMuon, trackerMuon, matchedStandAloneMuon, standAloneMuon, innerTrack, unknown };
}


using namespace edm;
using namespace std;

static constexpr const char* const MOD = "Analyzer";

class Analyzer : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit Analyzer(const edm::ParameterSet &);
  ~Analyzer() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  //void beginRun(edm::Run const &, edm::EventSetup const &) override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  int type(susybsm::HSCParticle hscp);

  // ----------member data ---------------------------
  // HSCP
  edm::EDGetTokenT<vector<susybsm::HSCParticle>> hscpToken_;

  edm::EDGetTokenT<edm::TriggerResults> triggerToken_;
  edm::EDGetTokenT<pat::TriggerObjectStandAloneCollection> triggerObjects_;
  edm::EDGetTokenT<trigger::TriggerEvent> trigEventToken_;

  edm::EDGetTokenT<vector<reco::Vertex>> vertexToken_;

  edm::EDGetTokenT<edm::View<pat::IsolatedTrack>> trackToken_;
  edm::EDGetTokenT<edm::View<pat::IsolatedTrack>> trackIsoToken_;

  edm::EDGetTokenT<std::vector<pat::Muon> > muonToken_;

  // edm::EDGetTokenT<reco::MuonTimeExtraMap> muonTimeToken_;  // for reading inverse beta
  // edm::EDGetTokenT<reco::MuonTimeExtraMap> muonDtTimeToken_;
  // edm::EDGetTokenT<reco::MuonTimeExtraMap> muonCscTimeToken_;

  edm::EDGetTokenT<std::vector<pat::PackedGenParticle>> genParticleToken_;
  edm::EDGetTokenT<GenEventInfoProduct>  genEventToken_;
  edm::EDGetTokenT<reco::DeDxHitInfoAss> dedxToken_;

  edm::EDGetTokenT<pat::PackedCandidateCollection> pfCandToken_;

  edm::ESGetToken<TrackerTopology, TrackerTopologyRcd> trackerTopoToken_;
  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> geometryToken_;
  std::string pixelCPE_;// = "PixelCPETemplateReco";
  edm::ESGetToken<PixelClusterParameterEstimator, TkPixelCPERecord> trackerPixelCPEToken_;

  TreeManager  *treeManager_;
  map<std::string, std::any> vars_;
  //Cutflows
  TH1F* EventCutFlow_;
  std::vector<std::string> EventCutFlowLabels = {"AllEvents",">=1Vtx","PassedHLT","MatchedMu",">=1HSCP",">=1track",">=1muon"};
  std::vector<std::string> HSCPCutFlowLabels = {"All",">=1Vtx","Technical","Trigger","p_{T}","#eta","N_{no-L1 pixel hits}","f_{valid/all hits}",
                                                "N_{dEdx hits}","HighPurity","#chi^{2} / N_{dof}","d_{z}","d_{xy}","MiniRelIsoAll","MiniRelTkIso",
                                                "E/p","#sigma_{p_{T}} / p_{T}^{2}","F_{i}"};
  const int N_CUTS = EventCutFlowLabels.size();
  const int NHSCP_CUTS = HSCPCutFlowLabels.size();

  double mcEventWeight_;

  std::vector<std::string> triggerPaths_;
  std::vector<std::string> triggerBranchNames_;
  bool triggerFilter_;

  edm::EDGetTokenT<l1t::EtSumBxCollection> l1TriggerEtSumToken_;
  edm::EDGetTokenT<std::vector<reco::PFMET>> pfMETToken_;
  edm::EDGetTokenT<std::vector<reco::CaloMET>> caloMETToken_;
  edm::EDGetTokenT</*std::vector<pat::MET>*/pat::METCollection> metToken_;
  //
  edm::EDGetTokenT<edm::TriggerResults> noiseCleaningFilterToken_;

  bool tapeRecallOnly_;

  std::string filterName_;

  int debug_;
  bool saveDeDxHitInfo_;
  /*float factorChargeToE_[2] = {3.61e-06, 3.61e-06 * 265};
  float dEdxSF[2] = {1.0, 1.035};
  float dEdxK = 2.3;
  float dEdxC = 3.17;*/
  float dEdxSF_0_, dEdxSF_1_;
  float dEdxSF[2] = {dEdxSF_0_, dEdxSF_1_};
  float dEdxK_;
  float dEdxC_;
  bool useClusterCleaning = true;
  bool useTemplateLayer_ = false;

  std::string dEdxTemplate_;
  TH3F* dEdxTemplates = nullptr;
  std::vector<TH3F*> dEdxTemplatesPU;
  bool puTreatment_, createGiTemplates_, createAndExitGitemplates_;
  vector<int> PuBins_ = std::vector<int>{0,20,25,30,35,200};
  int NbPuBins_ = PuBins_.size() - 1;

  //map<std::string, std::any> vars_vec_;

  bool addStripClusterInfo_;

  std::string year_;

  double trackPtMin_;
  double trackEtaMin_;
  double trackEtaMax_;

  //==========================
  //    Event information
  //==========================
  Bool_t isData_;
  Int_t runNumber;
  ULong64_t eventNumber;
  UInt_t lumiBlock;
  Bool_t triggerFired;

  //==========================
  //    Trigger
  //==========================
  const static int max_trig = 1000;
  bool HLT_trig[max_trig];  
};

#endif /* ANALYZER_H */