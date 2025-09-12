// system include files
#include <memory>
#include <vector>
#include <ctime>
#include <chrono>

#include "TMatrix.h"
#include "TLorentzVector.h"

// user include files
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "DataFormats/TrackReco/interface/DeDxHitInfo.h"
#include "DataFormats/TrackReco/interface/DeDxData.h"
#include "DataFormats/TrackerCommon/interface/TrackerTopology.h"
#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonDetUnit/interface/PixelGeomDetUnit.h"
#include "RecoLocalTracker/ClusterParameterEstimator/interface/PixelClusterParameterEstimator.h"

#include "SUSYBSMAnalysis/HSCP/interface/ClusterCleaning.h"
#include "SUSYBSMAnalysis/Analyzer/interface/SaturationCorrectionInStrip.h"

//
// class declaration
//

class HSCPDeDxTool {
public:
  explicit HSCPDeDxTool(const edm::Event &iEvent, const reco::DeDxHitInfo* dedxHits, const pat::PackedCandidateRef track, const TrackerTopology* tkTopo = nullptr, const TrackerGeometry* tkGeometry = nullptr);
  explicit HSCPDeDxTool(const edm::Event &iEvent, const reco::DeDxHitInfo* dedxHits, const reco::TrackRef track, const TrackerTopology* tkTopo = nullptr, const TrackerGeometry* tkGeometry = nullptr);
  virtual ~HSCPDeDxTool();

  //void process();

  float harmonicMean(std::vector<float> v, float expo = -2);
  float truncatedMean(std::vector<float> v, float rate = 0.40);
  float discriminatorIas(std::vector<float> v, bool symmetricSmirnov = false);
  unsigned int numberOfSaturatedClusters();
  std::vector<int> Convert(const std::vector<unsigned char>& input);
  std::vector<uint16_t> CrossTalkInv(const std::vector<uint16_t>& Q,
                                const float x1 = 0.10,
                                const float x2 = 0.04,
                                bool way = true,
                                float threshold = 20,
                                float thresholdSat = 25,
                                bool isClusterCleaning = false);
  std::vector<uint16_t> SaturationCorrection(const std::vector<uint16_t>&  Q, 
                                        const float x1, 
                                        const float x2, 
                                        bool way,
                                        float threshold,
                                        float thresholdSat);
  bool isHitInsideTkModule(const LocalPoint hitPos, const DetId& detid, const SiStripCluster* cluster = nullptr);

  void computedEdx(float* scaleFactors, 
                   bool useClusterCleaning = true,
                   bool mustBeInside = false, 
                   int crossTalkInvAlgo = 0, 
                   float dropLowerDeDxValue = 0.0,
                   TH3* templateHisto = nullptr,
                   bool addExtraDeDxEstimators = false);

  // Harmonic Mean
  reco::DeDxData dedxDataFullTracker() {return dedxDataFullTracker_;}
  reco::DeDxData dedxDataFullTrackerNoL1() {return dedxDataFullTrackerNoL1_;}
  reco::DeDxData dedxDataPixelOnly() {return dedxDataPixel_;}
  reco::DeDxData dedxDataPixelOnlyNoL1() {return dedxDataPixelNoL1_;}
  reco::DeDxData dedxDataStripOnly() {return dedxDataStrip_;}

  // Truncated Mean
  reco::DeDxData dedxTruncFullTracker() {return dedxTruncFullTracker_;}
  reco::DeDxData dedxTruncStripOnly() {return dedxTruncStrip_;}

  reco::DeDxData dedxIasFullTracker() {return dedxIasFullTracker_;}
  reco::DeDxData dedxIasFullTrackerNoL1() {return dedxIasFullTrackerNoL1_;}
  reco::DeDxData dedxIasPixelOnly() {return dedxIasPixel_;}
  reco::DeDxData dedxIasPixelOnlyNoL1() {return dedxIasPixelNoL1_;}
  reco::DeDxData dedxIasStripOnly() {return dedxIasStrip_;}

  //Extra Estimators
  std::vector<reco::DeDxData> dedxDataStripExtra() {return dedxDataStripExtra_;}
  std::vector<reco::DeDxData> dedxTruncStripExtra() {return dedxTruncStripExtra_;}

  std::vector<float> clusterCharge() {return clusterCharge_;}
  std::vector<uint32_t> clusterDetId() {return clusterDetId_;}
  std::vector<uint32_t> clusterSaturatingStrips() {return clusterSaturatingStrips_;}
  std::vector<float> clusterDeDxStrip() {return clusterDeDxStrip_;}

  void computeProbQ(const PixelClusterParameterEstimator* pixelCPE);
  float combineProbs(float probOnTrackWMulti, int numRecHits);

  float probQonTrack() {return probQonTrack_;}
  float probXYonTrack() {return probXYonTrack_;}
  float probQonTrackNoL1() {return probQonTrackNoL1_;}
  float probXYonTrackNoL1() {return probXYonTrackNoL1_;}

  uint32_t numberOfPixelNoL1Measurement() {return nonL1PixHits_;}


private:
  unsigned long long eventNumber_    =  0ULL;
  uint32_t runNumber_                =  {0};
  const reco::DeDxHitInfo* dedxHits_ = nullptr;
  const TrackerTopology* tkTopo_     = nullptr;
  const TrackerGeometry* tkGeometry_ = nullptr;
  float track_eta        =  0;
  float track_pt         =  0;
  float track_px         =  0;
  float track_py         =  0;
  float track_pz         =  0;
  int track_charge       =  0;

  TH3* templateHisto_    = nullptr;

  reco::DeDxData  dedxDataFullTracker_        = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxDataFullTrackerNoL1_    = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxDataPixel_              = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxDataPixelNoL1_          = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxDataStrip_              = reco::DeDxData(-1, -1, -1);
  //
  reco::DeDxData dedxTruncFullTracker_       = reco::DeDxData(-1, -1, -1);
  reco::DeDxData dedxTruncStrip_             = reco::DeDxData(-1, -1, -1);
  //
  reco::DeDxData  dedxIasFullTracker_        = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxIasFullTrackerNoL1_    = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxIasPixel_              = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxIasPixelNoL1_          = reco::DeDxData(-1, -1, -1);
  reco::DeDxData  dedxIasStrip_              = reco::DeDxData(-1, -1, -1);
  //bool isStrip_;
  //bool isPixel_;
  //bool useClusterCleaning_;
  //bool mustBeInside_;
  //int crossTalkInvAlgo_;
  //float dropLowerDeDxValue_;

  //Extra Estimators
  std::vector<reco::DeDxData> dedxDataStripExtra_ = {reco::DeDxData(-1, -1, -1)};
  std::vector<reco::DeDxData> dedxTruncStripExtra_= {reco::DeDxData(-1, -1, -1)};

  //saveDeDxHitInfo
  std::vector<float> clusterCharge_;
  std::vector<uint32_t> clusterDetId_;
  std::vector<uint32_t> clusterSaturatingStrips_;
  std::vector<float> clusterDeDxStrip_;

  float probQonTrack_ = -1.f;
  float probXYonTrack_ = -1.f;
  float probQonTrackNoL1_ = -1.f;
  float probXYonTrackNoL1_ = -1.f;

  int nonL1PixHits_ = 0;

  float factorChargeToE_[2] = {3.61e-06, 3.61e-06 * 265};

  const float TkModGeomThickness[15] = {1,
                                     0.029000,
                                     0.029000,
                                     0.047000,
                                     0.047000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.029000,
                                     0.047000,
                                     0.047000,
                                     0.047000};
  const float TkModGeomLength[15] = {1,
                                  5.844250,
                                  5.844250,
                                  9.306700,
                                  9.306700,
                                  5.542900,
                                  4.408000,
                                  5.533000,
                                  4.258000,
                                  4.408000,
                                  5.533000,
                                  5.758000,
                                  7.363125,
                                  9.204400,
                                  10.235775};
  const float TkModGeomWidthB[15] = {1,
                                  3.072000,
                                  3.072000,
                                  4.684800,
                                  4.684800,
                                  4.579445,
                                  5.502407,
                                  3.158509,
                                  4.286362,
                                  5.502407,
                                  4.049915,
                                  3.561019,
                                  6.002559,
                                  5.235483,
                                  3.574395};
  const float TkModGeomWidthT[15] = {1,
                                  3.072000,
                                  3.072000,
                                  4.684800,
                                  4.684800,
                                  3.095721,
                                  4.322593,
                                  4.049915,
                                  3.146580,
                                  4.322593,
                                  3.158509,
                                  2.898798,
                                  4.824683,
                                  4.177638,
                                  4.398049};
};


// ------------ HSCPDeDxTool  ------------

HSCPDeDxTool::HSCPDeDxTool(const edm::Event &iEvent, const reco::DeDxHitInfo* dedxHits, const pat::PackedCandidateRef track, const TrackerTopology* tkTopo, const TrackerGeometry* tkGeometry) {
  eventNumber_ = iEvent.id().event();
  runNumber_   = iEvent.run();
  dedxHits_    = dedxHits;
  tkTopo_      = tkTopo;
  tkGeometry_  = tkGeometry;
  track_eta    = track->eta();
  track_pt     = track->pt();
  track_px     = track->px();
  track_py     = track->py();
  track_pz     = track->pz();
  track_charge = track->charge();
}
HSCPDeDxTool::HSCPDeDxTool(const edm::Event &iEvent, const reco::DeDxHitInfo* dedxHits, const reco::TrackRef track, const TrackerTopology* tkTopo, const TrackerGeometry* tkGeometry) {
  eventNumber_ = iEvent.id().event();
  runNumber_   = iEvent.run();
  dedxHits_    = dedxHits;
  tkTopo_      = tkTopo;
  tkGeometry_  = tkGeometry;
  track_eta    = track->eta();
  track_pt     = track->pt();
  track_px     = track->px();
  track_py     = track->py();
  track_pz     = track->pz();
  track_charge = track->charge();
}

HSCPDeDxTool::~HSCPDeDxTool() {}

//
// member functions
//

//void HSCPDeDxTool::process(){
void HSCPDeDxTool::computedEdx(float* scaleFactors, 
                               bool useClusterCleaning,
                               bool mustBeInside,
                               int crossTalkInvAlgo,
                               float dropLowerDeDxValue,
                               TH3* templateHisto,
                               bool addExtraDeDxEstimators)
{

  if (!dedxHits_){
    return;
  }bool correctFEDSat = false;

  std::vector<float> dedxFullTracker, dedxFullTracker_noL1;
  std::vector<float> dedxStrip;
  std::vector<float> dedxPixel, dedxPixel_noL1;
  std::vector<float> dedxIasFullTracker, dedxIasFullTracker_noL1;
  std::vector<float> dedxIasStrip;
  std::vector<float> dedxIasPixel, dedxIasPixel_noL1;
  std::vector<float> debug_ClusterCharge, debug_pathlength;
  std::vector<int> debug_detid;

  dedxDataStripExtra_.clear();
  dedxTruncStripExtra_.clear();

  clusterCharge_.clear();
  clusterDetId_.clear();

  size_t MaxStripNOM = 99;
  unsigned int NSat = 0;
  unsigned int SiStripNOM = 0;
  bool passedClusterCleaning = false;

  //unsigned int nsatclust = numberOfSaturatedClusters();
  //\\float rsat = (float)nsatclust / (float)dedxHits_->size();

  //std::cout<<" [+] Event "<<eventNumber_<<": Input dedxHits size = "<< dedxHits_->size()<<std::endl; //EMERY//

  float scaleFactor = 1.;
  for (unsigned int h = 0; h < dedxHits_->size(); h++){
    DetId detid(dedxHits_->detId(h));
    bool isPixel =  (detid.subdetId() < 3);
    bool isStrip = (detid.subdetId() >= 3);

    scaleFactor = (isPixel) ? factorChargeToE_[0]: factorChargeToE_[1];
    scaleFactor *= (isPixel) ? scaleFactors[1]: scaleFactors[0];
    //float pixelScaling = GetSFPixel(detid.subdetId(), detid, year, run_number);//TBC

    clusterCharge_.push_back(dedxHits_->charge(h));
    clusterDetId_.push_back(detid.subdetId());

    // Corrected cluster charge per path length
    uint16_t ClusterCharge = 0;
    float dedx = scaleFactor * dedxHits_->charge(h) / dedxHits_->pathlength(h);

    bool isHitInside = isHitInsideTkModule(dedxHits_->pos(h), detid, (isStrip) ? dedxHits_->stripCluster(h) : nullptr);

    if (mustBeInside && !isHitInside)
      continue;

    //if(isStrip) std::cout<<"\t [+] "<<h<<"th hit is inside the tracker "<<std::endl; //EMERY//

    if (isStrip && ++SiStripNOM > MaxStripNOM)
      continue;  // skip remaining strips, but not pixel

    //if(isStrip) std::cout<<"\t [+] "<<h<<"th hit passes MaxStripNOM cut "<<std::endl; //EMERY//

    bool isPixelL1 = ( detid.subdetId() == 1 && abs(int(tkTopo_->pxbLayer(detid))) == 1 );

    dedxFullTracker.push_back(dedx);
    if(!isPixelL1) dedxFullTracker_noL1.push_back(dedx);

    //TESTME//if (skipPixelL1 && isPixelL1) continue;

    //_____________________________________________________
    if (isStrip){

      if(fabs(track_eta) < 1.0 && !(detid.subdetId() == 3 || detid.subdetId() == 5)) 
        continue; // eta < 1.0 -> only TIB+TOB hits
      if(fabs(track_eta) > 1.0 && fabs(track_eta) < 1.7 && !(detid.subdetId() == 3 || detid.subdetId() == 4 || detid.subdetId() == 6)) 
        continue; // 1.0 < eta < 1.7 -> only TIB+TID+TEC hits
      if(fabs(track_eta) > 1.7 && !(detid.subdetId() == 4 || detid.subdetId() == 6)) 
        continue; // eta > 1.

      //std::cout<<"\t [+] "<<h<<"th hit passes geometry cut "<<std::endl; //EMERY//

      //SiStripDetId Sdetid(dedxHits_->detId(h));
      //const SiStripCluster* cluster = &*(dedxHits_->stripCluster(h));
      //std::vector<int> amplitudes(cluster->amplitudes().begin(), cluster->amplitudes().end());//EMERY FOUND BUG

      const SiStripCluster* cluster = dedxHits_->stripCluster(h);
      std::vector<uint16_t> amplitudes(cluster->amplitudes().begin(), cluster->amplitudes().end()); //= Convert(cluster->amplitudes());
      //UNUSED//std::vector<uint16_t> amplitudesPrim = CrossTalkInv(amplitudes,0.10,0.04,true);

      int layer = 0;
      if (detid.subdetId() == StripSubdetector::TIB) layer= abs(int(tkTopo_->tibLayer(detid)));
      if (detid.subdetId() == StripSubdetector::TOB) layer= abs(int(tkTopo_->tobLayer(detid))) + 4;
      if (detid.subdetId() == StripSubdetector::TID) layer= abs(int(tkTopo_->tidWheel(detid))) + 10;
      if (detid.subdetId() == StripSubdetector::TEC) layer= abs(int(tkTopo_->tecWheel(detid))) + 13;
      std::vector <uint16_t> amplitudesPrim = CrossTalkInvInStrip(amplitudes, layer, true, 20, 0.10, 0.04);

      /*std::cout<<"\t\t [+] Testing nominal amplitudes [unsigned char]:"<<std::endl; //EMERY//
      for(unsigned int i=0;i<amplitudes.size();i++) std::cout << "\t\t\t"<<i<<": "<<amplitudes[i]<<std::endl;//EMERY
      std::cout<<"\t\t [+] Testing converted amplitudes:"<<std::endl; //EMERY//
      for(unsigned int i=0;i<amplitudesPrim.size();i++) std::cout << "\t\t\t"<<i<<": "<<amplitudesPrim[i]<<std::endl;//EMERY*/

      if (useClusterCleaning){
        ClusterCleaning* cc = new ClusterCleaning(amplitudesPrim);
        int crosstalkInv = 1;
        passedClusterCleaning = cc->passClusterCleaning(crosstalkInv);
        if (!passedClusterCleaning) continue;
      }
      //if (useClusterCleaning&&!passedClusterCleaning) continue;

      //std::cout<<"\t [+] "<<h<<"th hit passes ClusterCleaning cut "<<std::endl; //EMERY//

      //////////////////////////////////////////////////////////////
      //
      //     crossTalkInvAlgo == 0: no correction & no cross-talk inversion
      //     crossTalkInvAlgo == 1: standard correction & use of cross-talk inversion
      //     crossTalkInvAlgo == 2: correction from fits & no cross-talk inversion
      //     crossTalkInvAlgo == 3: correction from fits & use of cross-talk inversion (no recorrection -- see bool=false)
      //     crossTalkInvAlgo == 4: Saturation correction using neighbourhood strip information
      //
      //////////////////////////////////////////////////////////////

      //DEBUG-------------------------------------------------
      //UNUSED//amplitudes = SaturationCorrection(amplitudes,0.10,0.04,true,20,25);
      bool totrash = true;
      amplitudes = ReturnCorrVec(amplitudes, layer, totrash);// (crossTalkInvAlgo == 4)
      
      float gain = 1.0;
      bool isSatCluster = false;
      ClusterCharge = 0;
      for (unsigned int s = 0; s < amplitudes.size(); s++){
        uint16_t StripCharge = amplitudes[s];
        if (StripCharge < 254){
          StripCharge = (uint16_t)(StripCharge / gain);
          if (StripCharge >= 1024){
            StripCharge = 255;
          }
          else if (StripCharge >= 254){
            StripCharge = 254;
          }
        }
        if (StripCharge >= 254) isSatCluster = true;
        if (StripCharge >= 255 && correctFEDSat) StripCharge = 512;
        ClusterCharge += StripCharge;
      }
      if (isSatCluster) NSat++;

      dedxStrip.push_back(scaleFactor * ClusterCharge / dedxHits_->pathlength(h));
      debug_ClusterCharge.push_back(ClusterCharge); 
      debug_pathlength.push_back(dedxHits_->pathlength(h));
      debug_detid.push_back(detid.subdetId());
      
    }
    else if (isPixel){
      dedxPixel.push_back(dedx);
      if(!isPixelL1) dedxPixel_noL1.push_back(dedx);
    }

    // Discriminator probability
    if (templateHisto){
      //float ChargeOverPathlength = scaleFactor * ClusterCharge / (dedxHits_->pathlength(h) * 10.0 * (isPixel ? 265 : 1));
      float SF = (isPixel) ? scaleFactors[1] * 265 : scaleFactors[0];
      float ChargeOverPathlength = SF * ClusterCharge / (dedxHits_->pathlength(h) * 10.0);

      int moduleGeometry = (isPixel) ? 15 : static_cast<unsigned int>(SiStripDetId(detid).moduleGeometry());

      int layer = 0;
      if (detid.subdetId() == StripSubdetector::TIB) layer= abs(int(tkTopo_->tibLayer(detid)));
      if (detid.subdetId() == StripSubdetector::TOB) layer= abs(int(tkTopo_->tobLayer(detid))) + 4;
      if (detid.subdetId() == StripSubdetector::TID) layer= abs(int(tkTopo_->tidWheel(detid))) + 10;
      if (detid.subdetId() == StripSubdetector::TEC) layer= abs(int(tkTopo_->tecWheel(detid))) + 13;

      int  skip_templates_ias = 0;
      //skip templates ias = 1 --> skip pixel, TIB, TID, 3 first TEC layers
      if (skip_templates_ias == 1 && (
                     detid.subdetId()<5 ||
                     layer == 14 ||
                     layer == 15 ||
                     layer == 16
                  )
        ){continue;}

      //skip templates ias = 2 --> pixel only, with pixL1 or not
      bool isBPIXL1=false;
      int numLayers = tkGeometry_->numberOfLayers(PixelSubdetector::PixelBarrel);
      if ((numLayers == 4) && ((detid.subdetId() == PixelSubdetector::PixelBarrel) && (tkTopo_->pxbLayer(detid) == 1))) isBPIXL1=true;  // only for 2017 and 2018
      if (skip_templates_ias == 2 && (
                  detid.subdetId()>2 ||
                  isBPIXL1 
                  //(skipPixelL1 && detid.subdetId() == 1 && ((detid >> 16) & 0xF) == 1) //decoding mask for 2016 !
                  //(skipPixelL1 && detid.subdetId() == 1 && ((detid >> 20) & 0xF) == 1) //decoding mask for 2017-2018
                  )
        ){continue;}

      int BinX = templateHisto->GetXaxis()->FindBin(moduleGeometry);
      bool useTemplateLayer(false);
      if (useTemplateLayer) BinX = templateHisto->GetXaxis()->FindBin(layer);
      int BinY = templateHisto->GetYaxis()->FindBin(dedxHits_->pathlength(h) * 10.0);  //*10 because of cm-->mm
      int BinZ = templateHisto->GetZaxis()->FindBin(ChargeOverPathlength);
      float Prob = templateHisto->GetBinContent(BinX, BinY, BinZ);
      //std::cout<<"BinX = "<<BinX<<", BinY = "<<BinY<<", BinZ = "<<BinZ<<" >> Prob = "<<Prob<<std::endl;
      dedxIasFullTracker.push_back(Prob);
      if (isStrip) dedxIasStrip.push_back(Prob);
    }

  }

  dedxDataFullTracker_     = reco::DeDxData(harmonicMean(dedxFullTracker), NSat, dedxFullTracker.size());
  dedxDataFullTrackerNoL1_ = reco::DeDxData(harmonicMean(dedxFullTracker_noL1), NSat, dedxFullTracker_noL1.size());
  //
  dedxDataPixel_     = reco::DeDxData(harmonicMean(dedxPixel), NSat, dedxPixel.size());
  dedxDataPixelNoL1_ = reco::DeDxData(harmonicMean(dedxPixel_noL1), NSat, dedxPixel_noL1.size());
  //
  dedxDataStrip_     = reco::DeDxData(harmonicMean(dedxStrip), NSat, dedxStrip.size());
  clusterDeDxStrip_  = dedxStrip;

  dedxTruncFullTracker_    = reco::DeDxData(truncatedMean(dedxFullTracker), NSat, dedxFullTracker.size());
  dedxTruncStrip_           = reco::DeDxData(truncatedMean(dedxStrip), NSat, dedxStrip.size());

  dedxIasFullTracker_     = reco::DeDxData(discriminatorIas(dedxIasFullTracker), NSat, dedxIasFullTracker.size());
  dedxIasFullTrackerNoL1_ = reco::DeDxData(discriminatorIas(dedxFullTracker_noL1), NSat, dedxFullTracker_noL1.size());
  //
  dedxIasPixel_     = reco::DeDxData(discriminatorIas(dedxPixel), NSat, dedxPixel.size());
  dedxIasPixelNoL1_ = reco::DeDxData(discriminatorIas(dedxPixel_noL1), NSat, dedxPixel_noL1.size());
  //
  dedxIasStrip_     = reco::DeDxData(discriminatorIas(dedxIasStrip), NSat, dedxIasStrip.size());

  if (addExtraDeDxEstimators){
    for(int i=0; i<=35; i+=5)
      dedxTruncStripExtra_.push_back(reco::DeDxData(truncatedMean(dedxStrip,float(i/100.)), NSat, dedxStrip.size()));
    std::vector<int> powers = {1,3,4};
    for (auto p : powers)
      dedxDataStripExtra_.push_back(reco::DeDxData(harmonicMean(dedxStrip, float(-p)), NSat, dedxStrip.size()));
  }

  /*//if() {
    std::cout<<"Using harmonic mean of"<<std::endl;
    for(unsigned int i = 0; i < dedxStrip.size(); i++){
      auto timenow =  std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      char buf[100] = {0};
      std::strftime(buf, sizeof(buf), "%Y-%m-%d %T", std::localtime(&timenow));
      //std::cout<<"\t"<<"dedx["<<i<<"]: "<<dedxStrip[i]<<", detid="<<debug_detid[i] <<"\t (Q="<<debug_ClusterCharge[i]<<", pathlength="<<debug_pathlength[i]<<")"<<std::endl;
      std::cout<<"\t"<<buf<<"| dedx["<<i<<"]: "<<dedxStrip[i]<<"\t (Q="<<debug_ClusterCharge[i]<<", pathlength="<<debug_pathlength[i]<<")"<<std::endl;
    }
    std::cout<<"EMERY::DeDx="<<harmonicMean(dedxStrip)//<<"\t useClusterCleaning="<<useClusterCleaning<<", passedClusterCleaning="<<passedClusterCleaning<<")"<<std::endl;
    //std::cout
    <<"\tTrack: pt="<<track_pt<<", track_eta="<<track_eta<<std::endl;
  //}*/

}


float HSCPDeDxTool::harmonicMean(std::vector<float> v, float expo){
  int size = v.size();
  float result = 0.0;

  for (int i = 0; i < size; i++) {
    result += pow(v[i], expo);
  }
  result = (size > 0) ? pow(result / size, 1. / expo) : -1.0;

  return result;
}

float HSCPDeDxTool::truncatedMean(std::vector<float> v, float rate){
  int size = v.size();
  float result = 0.0;

  std::sort(v.begin(), v.end(), std::less<float>());
  int nTrunc = size * rate;
  for (int i = 0; i + nTrunc < size; i++) {
    result += v[i];
  }
  result /= (size - nTrunc);

  return (size > 0) ? result : -1.0;
}

float HSCPDeDxTool::discriminatorIas(std::vector<float> v, bool symmetricSmirnov){
  int size = v.size();
  float result = 1.0 / (12 * size);

  std::sort(v.begin(), v.end(), std::less<float>());

  for (int i = 1; i <= size; i++) {
    if(!symmetricSmirnov) result += v[i - 1] * pow(v[i - 1] - ((2.0 * i - 1.0) / (2.0 * size)), 2); //Ias
    else result += pow(v[i - 1] - ((2.0 * i - 1.0) / (2.0 * size)), 2); //Is
  }
  result *= (3.0 / size);

  return result;

  /*float alpha = 1;
  for (int i = 0; i < size; i++){
    alpha *= v[i];
  }
  float logAlpha = log(alpha);
  float probQm = 0;
  for (int i = 0; i < size; i++){
    probQm += ((pow(-logAlpha, i)) / (factorial(i)));
  }
  return alpha * probQm;*/
}


// number of saturated clusters in a track
unsigned int HSCPDeDxTool::numberOfSaturatedClusters(){
  clusterSaturatingStrips_.clear();
  unsigned int nsatclust = 0;
  for (unsigned int t = 0; t < dedxHits_->size(); t++) {
    DetId detid(dedxHits_->detId(t));
    bool test_sat = false;
    if (detid.subdetId() < 3){
      clusterSaturatingStrips_.push_back(0);
      continue;
    }
    const SiStripCluster* cluster = dedxHits_->stripCluster(t);
    const std::vector<uint16_t> amplitudes(cluster->amplitudes().begin(), cluster->amplitudes().end());
    unsigned int nsat_strips(0);
    for (unsigned int s = 0; s < amplitudes.size(); s++) {
      if (amplitudes[s] > 253){
        test_sat = true;
        nsat_strips++;
      }
    }
    if (test_sat)
      nsatclust++;

    clusterSaturatingStrips_.push_back(nsat_strips);
  }
  return nsatclust;
}

std::vector<int> HSCPDeDxTool::Convert(const std::vector<unsigned char>& input) {
  std::vector<int> output;
  for (unsigned int i = 0; i < input.size(); i++) {
    output.push_back((int)input[i]);
  }
  return output;
}

std::vector<uint16_t> HSCPDeDxTool::CrossTalkInv(const std::vector<uint16_t>& Q,
                                const float x1,
                                const float x2,
                                bool way,
                                float threshold,
                                float thresholdSat,
                                bool isClusterCleaning)
{
  const unsigned N = Q.size();
  std::vector<uint16_t> QII;
  std::vector<float> QI(N, 0);
  Double_t a = 1 - 2 * x1 - 2 * x2;
  //  bool debugbool=false;
  TMatrix A(N, N);

  //std::cout<<"\t\t A: Q.size() = "<<Q.size()<<std::endl;
  //---
  if (Q.size() < 2 || Q.size() > 8) {
    for (unsigned int i = 0; i < Q.size(); i++) {
      QII.push_back((uint16_t)Q[i]);
    }
    return QII;
  }

  if(way){
      std::vector<uint16_t>::const_iterator mQ = max_element(Q.begin(), Q.end()) ;
      if(*mQ>253){
         //std::cout << "\t\t Max index: "<< std::distance(Q.begin(), mQ) <<" Q(max)="<<*mQ << ", Q(max-1)=" << *(mQ-1)<< ", Q(max+1)=" << *(mQ+1)  << std::endl;
         if(*mQ==255 && *(mQ-1)>253 && *(mQ+1)>253 ) return Q ;
         if(*(mQ-1)>thresholdSat && *(mQ+1)>thresholdSat && *(mQ-1)<254 && *(mQ+1)<254 &&  abs(*(mQ-1) - *(mQ+1)) < 40 ){
             QII.push_back((10*(*(mQ-1))+10*(*(mQ+1)))/2); return QII;}
      }
   }
  //---

  for (unsigned int i = 0; i < N; i++) {
    A(i, i) = a;
    if (i < N - 1) {
      A(i + 1, i) = x1;
      A(i, i + 1) = x1;
    } else
      continue;
    if (i < N - 2) {
      A(i + 2, i) = x2;
      A(i, i + 2) = x2;
    }
  }

  if (N == 1)
    A(0, 0) = 1 / a;
  else
    A.InvertFast();

  for (unsigned int i = 0; i < N; i++) {
    for (unsigned int j = 0; j < N; j++) {
      QI[i] += A(i, j) * (float)Q[j];
    }
  }

  for (unsigned int i = 0; i < QI.size(); i++) {
    if (QI[i] < threshold)
      QI[i] = 0;
    QII.push_back((uint16_t)QI[i]);
  }

  return QII;
}


std::vector<uint16_t> HSCPDeDxTool::SaturationCorrection(const std::vector<uint16_t>&  Q, const float x1, const float x2, bool way,float threshold,float thresholdSat) {
  const unsigned N=Q.size();
  std::vector<uint16_t> QII;
  std::vector<float> QI(N,0);

//---  only for one max well-defined
 if(Q.size()<2 || Q.size()>8){
        for (unsigned int i=0;i<Q.size();i++){
                QII.push_back((uint16_t) Q[i]);
        }
        return QII;
  }
 if(way){
          std::vector<uint16_t>::const_iterator mQ = max_element(Q.begin(), Q.end())      ;
          if(*mQ>253){
                 if(*mQ==255 && *(mQ-1)>253 && *(mQ+1)>253 ) return Q ;
                 if(*(mQ-1)>thresholdSat && *(mQ+1)>thresholdSat && *(mQ-1)<254 && *(mQ+1)<254 &&  abs(*(mQ-1) - *(mQ+1)) < 40 ){
                     QII.push_back((10*(*(mQ-1))+10*(*(mQ+1)))/2); return QII;}
          }
      else{
          return Q; // no saturation --> no x-talk inversion
      }
  }
//---
 // do nothing else
 return Q;
}



bool HSCPDeDxTool::isHitInsideTkModule(const LocalPoint hitPos, const DetId& detid, const SiStripCluster* cluster) {
  if (detid.subdetId() < 3) {
    return true;
  }  //do nothing for pixel modules
  //SiStripDetId SSdetId(detid);
  auto moduleGeometry = static_cast<unsigned int>(SiStripDetId(detid).moduleGeometry()); //SSdetId.moduleGeometry();

  //clean along the apv lines
  if (cluster &&
      (cluster->firstStrip() % 128 == 0 || (cluster->firstStrip() + cluster->amplitudes().size() % 128 == 127)))
    return false;

  float nx, ny;
  if (moduleGeometry <= 4) {
    ny = hitPos.y() / TkModGeomLength[moduleGeometry];
    nx = hitPos.x() / TkModGeomWidthT[moduleGeometry];
  } else {
    float offset =
        TkModGeomLength[moduleGeometry] * (TkModGeomWidthT[moduleGeometry] + TkModGeomWidthB[moduleGeometry]) /
        (TkModGeomWidthT[moduleGeometry] -
         TkModGeomWidthB[moduleGeometry]);  // check sign if GeomWidthT[moduleGeometry] < TkModGeomWidthB[moduleGeometry] !!!
    float tan_a = TkModGeomWidthT[moduleGeometry] / std::abs(offset + TkModGeomLength[moduleGeometry]);
    ny = hitPos.y() / TkModGeomLength[moduleGeometry];
    nx = hitPos.x() / (tan_a * std::abs(hitPos.y() + offset));
  }

  // "blacklists" for the gaps and edges
  switch (moduleGeometry) {
    case 0:
      return true;
    case 1:
      if (fabs(ny) > 0.96 || fabs(nx) > 0.98)
        return false;
      break;
    case 2:
      if (fabs(ny) > 0.97 || fabs(nx) > 0.99)
        return false;
      break;
    case 3:
      if (fabs(ny) > 0.98 || fabs(nx) > 0.98 || fabs(ny) < 0.04)
        return false;
      break;
    case 4:
      if (fabs(ny) > 0.98 || fabs(nx) > 0.98 || fabs(ny) < 0.04)
        return false;
      break;
    case 5:
      if (fabs(ny) > 0.98 || fabs(nx) > 0.98)
        return false;
      break;
    case 6:
      if (fabs(ny) > 0.98 || fabs(nx) > 0.99)
        return false;
      break;
    case 7:
      if (fabs(ny) > 0.97 || fabs(nx) > 0.98)
        return false;
      break;
    case 8:
      if (fabs(ny) > 0.98 || fabs(nx) > 0.99)
        return false;
      break;
    case 9:
      if (fabs(ny) > 0.98 || fabs(nx) > 0.99)
        return false;
      break;
    case 10:
      if (fabs(ny) > 0.97 || fabs(nx) > 0.99)
        return false;
      break;
    case 11:
      if (fabs(ny) > 0.97 || fabs(nx) > 0.99)
        return false;
      break;
    case 12:
      if (fabs(ny) > 0.98 || fabs(nx) > 0.99 || (-0.17 < ny && ny < -0.07))
        return false;
      break;
    case 13:
      if (fabs(ny) > 0.97 || fabs(nx) > 0.99 || (-0.10 < ny && ny < -0.01))
        return false;
      break;
    case 14:
      if (fabs(ny) > 0.95 || fabs(nx) > 0.98 || (0.01 < ny && ny < 0.12))
        return false;
      break;
    default:
      std::cerr << "Unknown module geometry! Exiting!" << std::endl;
      exit(EXIT_FAILURE);
  }

  return true;
}

void HSCPDeDxTool::computeProbQ(const PixelClusterParameterEstimator* pixelCPE)
{
  if (!dedxHits_){
    return;
  }

  int numLayers = tkGeometry_->numberOfLayers(PixelSubdetector::PixelBarrel);
  int numRecHitsQ = 0, numRecHitsXY = 0;
  int numRecHitsQNoL1 = 0, numRecHitsXYNoL1 = 0;
  float probQonTrackWMulti = 1;
  float probXYonTrackWMulti = 1;
  float probQonTrackWMultiNoL1 = 1;
  float probXYonTrackWMultiNoL1 = 1;
  
  unsigned int nonL1PixHits = 0;

  for (unsigned int i = 0; i < dedxHits_->size(); i++){
    DetId detid(dedxHits_->detId(i));
    bool isPixel =  (detid.subdetId() < 3);
    bool isStrip = (detid.subdetId() >= 3);

    if (isStrip)
      continue;

    if (isPixel){
      // Calculate probQ and probXY for this pixel rechit
      // Taking the pixel cluster
      auto const* pixelCluster =  dedxHits_->pixelCluster(i);
      if (pixelCluster == nullptr)
        continue;
      if (tkGeometry_ == nullptr) {
        //edm::LogPrint("GeometryUnit") << "TrackerGeometry is empty";
        continue;
      }
      // Check on which geometry unit the hit is
      //const GeomDetUnit& geomDet = *tkGeometry_->idToDetUnit(detid);
      //const GeomDetUnit* geomDet = nullptr;
      const GeomDetUnit* geomDet = nullptr;
      try{
        //geomDet = tkGeometry_->idToDetUnit(detid);
        geomDet = dynamic_cast<const GeomDetUnit*>(tkGeometry_->idToDetUnit(detid));
      }
      catch (cms::Exception& e){
        edm::LogError("HSCPDeDxTool::computeProbQ") << "cms::Exception: " << e.explainSelf() << "\n";
        //if (geomDet==nullptr) std::cout<<"SKIP"<<std::endl;
        break;
      }
      // Get the local vector for the track direction
      LocalVector lv = geomDet->toLocal(GlobalVector(track_px, track_py, track_pz));
      // Re-run the CPE on this cluster with the lv above
      // getParameters will return std::tuple<LocalPoint, LocalError, SiPixelRecHitQuality::QualWordType>;
      // from this we pick the 2nd, the QualWordType
      auto reCPE = std::get<2>(pixelCPE->getParameters(*pixelCluster, *geomDet, LocalTrajectoryParameters(dedxHits_->pos(i), lv, track_charge)));
      // extract probQ and probXY from this
      float probQ = SiPixelRecHitQuality::thePacking.probabilityQ(reCPE);
      float probXY = SiPixelRecHitQuality::thePacking.probabilityXY(reCPE);

      // To measure how often the CPE fails
      bool cpeHasFailed = false;
      if (!SiPixelRecHitQuality::thePacking.hasFilledProb(reCPE))
        cpeHasFailed = true;

      if (cpeHasFailed) continue;
      if (probQ <= 0.0 || probQ >= 1.f) probQ = 1.f;
      if (probXY <= 0.0 || probXY >= 1.f) probXY = 0.f;


      bool isOnEdge = SiPixelRecHitQuality::thePacking.isOnEdge(reCPE);
      bool hasBadPixels = SiPixelRecHitQuality::thePacking.hasBadPixels(reCPE);
      bool spansTwoROCs = SiPixelRecHitQuality::thePacking.spansTwoROCs(reCPE);
      bool specInCPE = (isOnEdge || hasBadPixels || spansTwoROCs) ? true : false;

      if (!specInCPE && probQ < 0.8){
        numRecHitsQ++;
        // Calculate alpha term needed for the combination
        probQonTrackWMulti *= probQ;
        // Calculate alpha term needed for the combination
      }

      if (!specInCPE && probQ < 0.8 && probXY > 0.f){
        numRecHitsXY++;
        probXYonTrackWMulti *= probXY;
      }
      
      // Have a separate variable that excludes Layer 1
      // Layer 1 was very noisy in 2017/2018
      bool conditionForPhase0 = (numLayers == 3);
      bool conditionForPhase1 = ((numLayers == 4) && (( detid.subdetId() == PixelSubdetector::PixelEndcap) || ((detid.subdetId() == PixelSubdetector::PixelBarrel) && (tkTopo_->pxbLayer(detid) != 1))));
      if (conditionForPhase0 || conditionForPhase1){
        nonL1PixHits++;
        float probQNoL1 = SiPixelRecHitQuality::thePacking.probabilityQ(reCPE);
        float probXYNoL1 = SiPixelRecHitQuality::thePacking.probabilityXY(reCPE);

        if (probQNoL1 <= 0.0 || probQNoL1 >= 1.f) probQNoL1 = 1.f;
        if (probXYNoL1 <= 0.0 || probXYNoL1 >= 1.f) probXYNoL1 = 0.f;

        if (!specInCPE && probQ < 0.8){
          numRecHitsQNoL1++;
          // Calculate alpha term needed for the combination
          probQonTrackWMultiNoL1 *= probQNoL1;
        }
        if (!specInCPE && probQ < 0.8 && probXYNoL1 > 0.f){
          numRecHitsXYNoL1++;
          // Calculate alpha term needed for the combination
          probXYonTrackWMultiNoL1 *= probXYNoL1;
        }
      }
    }// end if on the pixel side

  }//end loop on dedx hits for the given track

  nonL1PixHits_ = nonL1PixHits;

  // Combine probQ-s into HSCP candidate (track) level quantity
  probQonTrack_ = combineProbs(probQonTrackWMulti, numRecHitsQ);
  probXYonTrack_ = combineProbs(probXYonTrackWMulti, numRecHitsXY);
  probQonTrackNoL1_ = combineProbs(probQonTrackWMultiNoL1, numRecHitsQNoL1);
  probXYonTrackNoL1_ = combineProbs(probXYonTrackWMultiNoL1, numRecHitsXYNoL1);
}


float HSCPDeDxTool::combineProbs(float probOnTrackWMulti, int numRecHits){
  float logprobOnTrackWMulti = (probOnTrackWMulti > 0) ? log(probOnTrackWMulti) : 0;
  float factQ = -logprobOnTrackWMulti;
  float probOnTrackTerm = 0.f;

  if (numRecHits == 1) {
    probOnTrackTerm = 1.f;
  } else if (numRecHits > 1) {
    probOnTrackTerm = 1.f + factQ;
    for (int iTkRh = 2; iTkRh < numRecHits; ++iTkRh) {
      factQ *= -logprobOnTrackWMulti / float(iTkRh);
      probOnTrackTerm += factQ;
    }
  }
  float probOnTrack = probOnTrackWMulti * probOnTrackTerm;

  return probOnTrack;
}


TH3F* loadDeDxTemplate(std::string path, bool splitByModuleType,bool puTreatment,int puBin){
  if (path.empty()) {
    //throw cms::Exception("HSCPDeDxTool::error::loadDeDxTemplate") << "Empty filename.";
    return nullptr;
  }
  std::unique_ptr<TFile> InputFile( TFile::Open(path.c_str()) );
  if (!InputFile || InputFile->IsZombie()) {
    throw cms::Exception("HSCPDeDxTool::error::loadDeDxTemplate") << "Can't open file '" << path << "'.";
  }
  //TFile* InputFile = new TFile(path.c_str());
  //std::unique_ptr<TH3> DeDxMap;
  TH3F* DeDxMap_;
  if(!puTreatment) {
     DeDxMap_ = (TH3F*)InputFile->Get("Calibration_GiTemplate"); // used to be Charge_Vs_Path 
  } else{
    if( puBin > 5) {
      printf("puBin > 5");
      exit(0);
    } else{  
      std::string template_name = "Calibration_GiTemplate_PU_" + std::to_string(puBin);
      DeDxMap_ = (TH3F*)InputFile->Get(template_name.c_str());
    }
  }

  if (!DeDxMap_) {
    throw cms::Exception("HSCPDeDxTool::error::loadDeDxTemplate") << "Can't open file '" << path << "'.";
  }

  TH3F* Prob_ChargePath = (TH3F*)(DeDxMap_->Clone("Prob_ChargePath"));
  Prob_ChargePath->Reset();
  Prob_ChargePath->SetDirectory(0);

  //FIXME is it still relevant with pixels?
  if (!splitByModuleType) {
    Prob_ChargePath->RebinX(Prob_ChargePath->GetNbinsX() - 1);  // <-- do not include pixel in the inclusive
  }

  for (int i = 0; i <= Prob_ChargePath->GetXaxis()->GetNbins() + 1; i++) {    // loop over geometry/layer
    for (int j = 0; j <= Prob_ChargePath->GetYaxis()->GetNbins() + 1; j++) {  // loop over pathlength
      float Ni = 0;
      for (int k = 0; k <= Prob_ChargePath->GetZaxis()->GetNbins() + 1; k++) {  // loop over ChargeOverPathlength
        Ni += DeDxMap_->GetBinContent(i, j, k);
      }
      for (int k = 0; k <= Prob_ChargePath->GetZaxis()->GetNbins() + 1; k++) {  // loop over ChargeOverPathlength
        float tmp = 0;
        for (int l = 0; l <= k; l++) {
          tmp += DeDxMap_->GetBinContent(i, j, l);
        }
        if (Ni > 0) {
          Prob_ChargePath->SetBinContent(i, j, k, tmp / Ni);
        } else {
          Prob_ChargePath->SetBinContent(i, j, k, 0);
        }
      }
    }
  }
  InputFile->Close();
  return Prob_ChargePath;
}

//factorial function used to compute probQ
int factorial(int n) { 
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n; 
} 