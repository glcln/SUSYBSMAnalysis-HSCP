#include <memory>
#include <vector>
#include <map>

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/ESGetToken.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"

#include "TrackingTools/TrackAssociator/interface/TrackDetectorAssociator.h"
#include "TrackingTools/TrackAssociator/interface/TrackAssociatorParameters.h"
#include "TrackingTools/TrackAssociator/interface/DetIdAssociator.h"
#include "TrackingTools/Records/interface/DetIdAssociatorRecord.h"
#include "TrackingTools/TrajectoryState/interface/FreeTrajectoryState.h"

#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "AnalysisDataFormats/SUSYBSMObjects/interface/HSCParticle.h"
#include "AnalysisDataFormats/SUSYBSMObjects/interface/HSCPCaloInfo.h"

class BetaCalculatorECAL {

public:
    BetaCalculatorECAL(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iC);
    void addInfoToCandidate(susybsm::HSCParticle& candidate,
                            edm::Handle<reco::TrackCollection>& tracks,
                            edm::Event& iEvent,
                            const edm::EventSetup& iSetup,
                            susybsm::HSCPCaloInfo& caloInfo);

private:
    int getDetailedTrackLengthInXtals(std::map<int,GlobalPoint>& trackExitPositionMap,
                                      std::map<int,float>& trackCrossedXtalMap,
                                      double& totalLengthCurved,
                                      GlobalPoint& internalPointCurved,
                                      GlobalPoint& externalPointCurved,
                                      const CaloGeometry* theGeometry,
                                      const CaloTopology* theTopology,
                                      const std::vector<SteppingHelixStateInfo>& neckLace);

    std::vector<SteppingHelixStateInfo> calcEcalDeposit(const FreeTrajectoryState* tkInnerState,
                                                        const DetIdAssociator& associator,
                                                        const MagneticField* bField);

    void addStepToXtal(std::map<int,GlobalPoint>& trackExitPositionMap,
                       std::map<int,float>& trackCrossedXtalMap,
                       DetId aDetId,
                       float step,
                       GlobalPoint point,
                       const CaloSubdetectorGeometry* theSubdetGeometry);

    TrackDetectorAssociator trackAssociator_;
    TrackAssociatorParameters parameters_;
    edm::EDGetTokenT<EBRecHitCollection> EBRecHitCollectionToken_;
    edm::EDGetTokenT<EERecHitCollection> EERecHitCollectionToken_;

    edm::ESGetToken<DetIdAssociator, DetIdAssociatorRecord> ecalDetIdAssociatorToken_;
    edm::ESGetToken<MagneticField, IdealMagneticFieldRecord> bFieldToken_;
    edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeometryToken_;
    edm::ESGetToken<CaloTopology, CaloTopologyRecord> caloTopologyToken_;
};
