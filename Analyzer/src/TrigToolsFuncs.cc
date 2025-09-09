#include "SUSYBSMAnalysis/Analyzer/interface/TrigToolsFuncs.h"

#include "FWCore/Utilities/interface/InputTag.h"

#include "DataFormats/HLTReco/interface/TriggerEvent.h"

//=============================================================
//     HLT Trigger paths (match patterns)
//=============================================================
bool trigtools::passedHLT(const edm::TriggerNames triggerNames, const edm::Handle<edm::TriggerResults> triggerResults, std::vector<std::string> triggerPaths, std::vector<bool> &triggerDecisions){
  bool passedFilterOR(false), passed(false);
  for (auto triggerPath : triggerPaths) {
    bool pathFound = false;
    std::string name ="";
    for (unsigned int i = 0; i < triggerNames.triggerNames().size(); i++){
      name = triggerNames.triggerNames()[i];
      if (name.find(triggerPath) != std::string::npos){
        pathFound = true;
        passed = (triggerResults->accept(i));
        passedFilterOR |= (passed);
        break;
      }
    }
    if (!pathFound) throw cms::Exception("TriggerNames")  << "=== Name '" << triggerPath << "' does NOT match any trigger path." << std::endl;
    //std::cout<<"Found Trigger Path: "<<name<<std::endl;
    triggerDecisions.push_back(passed);
  }

  if (triggerPaths.size()!=triggerDecisions.size()) throw cms::Exception("TriggerDecisions")  << "=== Mismatch." << std::endl;

  return passedFilterOR;
}

void trigtools::getP4sOfObsPassingFilter(std::vector<math::XYZTLorentzVector>& p4s,const trigger::TriggerEvent& trigEvent,const std::string& filterName,const std::string& hltProcess)
{
  p4s.clear();

  edm::InputTag filterTag(filterName,"",hltProcess); 
  trigger::size_type filterIndex = trigEvent.filterIndex(filterTag); 
  if(filterIndex<trigEvent.sizeFilters()){ //check that filter is in triggerEvent
    const trigger::Keys& trigKeys = trigEvent.filterKeys(filterIndex); 
    const trigger::TriggerObjectCollection & trigObjColl(trigEvent.getObjects());
    for(trigger::Keys::const_iterator keyIt=trigKeys.begin();keyIt!=trigKeys.end();++keyIt){ 
      const trigger::TriggerObject& obj = trigObjColl[*keyIt];
      math::XYZTLorentzVector objP4;
      objP4.SetPxPyPzE(obj.px(),obj.py(),obj.pz(),obj.energy());
      p4s.push_back(objP4);
    }//end loop over keys
  }//end check that filter is valid and in trigEvent
}



void trigtools::getP4sOfObsPassingFilter(std::vector<TLorentzVector>& p4s,const trigger::TriggerEvent& trigEvent,const std::string& filterName,const std::string& hltProcess)
{
  p4s.clear();
 
  edm::InputTag filterTag(filterName,"",hltProcess); 
  trigger::size_type filterIndex = trigEvent.filterIndex(filterTag); 
  if(filterIndex<trigEvent.sizeFilters()){ //check that filter is in triggerEvent
    const trigger::Keys& trigKeys = trigEvent.filterKeys(filterIndex); 
    const trigger::TriggerObjectCollection & trigObjColl(trigEvent.getObjects());
    for(trigger::Keys::const_iterator keyIt=trigKeys.begin();keyIt!=trigKeys.end();++keyIt){ 
      const trigger::TriggerObject& obj = trigObjColl[*keyIt];
      TLorentzVector objP4;
      objP4.SetPtEtaPhiM(obj.pt(),obj.eta(),obj.phi(),obj.mass());
      p4s.push_back(objP4);
    }//end loop over keys
  }//end check that filter is valid and in trigEvent
}

void trigtools::dumpTriggerEvent(const trigger::TriggerEvent& trigEvt)
{
  std::cout <<"number of filters in event "<<trigEvt.sizeFilters()<<std::endl;
  for(size_t filterNr=0;filterNr<trigEvt.sizeFilters();filterNr++){
    const std::string filterName(trigEvt.filterTag(filterNr).label());  
    const trigger::Keys& trigKeys = trigEvt.filterKeys(filterNr);//trigger::Keys is actually a vector<uint16_t> holding the position of trigger objects in the trigger collection passing the filter
    std::cout <<"filter "<<filterName<<" has "<<trigKeys.size()<<" passing "<<std::endl;
    const trigger::TriggerObjectCollection & trigObjColl(trigEvt.getObjects());
    for(trigger::Keys::const_iterator keyIt=trigKeys.begin();keyIt!=trigKeys.end();++keyIt){
      const trigger::TriggerObject& obj = trigObjColl[*keyIt];
      TLorentzVector p4;
      p4.SetPtEtaPhiM(obj.pt(),obj.eta(),obj.phi(),obj.mass());
    }
  } 

  
}


bool trigtools::passedFilter(const trigger::TriggerEvent& trigEvt, const std::string& givenFilter)
{
  for(size_t filterNr=0;filterNr<trigEvt.sizeFilters();filterNr++){
    const std::string filterName(trigEvt.filterTag(filterNr).label());  
    const trigger::Keys& trigKeys = trigEvt.filterKeys(filterNr);//trigger::Keys is actually a vector<uint16_t> holding the position of trigger objects in the trigger collection passing the filter
    if(filterName == givenFilter){
      if(trigKeys.size()>0) return true;
      else
      {
        return false;
      }
    } 
  }
  return false;
}

//===================== AOD HLT Trigger Summary ===================
std::map<std::string, float> trigtools::getHLTMETOjects(const trigger::TriggerEvent& trigEvent)
{
  std::map<std::string, float> met_map = {
    {"HLTCaloMET",      -10}, {"HLTCaloMET_phi",      -10}, {"HLTCaloMET_sigf",      -10},
    {"HLTCaloMETClean", -10}, {"HLTCaloMETClean_phi", -10}, {"HLTCaloMETClean_sigf", -10},
    {"HLTCaloMHT",      -10}, {"HLTCaloMHT_phi",      -10}, {"HLTCaloMHT_sigf",      -10},
    {"HLTPFMHT",        -10}, {"HLTPFMHT_phi",        -10}, {"HLTPFMHT_sigf",        -10},
    {"HLTPFMET",        -10}, {"HLTPFMET_phi",        -10}, {"HLTPFMET_sigf",        -10}
  };

  // loop over trigger object collections to find HLT CaloMET, CaloMETClean, CaloMHT, PFMHT, PFMET collections
  for (int iC = 0; iC < trigEvent.sizeCollections(); iC++) {

    std::string Tag = trigEvent.collectionTag(iC).encode();

    int Key = trigEvent.collectionKey(iC);
    float objpt(trigEvent.getObjects()[Key-4].pt());
    float objphi(trigEvent.getObjects()[Key-4].phi());
    float objsigf(trigEvent.getObjects()[Key-2].pt()); // and -2 for MET significance

    // HLT CaloMET: HLT MET object collections ALWAYS have four objects {MET, TET, MET significance, ELongitudinal}, hence -4 for MET value
    // significance  saved as .pt() but obviously pt holds no meaning here
    if (Tag == "hltMet::HLT"){
      met_map["HLTCaloMET"]      = objpt;//trigEvent.getObjects()[Key-4].pt();
      met_map["HLTCaloMET_phi"]  = objphi;//trigEvent.getObjects()[Key-4].phi();
      met_map["HLTCaloMET_sigf"] = objsigf;//trigEvent.getObjects()[Key-2].pt();// and -2 for MET significance
    }

    // HLT CaloMETClean
    if (Tag=="hltMetClean::HLT"){
      met_map["HLTCaloMETClean"]      = objpt;//trigEvent.getObjects()[Key-4].pt();
      met_map["HLTCaloMETClean_phi"]  = objphi;//trigEvent.getObjects()[Key-4].phi();
      met_map["HLTCaloMETClean_sigf"] = objsigf;//trigEvent.getObjects()[Key-2].pt();
    }

    // HLT CaloMHT: HLT MHT object collections ALWAYS have four objects {MHT, THT, MHT significance, HLongitudinal}, hence -4 for MHT value
    if (Tag=="hltMht::HLT"){
      met_map["HLTCaloMHT"]      = objpt;//trigEvent.getObjects()[Key-4].pt();
      met_map["HLTCaloMHT_phi"]  = objphi;//trigEvent.getObjects()[Key-4].phi();
      met_map["HLTCaloMHT_sigf"] = objsigf;//trigEvent.getObjects()[Key-2].pt(); // and -2 for MHT significance
      // significance  saved as .pt() but obviously pt holds no meaning here
    }

    // HLT PFMHT
    if (Tag=="hltPFMHTTightID::HLT"){
      met_map["HLTPFMHT"]      = objpt;//trigEvent.getObjects()[Key-4].pt();
      met_map["HLTPFMHT_phi"]  = objphi;//trigEvent.getObjects()[Key-4].phi();
      met_map["HLTPFMHT_sigf"] = objsigf;//trigEvent.getObjects()[Key-2].pt();
    }

    //HLT PFMET
    if (Tag=="hltPFMETProducer::HLT"){
      met_map["HLTPFMET"]      = objpt;//trigEvent.getObjects()[Key-4].pt();
      met_map["HLTPFMET_phi"]  = objphi;//trigEvent.getObjects()[Key-4].phi();
      met_map["HLTPFMET_sigf"] = objsigf;//trigEvent.getObjects()[Key-2].pt();
    }

  }
  return met_map;
}

//===================== MINIAOD HLT Trigger Summary ===================
std::map<std::string, float> trigtools::getHLTMETOjects(const pat::TriggerObjectStandAloneCollection& triggerObjects)
{
  std::map<std::string, float> met_map = {
    {"HLTCaloMET",      -10}, {"HLTCaloMET_phi",      -10}, {"HLTCaloMET_sigf",      -10},
    {"HLTCaloMETClean", -10}, {"HLTCaloMETClean_phi", -10}, {"HLTCaloMETClean_sigf", -10},
    {"HLTCaloMHT",      -10}, {"HLTCaloMHT_phi",      -10}, {"HLTCaloMHT_sigf",      -10},
    {"HLTPFMHT",        -10}, {"HLTPFMHT_phi",        -10}, {"HLTPFMHT_sigf",        -10},
    {"HLTPFMET",        -10}, {"HLTPFMET_phi",        -10}, {"HLTPFMET_sigf",        -10}
  };

  // loop over trigger object collections to find HLT CaloMET, CaloMETClean, CaloMHT, PFMHT, PFMET collections
  for (pat::TriggerObjectStandAlone obj : triggerObjects){

    //int Key = trigEvent.collectionKey(iC);
    std::string Tag = obj.collection();
    float objpt(obj.pt()), objphi(obj.phi());

    if (Tag == "hltMet::HLT"){
      met_map["HLTCaloMET"]      = objpt;
      met_map["HLTCaloMET_phi"]  = objphi;
      //met_map["HLTCaloMET_sigf"] = ???
    }

    // HLT CaloMETClean
    if (Tag=="hltMetClean::HLT"){
      met_map["HLTCaloMETClean"]      = objpt;
      met_map["HLTCaloMETClean_phi"]  = objphi;
      //met_map["HLTCaloMETClean_sigf"] = ???
    }

    // HLT CaloMHT
    if (Tag=="hltMht::HLT"){
      met_map["HLTCaloMHT"]      = objpt;
      met_map["HLTCaloMHT_phi"]  = objphi;
      //met_map["HLTCaloMHT_sigf"] = ???
    }

    // HLT PFMHT
    if (Tag=="hltPFMHTTightID::HLT"){
      met_map["HLTPFMHT"]      = objpt;
      met_map["HLTPFMHT_phi"]  = objphi;
      //met_map["HLTPFMHT_sigf"] = ???
    }

    //HLT PFMET
    if (Tag=="hltPFMETProducer::HLT"){
      met_map["HLTPFMET"]      = objpt;
      met_map["HLTPFMET_phi"]  = objphi;
      //met_map["HLTPFMET_sigf"] = ???
    }

  }
  return met_map;
}