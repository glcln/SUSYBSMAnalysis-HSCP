////////
#include "SUSYBSMAnalysis/Analyzer/plugins/Analyzer.h"
///////


Analyzer::Analyzer(const edm::ParameterSet &iConfig) :
    hscpToken_(consumes<vector<susybsm::HSCParticle>>(iConfig.getParameter<edm::InputTag>("HscpCollection"))),
    triggerToken_(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("TriggerCollection"))),
    triggerObjects_(consumes<pat::TriggerObjectStandAloneCollection>(iConfig.getParameter<edm::InputTag>("TriggerObjects"))),
    trigEventToken_(consumes<trigger::TriggerEvent>(iConfig.getParameter<edm::InputTag>("TriggerSummary"))),
    vertexToken_(consumes<vector<reco::Vertex>>(iConfig.getParameter<edm::InputTag>("OfflinePVCollection"))),
    trackToken_(consumes<edm::View<pat::IsolatedTrack>>(iConfig.getParameter<edm::InputTag>("TrackCollection"))),
    trackIsoToken_(consumes<edm::View<pat::IsolatedTrack>>(iConfig.getParameter<edm::InputTag>("TrackIsoCollection"))),
    muonToken_(consumes<std::vector<pat::Muon>>(iConfig.getParameter<edm::InputTag>("MuonCollection"))),
    // muonTimeToken_(consumes<reco::MuonTimeExtraMap>(iConfig.getParameter<edm::InputTag>("MuonTimeCollection"))),
    // muonDtTimeToken_(consumes<reco::MuonTimeExtraMap>(iConfig.getParameter<edm::InputTag>("MuonDtTimeCollection"))),
    // muonCscTimeToken_(consumes<reco::MuonTimeExtraMap>(iConfig.getParameter<edm::InputTag>("MuonCscTimeCollection"))),
    genParticleToken_(consumes<std::vector<pat::PackedGenParticle>>(iConfig.getParameter<edm::InputTag>("GenPartCollection"))),
    genEventToken_(consumes<GenEventInfoProduct>(iConfig.getParameter<edm::InputTag>("GenCollection"))),
    dedxToken_(consumes<reco::DeDxHitInfoAss>(iConfig.getParameter<edm::InputTag>("DeDxCollection"))),
    pfCandToken_(consumes<pat::PackedCandidateCollection>(iConfig.getParameter<edm::InputTag>("PfCand"))),
    //
    trackerTopoToken_(esConsumes<TrackerTopology, TrackerTopologyRcd>()),
    geometryToken_(esConsumes<TrackerGeometry, TrackerDigiGeometryRecord>()),
    pixelCPE_(iConfig.getParameter<std::string>("PixelCPE")),
    trackerPixelCPEToken_(esConsumes<PixelClusterParameterEstimator, TkPixelCPERecord>(edm::ESInputTag("", pixelCPE_))),
    //
    triggerPaths_(iConfig.getUntrackedParameter<std::vector<std::string>>("TriggerPaths")),
    triggerFilter_(iConfig.getUntrackedParameter<bool>("TriggerFilter")),
    //
    l1TriggerEtSumToken_(consumes<l1t::EtSumBxCollection>(iConfig.getParameter<edm::InputTag>("l1TriggerEtSum"))),
    metToken_(consumes<pat::METCollection>(iConfig.getParameter<edm::InputTag>("SlimmedMET"))),
    //caloMETToken_(consumes<std::vector<reco::CaloMET>>(iConfig.getParameter<edm::InputTag>("CaloMET"))),
    //
    noiseCleaningFilterToken_(consumes<edm::TriggerResults>(iConfig.getParameter<edm::InputTag>("NoiseFilters"))),
    //
    tapeRecallOnly_(iConfig.getUntrackedParameter<bool>("TapeRecallOnly")),
    filterName_(iConfig.getParameter<std::string>("FilterName")),
    dEdxSF_0_(iConfig.getUntrackedParameter<double>("DeDxSF_0")),
    dEdxSF_1_(iConfig.getUntrackedParameter<double>("DeDxSF_1")),
    dEdxK_(iConfig.getUntrackedParameter<double>("DeDxK")),
    dEdxC_(iConfig.getUntrackedParameter<double>("DeDxC")),
    dEdxTemplate_(iConfig.getUntrackedParameter<string>("DeDxTemplate")),
    //
    addStripClusterInfo_(iConfig.getUntrackedParameter<bool>("AddStripClusterInfo"))
{}


Analyzer::~Analyzer() = default;


void Analyzer::beginJob(){
  // if the only purpose is to trick CRAB to do a TAPERECALL
  if (tapeRecallOnly_) return;

  year_ = "";

  edm::Service<TFileService> fs;

  // Some histograms
  EventCutFlow_ = fs->make<TH1F>("EventCutFlow" , "EventCutFlow" , N_CUTS , -0.5 , float(N_CUTS)-0.5 );

  treeManager_ = new TreeManager();
  treeManager_->initialize("Events", fs);

  // Event information
  vars_["isData"]           = bool {false};
  vars_["run"]              = uint32_t {0};
  vars_["luminosityBlock"]  = uint32_t {0};
  vars_["event"]            = 0ULL;
  vars_["bunchCrossing"]    = uint32_t {0};


  // Trigger
  triggerBranchNames_.clear();
  for(unsigned int i = 0; i < triggerPaths_.size(); i++) {
    TString branch_name = triggerPaths_[i];
    if (branch_name.EndsWith("_v")){
      branch_name = branch_name.Strip(TString::kTrailing, 'v');
      branch_name = branch_name.Strip(TString::kTrailing, '_');
    }
    vars_[branch_name.Data()] = bool {false};
    triggerBranchNames_.push_back(branch_name.Data());
  }
  vars_["HLT_FilterOR"] = bool {false};

  // L1/HLT MET variables
  vars_["L1MHT"]       = double {0};
  vars_["L1MHT_phi"]   = double {0};
  vars_["L1ETSum"]     = double {0};
  vars_["L1HTSum"]     = double {0};
  vars_["L1MET"]       = double {0};
  vars_["L1MET_phi"]   = double {0};
  vars_["L1METHF"]     = double {0};
  vars_["L1METHF_phi"] = double {0};
  vars_["RecoCaloMET"]     = double {0};
  vars_["RecoCaloMET_phi"] = double {0};
  vars_["RecoCaloMET_sigf"]= double {0};
  vars_["RecoPFMET"]       = double {0};
  vars_["RecoPFMET_phi"]   = double {0};
  vars_["RecoPFMET_sigf"]  = double {0};
  //
  vars_["HLTCaloMET"]           = float {0};
  vars_["HLTCaloMET_phi"]       = float {0};
  vars_["HLTCaloMET_sigf"]      = float {0};
  vars_["HLTCaloMETClean"]      = float {0};
  vars_["HLTCaloMETClean_phi"]  = float {0};
  vars_["HLTCaloMETClean_sigf"] = float {0};
  vars_["HLTCaloMHT"]           = float {0};
  vars_["HLTCaloMHT_phi"]       = float {0};
  vars_["HLTCaloMHT_sigf"]      = float {0};
  vars_["HLTPFMHT"]             = float {0};
  vars_["HLTPFMHT_phi"]         = float {0};
  vars_["HLTPFMHT_sigf"]        = float {0};
  vars_["HLTPFMET"]             = float {0};
  vars_["HLTPFMET_phi"]         = float {0};
  vars_["HLTPFMET_sigf"]        = float {0};

  // Noise Filters
  vars_["Flag_globalSuperTightHalo2016Filter"]     = bool {false};
  vars_["Flag_HBHENoiseFilter"]                    = bool {false};
  vars_["Flag_HBHENoiseIsoFilter"]                 = bool {false};
  vars_["Flag_EcalDeadCellTriggerPrimitiveFilter"] = bool {false};
  vars_["Flag_BadPFMuonFilter"]                    = bool {false};
  vars_["Flag_BadPFMuonDzFilter"]                  = bool {false};
  vars_["Flag_hfNoisyHitsFilter"]                  = bool {false};
  vars_["Flag_eeBadScFilter"]                      = bool {false};
  vars_["Flag_ecalBadCalibFilter"]                 = bool {false};
  vars_["Flag_allMETFilters"]                      = bool {false};
  vars_["Flag_METFilters"]                         = bool {false};

  //Event weights
  vars_["weight_generator"]  = double {0};
  vars_["weight_generatorBinningValues"]  = double {0};

  // Vertices
  vars_["PV_npvsGood"]      = uint32_t {0};
  vars_["PV_ndof"]          = vector<double> {};
  vars_["PV_x"]             = vector<double> {};
  vars_["PV_y"]             = vector<double> {};
  vars_["PV_z"]             = vector<double> {};
  vars_["PV_rho"]           = vector<double> {};

  // HSCP Candidates
  vars_["HSCP_n"]          = uint32_t {0};
  vars_["HSCP_type"]       = vector<int> {};
  vars_["HSCP_hasTrack"]   = vector<bool> {};
  vars_["HSCP_hasMuon"]    = vector<bool> {};
  vars_["HSCP_hasDeDx"]    = vector<bool> {};
  // Track
  vars_["nIsoTrack"]        = uint32_t {0};
  vars_["IsoTrack_charge"]     = vector<int> {};
  vars_["IsoTrack_p"]          = vector<double> {};
  vars_["IsoTrack_px"]         = vector<double> {};
  vars_["IsoTrack_py"]         = vector<double> {};
  vars_["IsoTrack_pz"]         = vector<double> {};
  vars_["IsoTrack_pt"]         = vector<double> {};
  vars_["IsoTrack_ptError"]    = vector<double> {};
  vars_["IsoTrack_ptErrOverPt"]  = vector<double> {};
  vars_["IsoTrack_ptErrOverPt2"] = vector<double> {};
  vars_["IsoTrack_eta"]        = vector<double> {};
  vars_["IsoTrack_phi"]        = vector<double> {};
  vars_["IsoTrack_dz"]         = vector<float> {};
  vars_["IsoTrack_dzError"]    = vector<float> {};
  vars_["IsoTrack_dxy"]         = vector<float> {};
  vars_["IsoTrack_dxyError"]    = vector<float> {};
  vars_["IsoTrack_normChi2"]   = vector<double> {};
  vars_["IsoTrack_isHighPurityTrack"]   = vector<bool> {};
  vars_["IsoTrack_fractionOfValidHits"] = vector<double>{};
  vars_["IsoTrack_numberOfAllHits"]   = vector<unsigned short>{};
  vars_["IsoTrack_numberOfValidHits"] = vector<unsigned short>{};
  vars_["IsoTrack_numberOfValidPixelHits"] = vector<int>{};
  vars_["IsoTrack_numberOfTrackerLayers"]  = vector<int>{};
  vars_["IsoTrack_isPFcand"]        = vector<bool> {};
  vars_["IsoTrack_pfMiniRelIsoChg"] = vector<float>{};
  vars_["IsoTrack_pfMiniRelIsoAll"] = vector<float>{};
  vars_["IsoTrack_pfEnergyOverP"]   = vector<double>{};
  vars_["IsoTrack_pfEcalEnergy"]    = vector<float>{};
  vars_["IsoTrack_pfHcalEnergy"]    = vector<float>{};
  vars_["IsoTrack_miniRelIso"]      = vector<float>{};
  // DeDx
  vars_["DeDx_Ih"]           = vector<float> {};
  vars_["DeDx_IhNOM"]        = vector<uint32_t> {};
  vars_["DeDx_IhNoL1"]       = vector<float> {};
  vars_["DeDx_IhNoL1NOM"]    = vector<uint32_t> {};
  vars_["DeDx_IhPixel"]      = vector<float> {};
  vars_["DeDx_IhPixelNoL1"]  = vector<float> {};
  vars_["DeDx_IhStrip"]      = vector<float> {};
  vars_["DeDx_Gi"]           = vector<float> {};
  vars_["DeDx_GiStrip"]      = vector<float> {};
  vars_["DeDx_FiPixel"]      = vector<float> {};
  vars_["DeDx_FiPixelNoL1"]  = vector<float> {};
  vars_["DeDx_PixelNoL1NOM"] = vector<uint32_t> {};
  vars_["DeDx_NoL1NOM"]      = vector<uint32_t> {};
  // DeDxHitInfo
  if (addStripClusterInfo_){
    vars_["cluster_DetId"]             = vector<vector<uint32_t>> {};
    vars_["cluster_Charge"]            = vector<vector<float>> {};
    vars_["cluster_SaturatingStrips"]  = vector<vector<uint32_t>> {};
  }
  //Muon
  vars_["nMuon"]            = uint32_t {0};
  vars_["Muon_pt"]          = vector<double> {};
  vars_["Muon_eta"]         = vector<double> {};
  vars_["Muon_phi"]         = vector<double> {};
  vars_["Muon_isPFcand"]    = vector<bool> {};//muon.isPFMuon();
  vars_["Muon_isStandAlone"]= vector<bool> {};
  vars_["Muon_isTracker"]   = vector<bool> {};
  vars_["Muon_isGlobal"]    = vector<bool> {};
  vars_["Muon_pfIso03_sumPU"]       = vector<float> {};//muon.pfIsolationR03().sumPUPt
  vars_["Muon_combRelIsoPF03dBeta"] = vector<float> {};
  vars_["Muon_pfIso04_charged"]     = vector<float> {};//muon.pfIsolationR04().sumChargedHadronPt;
  vars_["Muon_pfIso04_neutral"]     = vector<float> {};//muon.pfIsolationR04().sumNeutralHadronEt;
  vars_["Muon_pfIso04_photon"]      = vector<float> {};//muon.pfIsolationR04().sumPhotonEt;
  // Muon TOF (Combined)
  //vars_["Muon_time_nDof"]               = vector<int> {};
  vars_["Muon_time_inverseBeta"]        = vector<float> {};
  vars_["Muon_time_inverseBetaErr"]     = vector<float> {};

  //gen. particles
  vars_["GenPart_pt"]      = vector<double> {};
  vars_["GenPart_eta"]     = vector<double> {};
  vars_["GenPart_phi"]     = vector<double> {};
  vars_["GenPart_mass"]    = vector<double> {};
  vars_["GenPart_energy"]  = vector<double> {};
  vars_["GenPart_p"]       = vector<double> {};
  vars_["GenPart_charge"]  = vector<int> {};
  vars_["GenPart_status"]  = vector<int> {};
  vars_["GenPart_pdgId"]   = vector<int> {};
  vars_["GenPart_beta"]    = vector<double> {};

  // Di-gluon pT
  vars_["GenPart_gluino_n"]      = uint32_t {0};
  vars_["GenPart_digluino_pt"]   = double {0};


  for( auto& pair : vars_ ) {
    treeManager_->book(pair.first.c_str(), pair.second);
  }

  bool splitByModuleType = true;
  //bool puTreatment = false;
  dEdxTemplates = loadDeDxTemplate(dEdxTemplate_, splitByModuleType, /*puTreatment*/false,0);
  dEdxTemplatesPU.resize(NbPuBins_, nullptr);
  for (int i = 0; i < NbPuBins_ ; i++){
    dEdxTemplatesPU[i] = loadDeDxTemplate(dEdxTemplate_, splitByModuleType, /*puTreatment*/true,(i+1));
  }
  if (dEdxTemplates==nullptr && dEdxTemplatesPU[0]==nullptr)
    edm::LogWarning("loadDeDxTemplate") << "WARNING!! Couldn't find Gi templates. No corrections added!";
}


void Analyzer::analyze(const edm::Event &iEvent, const edm::EventSetup &iSetup) {
  // if the only purpose is to trick CRAB to do a TAPERECALL
  if (tapeRecallOnly_) return;

  //-----------------------------------------//
  // Clear vector branches                   //
  //-----------------------------------------//
  for( auto& pair : vars_ ) {
    treeManager_->clearVectorBranch(pair.first.c_str(), pair.second);
  }

  bool EvtCuts[N_CUTS];
  EvtCuts[0] = true; EventCutFlow_->Fill(0);

  using namespace std;
  using namespace edm;
  using namespace reco;
  using namespace pat;

  // Retrieve tracker topology/geometry/CPE from the event setup
  const TrackerTopology* tTopo = &iSetup.getData(trackerTopoToken_);
  const TrackerGeometry* tkGeometry = &iSetup.getData(geometryToken_);
  const PixelClusterParameterEstimator* pixelCPE = &iSetup.getData(trackerPixelCPEToken_);
  //edm::LogDebug("PIXELCPE") << " Asking for the Pixel-CPE with name " << pixelCPE_ << endl;
  //edm::LogDebug("PIXELCPE") << " Got a " << typeid(pixelCPE).name() << endl;

  // load infos
  const edm::Handle<TriggerResults> trigger = iEvent.getHandle(triggerToken_);
  const edm::Handle<pat::TriggerObjectStandAloneCollection> triggerObjects = iEvent.getHandle(triggerObjects_);
  const edm::Handle<trigger::TriggerEvent> trigEvent = iEvent.getHandle(trigEventToken_);
  const edm::Handle<edm::View<pat::IsolatedTrack>> trackCollection = iEvent.getHandle(trackToken_);
  const edm::Handle<std::vector<pat::Muon>> muonCollection = iEvent.getHandle(muonToken_);
  const edm::Handle<reco::DeDxHitInfoAss> dedxCollection = iEvent.getHandle(dedxToken_);
  const edm::Handle<pat::PackedCandidateCollection> pfCandHandle = iEvent.getHandle(pfCandToken_);
  // std::vector<pat::PackedCandidateCollection> pfCands = *pfCandHandle;
  // const pat::PackedCandidateCollection pfCands = &pfCandHandle;


  isData_ = iEvent.isRealData();
  //-Wunused-variable//uint32_t run_number = iEvent.run();

  dEdxSF[0] = dEdxSF_0_;
  dEdxSF[1] = dEdxSF_1_;

  // Event information
  //------------------------------------------------------------------------
  vars_["isData"]            = iEvent.isRealData();
  vars_["run"]               = iEvent.run();
  vars_["luminosityBlock"]   = iEvent.luminosityBlock();
  vars_["event"]             = iEvent.id().event();
  vars_["bunchCrossing"]     = iEvent.bunchCrossing();

  year_ = (isData_) ? getYear(iEvent) : "-1";

  // Vertex
  //------------------------------------------------------------------------
  HSCPVertexSelector *vertexSelector = new HSCPVertexSelector(iEvent.get(vertexToken_));
  vector<Vertex> goodVertices = vertexSelector->getGoodVertices();
  const int numGoodVerts = goodVertices.size();
  vars_["PV_npvsGood"] = numGoodVerts;
  for( const auto &pv: goodVertices){
    addToVectorBranch(vars_, "PV_ndof", pv.ndof());
    addToVectorBranch(vars_, "PV_x", pv.x());
    addToVectorBranch(vars_, "PV_y", pv.y());
    addToVectorBranch(vars_, "PV_z", pv.z());
    addToVectorBranch(vars_, "PV_rho", pv.position().Rho());
  }
  auto bestVertex = vertexSelector->getBestVertex();

  EvtCuts[1] = EvtCuts[0] && (numGoodVerts>0);  if (EvtCuts[1]) EventCutFlow_->Fill(1);

  // HLT information
  //------------------------------------------------------------------------
  LogInfo(MOD) << "=== TRIGGER ===";
  const auto triggerNames = iEvent.triggerNames(*trigger);
  std::vector<bool> triggerDecision;

  bool passedFilterOR = trigtools::passedHLT(triggerNames, trigger, triggerPaths_, triggerDecision);
  
  for (unsigned int i = 0; i < triggerPaths_.size(); i++) {
    vars_[triggerBranchNames_.at(i).c_str()] = (bool) triggerDecision.at(i);
  }
  vars_["HLT_FilterOR"] = passedFilterOR;

  EvtCuts[2] = EvtCuts[1] && (passedFilterOR);  if (EvtCuts[2]) EventCutFlow_->Fill(2);

  if (triggerFilter_){
    if (!passedFilterOR) return;
  }

  // HLT Mu Trigger matching with Trigger Object
  //------------------------------------------------------------------------
  std::vector<pat::Muon> muons = *muonCollection;
  std::vector<ROOT::Math::PtEtaPhiMVector> trigObjP4s;
  for (pat::TriggerObjectStandAlone obj : *triggerObjects){
    obj.unpackFilterLabels(iEvent, *trigger);
    for (unsigned h = 0; h < obj.filterLabels().size(); ++h){
      if (obj.filterLabels()[h]==filterName_){
        ROOT::Math::PtEtaPhiMVector objP4(obj.pt(),obj.eta(),obj.phi(),obj.mass());
        trigObjP4s.push_back(objP4);
      }
    }
  }
  bool matchedMuonWasFound = findBestHLTMuonMatch(trigObjP4s, muons, bestVertex);
  EvtCuts[3] = EvtCuts[2] && (matchedMuonWasFound);  if (EvtCuts[3]) EventCutFlow_->Fill(3);

  // L1 Trigger EtSum
  //------------------------------------------------------------------------
  edm::Handle<l1t::EtSumBxCollection> l1TriggerEtSumHandle = iEvent.getHandle(l1TriggerEtSumToken_);
  if (!l1TriggerEtSumHandle.isValid()){
    LogPrint(MOD) << "Invalid EtSumBxCollection: Run "<<iEvent.run()<<", Event "<<iEvent.id().event()<<", LumiSection "<<iEvent.luminosityBlock()<<" !";
  }
  else{
    l1t::EtSumHelper hsum(l1TriggerEtSumHandle);
    vars_["L1MHT"] = hsum.MissingHt();
    vars_["L1MHT_phi"] = hsum.MissingHtPhi();
    vars_["L1ETSum"] = hsum.TotalEt();
    vars_["L1HTSum"] = hsum.TotalHt();
    //energy sum
    for (int itBX = l1TriggerEtSumHandle->getFirstBX(); itBX <= l1TriggerEtSumHandle->getLastBX(); ++itBX){
      for (l1t::EtSumBxCollection::const_iterator itEtSum = l1TriggerEtSumHandle->begin(itBX); itEtSum!= l1TriggerEtSumHandle->end(itBX); ++itEtSum) {
        if (itBX == 0){
          if (l1t::EtSum::EtSumType::kMissingEt == itEtSum->getType()){ // MET
            vars_["L1MET"] = itEtSum->pt();
            vars_["L1MET_phi"] = itEtSum->phi();
          }
          if (l1t::EtSum::EtSumType::kMissingEtHF == itEtSum->getType()){ // METHF
            vars_["L1METHF"] = itEtSum->pt();
            vars_["L1METHF_phi"] = itEtSum->phi();
          }
        }
      }
    }
  }
  // RecoPFMET & RecoCaloMET
  //------------------------------------------------------------------------
  const edm::Handle<pat::METCollection> mets = iEvent.getHandle(metToken_);
  vars_["RecoCaloMET"]=-10; vars_["RecoCaloMET_phi"]=-10; vars_["RecoCaloMET_sigf"]=-10;
  vars_["RecoPFMET"]=-10;   vars_["RecoPFMET_phi"]=-10;   vars_["RecoPFMET_sigf"]=-10;                           
  if (mets.isValid() && !mets->empty()) {
    const pat::MET &met = mets->front();

    vars_["RecoCaloMET"]      = met.caloMETPt();
    vars_["RecoCaloMET_phi"]  = met.caloMETPhi();
    if (met.isCaloMET()) vars_["RecoCaloMET_sigf"] = met.caloMetSignificance();//Otherwise get ERROR This pat::MET has not been made from a reco::CaloMET

    vars_["RecoPFMET"]      = met.pt();
    vars_["RecoPFMET_phi"]  = met.phi();
    vars_["RecoPFMET_sigf"] = met.significance();
  }

  std::map<std::string, float> met_map = trigtools::getHLTMETOjects(*triggerObjects);
  vars_["HLTCaloMET"] = met_map["HLTCaloMET"];
  vars_["HLTCaloMET_phi"] = met_map["HLTCaloMET_phi"];
  vars_["HLTCaloMET_sigf"] = met_map["HLTCaloMET_sigf"];
  vars_["HLTCaloMETClean"] = met_map["HLTCaloMETClean"];
  vars_["HLTCaloMETClean_phi"] = met_map["HLTCaloMETClean_phi"];
  vars_["HLTCaloMETClean_sigf"] = met_map["HLTCaloMETClean_sigf"];
  vars_["HLTCaloMHT"] = met_map["HLTCaloMHT"];
  vars_["HLTCaloMHT_phi"] = met_map["HLTCaloMHT_phi"];
  vars_["HLTCaloMHT_sigf"] = met_map["HLTCaloMHT_sigf"];
  vars_["HLTPFMHT"] = met_map["HLTPFMHT"];
  vars_["HLTPFMHT_phi"] = met_map["HLTPFMHT_phi"];
  vars_["HLTPFMHT_sigf"] = met_map["HLTPFMHT_sigf"];
  vars_["HLTPFMET"] = met_map["HLTPFMET"];
  vars_["HLTPFMET_phi"] = met_map["HLTPFMET_phi"];
  vars_["HLTPFMET_sigf"] = met_map["HLTPFMET_sigf"];

  // MET Filters
  //------------------------------------------------------------------------
  const edm::Handle<TriggerResults> noiseFilter = iEvent.getHandle(noiseCleaningFilterToken_);
  if (noiseFilter.isValid()){
    const auto filterNames = iEvent.triggerNames(*noiseFilter);
    bool passedAllFilters(true);
    for (unsigned int i = 0; i < filterNames.triggerNames().size(); i++){
      std::string filterName = filterNames.triggerNames()[i];
      if (filterName.find("Flag_globalSuperTightHalo2016Filter") != std::string::npos) {vars_["Flag_globalSuperTightHalo2016Filter"] = noiseFilter->accept(i); passedAllFilters&=noiseFilter->accept(i);}
      else if (filterName.find("Flag_HBHENoiseFilter") != std::string::npos) {vars_["Flag_HBHENoiseFilter"] = noiseFilter->accept(i); passedAllFilters&=noiseFilter->accept(i);}
      else if (filterName.find("Flag_HBHENoiseIsoFilter") != std::string::npos) {vars_["Flag_HBHENoiseIsoFilter"] = noiseFilter->accept(i); passedAllFilters&=noiseFilter->accept(i);}
      else if (filterName.find("Flag_EcalDeadCellTriggerPrimitiveFilter") != std::string::npos) {vars_["Flag_EcalDeadCellTriggerPrimitiveFilter"] = noiseFilter->accept(i); passedAllFilters&=noiseFilter->accept(i);}
      else if (filterName.find("Flag_BadPFMuonFilter") != std::string::npos) {vars_["Flag_BadPFMuonFilter"] = noiseFilter->accept(i); passedAllFilters&=noiseFilter->accept(i);}
      else if (filterName.find("Flag_BadPFMuonDzFilter") != std::string::npos) {vars_["Flag_BadPFMuonDzFilter"] = noiseFilter->accept(i); passedAllFilters&=noiseFilter->accept(i);}
      else if (filterName.find("Flag_hfNoisyHitsFilter") != std::string::npos) {vars_["Flag_hfNoisyHitsFilter"] = noiseFilter->accept(i); passedAllFilters&=noiseFilter->accept(i);}
      else if (filterName.find("Flag_eeBadScFilter") != std::string::npos) {vars_["Flag_eeBadScFilter"] = noiseFilter->accept(i); passedAllFilters&=noiseFilter->accept(i);}
      else if (filterName.find("Flag_ecalBadCalibFilter") != std::string::npos) {vars_["Flag_ecalBadCalibFilter"] = noiseFilter->accept(i); passedAllFilters&=noiseFilter->accept(i);}
      else if (filterName.find("Flag_METFilters") != std::string::npos) {vars_["Flag_METFilters"] = noiseFilter->accept(i);}
    }
    vars_["Flag_allMETFilters"] = passedAllFilters;
  }

   //generator stuff
   std::vector<pat::PackedGenParticle> genColl;
   vector<TLorentzVector> gluino4vec;

  if (!isData_){
    // Gen Event Infos
    //------------------------------------------------------------------------
    const edm::Handle<GenEventInfoProduct> genEvt = iEvent.getHandle(genEventToken_);
    if (genEvt.isValid()){
      vars_["weight_generator"] = genEvt->weight();
      if(genEvt->binningValues().size() > 0)
        vars_["weight_generatorBinningValues"] = genEvt->hasBinningValues() ? (genEvt->binningValues())[0] : 0.0;//pthat
    }
    else{
      LogDebug(MOD) << "Invalid GenEventInfoProduct collection!";
    }
    // Gen particles
    //------------------------------------------------------------------------
    const edm::Handle<std::vector<pat::PackedGenParticle>> genCollH = iEvent.getHandle(genParticleToken_);
    if (!genCollH.isValid()) {
      LogPrint(MOD) << "Invalid GenParticle collection, this event will be ignored!";
      return;
    } else {
      genColl = *genCollH;
    }
    
    for (auto const& gen : genColl){
      //
      if (gen.pt()<10) continue;
      //if (gen.status()!=1) continue;
      //
      addToVectorBranch(vars_,"GenPart_pt",     gen.pt());
      addToVectorBranch(vars_,"GenPart_eta",    gen.eta());
      addToVectorBranch(vars_,"GenPart_phi",    gen.phi());
      addToVectorBranch(vars_,"GenPart_mass",   gen.mass());
      addToVectorBranch(vars_,"GenPart_energy", gen.energy());
      addToVectorBranch(vars_,"GenPart_p",      gen.p());
      addToVectorBranch(vars_,"GenPart_charge", gen.charge());
      addToVectorBranch(vars_,"GenPart_status", gen.status());
      addToVectorBranch(vars_,"GenPart_pdgId",  gen.pdgId());
      addToVectorBranch(vars_,"GenPart_beta",  gen.p()/gen.energy());

      if ( fabs(gen.pdgId())==1000021 
         //&& status==102 
         //&& (fabs(2000.-mass) < 2000*0.1) // 10% mass window
      ){
        TLorentzVector tvec;
        tvec.SetPtEtaPhiM(gen.pt(), gen.eta(), gen.phi(), gen.mass());
        gluino4vec.push_back(tvec);
      }
    }
  }

  vars_["GenPart_gluino_n"] = gluino4vec.size();
  float digluon_pt;
  if (gluino4vec.size() == 2)  digluon_pt = (gluino4vec[0] + gluino4vec[1]).Pt();
  else digluon_pt = -999;
  vars_["GenPart_digluino_pt"] = digluon_pt;

  // for(unsigned int it = 0; it < trackCollection->size(); it++){
  //   auto isotrack = trackCollection->ptrAt(it);
  //   std::cout<< ">> miniPFIsolation().chargedHadronIso()" << isotrack->miniPFIsolation().chargedHadronIso()<<std::endl;
  // }
  

  //------------------------------------------------------------------------
  // HSCPs
  //------------------------------------------------------------------------
  TH3F* localdEdxTemplates;
  localdEdxTemplates = dEdxTemplates;
  bool skipPixelL1 = true;
  bool mustBeInside = true;
  //-Wunused-variable//size_t MaxStripNOM = 99;
  //-Wunused-variable//bool correctFEDSat = false;
  int crossTalkInvAlgo = 1;
  float dropLowerDeDxValue = 0.0;
  //-Wunused-variable//float dEdxErr = 0;
  unsigned int nHSCP = 0, nTrack = 0, nMuon = 0;
  for (const auto& hscp : iEvent.get(hscpToken_)) {
    
    pat::IsolatedTrack isotrack = hscp.track();
    const pat::PackedCandidateRef track = isotrack.packedCandRef();
    const pat::MuonRef  muon  = hscp.muon();
    bool hasTrack = (hscp.hasTrack());
    bool hasMuon  = (hscp.hasMuon());
    bool hasDeDx  = false;

    int hscpType = -1;
    if (type(hscp) == HSCPType::globalMuon) //globalMuon
      hscpType = 0;
    else if (type(hscp) == HSCPType::trackerMuon) //trackerMuon
      hscpType = 1;
    else if (type(hscp) == HSCPType::matchedStandAloneMuon) //matchedStandAloneMuon
      hscpType = 2;
    else if (type(hscp) == HSCPType::standAloneMuon) //standAloneMuon
      hscpType = 3;
    else if (type(hscp) == HSCPType::innerTrack) //innerTrack
      hscpType = 4;
    else if (type(hscp) == HSCPType::unknown)
      hscpType = 5;

    //if ((type(hscp) != 1) && (type(hscp) != 0) && (htype(hscp) != 4)) continue;
    if ((type(hscp) != HSCPType::trackerMuon) && (type(hscp) != HSCPType::globalMuon) && (type(hscp) != HSCPType::innerTrack)) continue;

    //TESTME
    //////if (track.isNull()) continue;

    if (hasTrack){
      bool isHighPurity = track->trackHighPurity();

    // MiniIsolation
    //------------------------------------------------------------------------
    bool track_isPF = true;
    float miniRelIsoChg = isotrack.miniPFIsolation().chargedHadronIso();//track->pt();
    float miniRelIsoAll = (isotrack.miniPFIsolation().chargedHadronIso()
                          +isotrack.miniPFIsolation().neutralHadronIso()
                          +isotrack.miniPFIsolation().photonIso()
                          +isotrack.miniPFIsolation().puChargedHadronIso());
    //energy of nearest calojet within a given dR;
    float pf_ecalEnergy = isotrack.matchedCaloJetEmEnergy();
    float pf_hcalEnergy = isotrack.matchedCaloJetHadEnergy();
    float pf_energy = pf_ecalEnergy + pf_hcalEnergy;

    if (!isData_){
      int closestGenIndex = findBestHSCPMatch(genColl, track, 0.015);//dR(Track,HSCP)=0.015
      if (closestGenIndex < 0) continue;
    }

    // dE/dx
    //------------------------------------------------------------------------
      const reco::DeDxHitInfo* dedxHits = hscp.dedxHitInfo();
      if (dedxHits) hasDeDx = true;
      if (!dedxHits) {continue;}

      TH3* templateHisto = nullptr;
      HSCPDeDxTool* dedxTool = new HSCPDeDxTool(iEvent, dedxHits, track, tTopo, tkGeometry);
      dedxTool->computedEdx(dEdxSF, useClusterCleaning, mustBeInside, crossTalkInvAlgo,dropLowerDeDxValue,templateHisto,skipPixelL1);
      auto dedxData_FullTracker = dedxTool->dedxDataFullTracker();
      auto dedxData_FullTracker_noL1 = dedxTool->dedxDataFullTrackerNoL1();
      auto dedxData_StripOnly = dedxTool->dedxDataStripOnly();

      HSCPDeDxTool* dedxDiscriminator = new HSCPDeDxTool(iEvent, dedxHits, track, tTopo, tkGeometry);
      int NPV = numGoodVerts;
      for(int i = 0 ; i < NbPuBins_ ; i++){
        if ( NPV > PuBins_[i] && NPV <= PuBins_[i+1] ) dedxDiscriminator->computedEdx(dEdxSF, useClusterCleaning, mustBeInside, crossTalkInvAlgo,dropLowerDeDxValue,localdEdxTemplates = dEdxTemplatesPU[i],skipPixelL1);
      }
      // auto dedxIas_FullTracker = dedxTool->dedxIasFullTracker();
      // auto dedxIas_StripOnly = dedxTool->dedxIasStripOnly();
      auto dedxIas_FullTracker = dedxDiscriminator->dedxIasFullTracker();
      auto dedxIas_StripOnly = dedxDiscriminator->dedxIasStripOnly();

      dedxTool->computeProbQ(pixelCPE);

      /*auto dedxMObj_FullTrackerTmp =
          computedEdx(track->eta(),iSetup, run_number, to_string(year), dedxHits, dEdxSF, localdEdxTemplates = nullptr, 
                      usePixel = true, useStrip = true, useClusterCleaning, false,
                      mustBeInside, MaxStripNOM, correctFEDSat, 1, dropLowerDeDxValue = 0.0, &dEdxErr, useTemplateLayer_,
                      false,0, false, false, true, pixelCPE_, tTopo, tkGeometry, pixelCPE,
                      track->px(), track->py(), track->pz(), track->charge());*/

      addToVectorBranch(vars_,"IsoTrack_charge", track->charge());
      addToVectorBranch(vars_,"IsoTrack_p",  track->p());
      addToVectorBranch(vars_,"IsoTrack_px", track->px());
      addToVectorBranch(vars_,"IsoTrack_py", track->py());
      addToVectorBranch(vars_,"IsoTrack_pz", track->pz());
      addToVectorBranch(vars_,"IsoTrack_pt", track->pt());
      addToVectorBranch(vars_,"IsoTrack_ptError", track->pseudoTrack().ptError() );
      addToVectorBranch(vars_,"IsoTrack_ptErrOverPt", track->pseudoTrack().ptError()/track->pseudoTrack().pt() );
      addToVectorBranch(vars_,"IsoTrack_ptErrOverPt2", track->pseudoTrack().ptError()/track->pseudoTrack().pt2() );
      addToVectorBranch(vars_,"IsoTrack_eta", track->eta() );
      addToVectorBranch(vars_,"IsoTrack_phi", track->phi() );
      addToVectorBranch(vars_,"IsoTrack_dz", track->dz(bestVertex.position()) );
      addToVectorBranch(vars_,"IsoTrack_dzError", track->dzError() );
      addToVectorBranch(vars_,"IsoTrack_dxy", track->dxy(bestVertex.position()) );
      addToVectorBranch(vars_,"IsoTrack_dxyError", track->dxyError() );
      addToVectorBranch(vars_,"IsoTrack_normChi2", track->pseudoTrack().normalizedChi2() );
      addToVectorBranch(vars_,"IsoTrack_isHighPurityTrack", isHighPurity );
      addToVectorBranch(vars_,"IsoTrack_fractionOfValidHits", track->pseudoTrack().validFraction());
      addToVectorBranch(vars_,"IsoTrack_numberOfValidHits", track->pseudoTrack().numberOfValidHits());
      addToVectorBranch(vars_,"IsoTrack_numberOfValidPixelHits", track->pseudoTrack().hitPattern().numberOfValidPixelHits());
      addToVectorBranch(vars_,"IsoTrack_numberOfTrackerLayers", track->pseudoTrack().hitPattern().trackerLayersWithMeasurement());
      addToVectorBranch(vars_,"IsoTrack_isPFcand", track_isPF);
      addToVectorBranch(vars_,"IsoTrack_pfMiniRelIsoChg", miniRelIsoChg);
      addToVectorBranch(vars_,"IsoTrack_pfMiniRelIsoAll", miniRelIsoAll);
      addToVectorBranch(vars_,"IsoTrack_pfEnergyOverP", pf_energy/track->p());
      addToVectorBranch(vars_,"IsoTrack_pfEcalEnergy", pf_ecalEnergy);
      addToVectorBranch(vars_,"IsoTrack_pfHcalEnergy", pf_hcalEnergy);

      addToVectorBranch(vars_,"DeDx_Ih", dedxData_FullTracker.dEdx() );
      addToVectorBranch(vars_,"DeDx_IhNOM", dedxData_FullTracker.numberOfMeasurements());
      addToVectorBranch(vars_,"DeDx_IhNoL1", dedxData_FullTracker_noL1.dEdx() );
      addToVectorBranch(vars_,"DeDx_IhNoL1NOM", dedxData_FullTracker_noL1.numberOfMeasurements());//????? 
      addToVectorBranch(vars_,"DeDx_IhStrip", dedxData_StripOnly.dEdx() );

      addToVectorBranch(vars_,"DeDx_Gi", dedxIas_FullTracker.dEdx() );
      addToVectorBranch(vars_,"DeDx_GiStrip", dedxIas_StripOnly.dEdx());

      addToVectorBranch(vars_,"DeDx_FiPixel", 1-dedxTool->probQonTrack());
      addToVectorBranch(vars_,"DeDx_FiPixelNoL1", 1-dedxTool->probQonTrackNoL1());
      addToVectorBranch(vars_,"DeDx_PixelNoL1NOM", dedxTool->numberOfPixelNoL1Measurement());
      addToVectorBranch(vars_,"DeDx_NoL1NOM", dedxTool->numberOfPixelNoL1Measurement()+dedxIas_StripOnly.numberOfMeasurements());

      nTrack++;
    }//end HSCP with Tracks
    if (hasMuon){ 
      addToVectorBranch(vars_,"Muon_pt", muon->pt());
      addToVectorBranch(vars_,"Muon_eta", muon->eta());
      addToVectorBranch(vars_,"Muon_phi", muon->phi());
      addToVectorBranch(vars_,"Muon_isPFcand", muon->isPFMuon());
      addToVectorBranch(vars_,"Muon_isStandAlone", muon->isStandAloneMuon());
      addToVectorBranch(vars_,"Muon_isTracker", muon->isTrackerMuon());
      addToVectorBranch(vars_,"Muon_isGlobal", muon->isGlobalMuon());
      addToVectorBranch(vars_,"Muon_pfIso03_sumPU", muon->pfIsolationR03().sumPUPt);
      //addToVectorBranch(vars_,"Muon_combRelIsoPF03dBeta", muon->pfIsolationR04().sumChargedHadronPt);
      addToVectorBranch(vars_,"Muon_pfIso04_charged", muon->pfIsolationR04().sumChargedHadronPt);
      addToVectorBranch(vars_,"Muon_pfIso04_neutral", muon->pfIsolationR04().sumNeutralHadronEt);
      addToVectorBranch(vars_,"Muon_pfIso04_photon", muon->pfIsolationR04().sumPhotonEt);

      // MuonTimeExtra information
      addToVectorBranch(vars_,"Muon_time_inverseBeta",muon->inverseBeta());
      addToVectorBranch(vars_,"Muon_time_inverseBetaErr",muon->inverseBetaErr());

      nMuon++;
    }//end HSCP with Muons

    addToVectorBranch(vars_,"HSCP_type", hscpType); 
    addToVectorBranch(vars_,"HSCP_hasTrack", hasTrack);
    addToVectorBranch(vars_,"HSCP_hasMuon", hasMuon);
    addToVectorBranch(vars_,"HSCP_hasDeDx", hasDeDx);

    nHSCP++;
  }//// end HSCP loop
  //------------------------------------------------------------------------
  vars_["HSCP_n"]  = nHSCP;
  vars_["nIsoTrack"] = nTrack;
  vars_["nMuon"]  = nMuon;

  EvtCuts[4] = EvtCuts[3] && (nHSCP>0);   if (EvtCuts[4]) EventCutFlow_->Fill(4);
  EvtCuts[5] = EvtCuts[4] && (nTrack>0);  if (EvtCuts[5]) EventCutFlow_->Fill(5);
  EvtCuts[6] = EvtCuts[4] && (nMuon>0);   if (EvtCuts[6]) EventCutFlow_->Fill(6);

  // Fill event
  treeManager_->fill();


  #ifdef THIS_IS_AN_EVENT_EXAMPLE
   Handle<ExampleData> pIn;
   iEvent.getByLabel("example",pIn);
  #endif

  #ifdef THIS_IS_AN_EVENTSETUP_EXAMPLE
    ESHandle<SetupData> pSetup;
    iSetup.get<SetupRecord>().get(pSetup);
  #endif
}


void Analyzer::endJob() {
  // if the only purpose is to trick CRAB to do a TAPERECALL
  if (tapeRecallOnly_) return;

  std::string input_type = isData_ ? Form("Data %s:",year_.c_str()) : Form("MC %s:",year_.c_str());
  LogPrint(MOD) << "\n" <<  input_type.c_str();
  int N_CUTS = EventCutFlow_->GetNbinsX();
  for(int i=0; i<N_CUTS; i++){
    if (EventCutFlow_->GetBinContent(1)==0) break;
    LogPrint(MOD) <<  Form("%10s = %.0f \t:  %.3f", EventCutFlowLabels[i].c_str(), EventCutFlow_->GetBinContent(i+1), EventCutFlow_->GetBinContent(i+1)/EventCutFlow_->GetBinContent(1));
    EventCutFlow_->GetXaxis()->SetBinLabel(i+1,EventCutFlowLabels[i].c_str());
  }
}

int Analyzer::type(susybsm::HSCParticle hscp){
  if (hscp.hasTrack() && !hscp.hasMuon()) {
    return HSCPType::innerTrack;
  } else if (!hscp.hasTrack() && hscp.hasMuon()) {
    return HSCPType::standAloneMuon;
  } else if (hscp.hasTrack() && hscp.hasMuon() && hscp.muon()->isGlobalMuon()) {
    return HSCPType::globalMuon;
  } else if (hscp.hasTrack() && hscp.hasMuon() && hscp.muon()->isStandAloneMuon()) {
    return HSCPType::matchedStandAloneMuon;
  } else if (hscp.hasTrack() && hscp.hasMuon() && hscp.muon()->isTrackerMuon()) {
    return HSCPType::trackerMuon;
  } else
    return HSCPType::unknown;
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void Analyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setComment("Run2 Analyzer for HSCP search");
  desc.add("HscpCollection",      edm::InputTag("HSCParticleProducer"))->setComment("Input collection for HSCP candidate");
  desc.add("TriggerCollection",   edm::InputTag("TriggerResults","","HLT"));
  desc.add("TriggerObjects",      edm::InputTag("slimmedPatTrigger"));
  desc.add("TriggerSummary",      edm::InputTag("hltTriggerSummaryAOD"));
  //desc.add("TriggerSummary",      edm::InputTag("selectedPatTrigger"));
  desc.add("OfflinePVCollection", edm::InputTag("offlineSlimmedPrimaryVertices"));//same content as the AOD
  desc.add("TrackCollection",     edm::InputTag("packedPFCandidates"));
  desc.add("TrackIsoCollection",  edm::InputTag("packedPFCandidates"));
  desc.add("MuonCollection",      edm::InputTag("slimmedMuons"));
  // desc.add("MuonTimeCollection", edm::InputTag("muons", "combined"))->setComment("combined muon timing information");
  // desc.add("MuonDtTimeCollection", edm::InputTag("muons", "dt"))->setComment("dt");
  // desc.add("MuonCscTimeCollection", edm::InputTag("muons", "csc"))->setComment("csc");
  desc.add("GenPartCollection",   edm::InputTag("packedGenParticles"));
  desc.add("GenCollection",       edm::InputTag("generator","","RECO"))->setComment("A");
  desc.add("DeDxCollection",      edm::InputTag("isolatedTracks"));
  desc.add("PfCand",              edm::InputTag("packedPFCandidates"));
  desc.addUntracked("TriggerPaths", std::vector<std::string>{"HLT_Mu50_v"});
  desc.addUntracked("TriggerFilter", true);
  //MET
  desc.add("l1TriggerEtSum",       edm::InputTag("caloStage2Digis","EtSum"));
  desc.add("SlimmedMET",                edm::InputTag("slimmedMETs"));
  //desc.add("CaloMET",              edm::InputTag("caloMet"));
  //
  desc.add("NoiseFilters",   edm::InputTag("TriggerResults","","PAT"));
  //"PileupSummaryInfo" -> "slimmedAddPileupInfo"
  desc.add<std::string>("PixelCPE","PixelCPETemplateReco")->setComment("CPE used in the pixel reco, PixelCPEClusterRepair is the best available so far, template only is PixelCPETemplateReco");
  desc.addUntracked("TapeRecallOnly",false)->setComment("Set to true if the only purpose is to trick CRAB to do a TAPERECALL");
  desc.add<std::string>("FilterName",std::string("hltL3fL1sMu22Or25L1f0L2f10QL3Filtered50Q")) 
      ->setComment("Meaning of this is: L1f0  = L1 filtered with threshold 0, L2f10Q = L2 filtered at 10 GeV with quality cuts, L3Filtered50Q = L3 filtered at 50 GeV with quality cuts");

  desc.addUntracked("DeDxSF_0",1.0)->setComment(", really controlled by the config for each era");
  desc.addUntracked("DeDxSF_1",1.035)->setComment("Scale factor to scale the pixel charge to match the strips scale, really controlled by the config for each era");
  desc.addUntracked("DeDxK",2.3)->setComment("K constant, really controlled by the config for each era");
  desc.addUntracked("DeDxC",3.17)->setComment("C constant, really controlled by the config for each era");
  desc.addUntracked<std::string>("DeDxTemplate","")->setComment("Norm charge vs path lenght vs module geometry templates for the strips detector, really controlled by the config for each era");

  desc.addUntracked("AddStripClusterInfo",false)->setComment("Add dedx measurements");

  descriptions.add("HSCParticleAnalyzer",desc);

  //desc.setUnknown();
  //descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addWithDefaultLabel(desc);
}

DEFINE_FWK_MODULE(Analyzer);
