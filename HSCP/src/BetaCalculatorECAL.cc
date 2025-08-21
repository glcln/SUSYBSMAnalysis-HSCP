#include "SUSYBSMAnalysis/HSCP/interface/BetaCalculatorECAL.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "Geometry/CaloGeometry/interface/TruncatedPyramid.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloTopology/interface/CaloSubdetectorTopology.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixStateInfo.h"
#include "TrackPropagation/SteppingHelixPropagator/interface/SteppingHelixPropagator.h"
#include "RecoLocalCalo/EcalRecAlgos/interface/EcalSeverityLevelAlgo.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"

using namespace susybsm;

BetaCalculatorECAL::BetaCalculatorECAL(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iC) :
    EBRecHitCollectionToken_(iC.consumes<EBRecHitCollection>(iConfig.getParameter<edm::InputTag>("EBRecHitCollection"))),
    EERecHitCollectionToken_(iC.consumes<EERecHitCollection>(iConfig.getParameter<edm::InputTag>("EERecHitCollection"))),
    ecalDetIdAssociatorToken_(iC.esConsumes<DetIdAssociator, DetIdAssociatorRecord>()),
    bFieldToken_(iC.esConsumes<MagneticField, IdealMagneticFieldRecord>()),
    caloGeometryToken_(iC.esConsumes<CaloGeometry, CaloGeometryRecord>()),
    caloTopologyToken_(iC.esConsumes<CaloTopology, CaloTopologyRecord>())
{
    edm::ParameterSet trkParameters = iConfig.getParameter<edm::ParameterSet>("TrackAssociatorParameters");
    parameters_.loadParameters(trkParameters, iC);
    trackAssociator_.useDefaultPropagator();
}

void BetaCalculatorECAL::addInfoToCandidate(HSCParticle& candidate,
                                            edm::Handle<reco::TrackCollection>& tracks,
                                            edm::Event& iEvent,
                                            const edm::EventSetup& iSetup,
                                            HSCPCaloInfo& caloInfo) {
    bool setCalo = false;
    HSCPCaloInfo result;

    const DetIdAssociator& ecalDetIdAssociator = iSetup.getData(ecalDetIdAssociatorToken_);
    const MagneticField* bField = &iSetup.getData(bFieldToken_);
    const CaloGeometry* theCaloGeometry = &iSetup.getData(caloGeometryToken_);
    const CaloTopology* theCaloTopology = &iSetup.getData(caloTopologyToken_);


    edm::Handle<EBRecHitCollection> ebRecHits;
    iEvent.getByToken(EBRecHitCollectionToken_, ebRecHits);
    edm::Handle<EERecHitCollection> eeRecHits;
    iEvent.getByToken(EERecHitCollectionToken_, eeRecHits);

    reco::Track track;
    if(candidate.hasTrackRef())
        track = *(candidate.trackRef());
    else
        return;

    result.trkIsoDr = 100;
    for(const auto& ndTrack : *tracks) {
        double dr = std::hypot(track.outerEta() - ndTrack.outerEta(),
                               track.outerPhi() - ndTrack.outerPhi());
        if(dr > 0.00001 && dr < result.trkIsoDr) result.trkIsoDr = dr;
    }

    TrackDetMatchInfo info = trackAssociator_.associate(iEvent, iSetup,
                                                        trackAssociator_.getFreeTrajectoryState(bField, track),
                                                        parameters_);

    std::map<int, GlobalPoint> trackExitPositionMap;
    std::map<int, float> trackCrossedXtalCurvedMap;

    FreeTrajectoryState tkInnerState = trajectoryStateTransform::innerFreeState(track, bField);
    std::vector<SteppingHelixStateInfo> neckLace = calcEcalDeposit(&tkInnerState, ecalDetIdAssociator, bField);

    double totalLengthCurved = 0.;
    GlobalPoint internalPointCurved(0., 0., 0.);
    GlobalPoint externalPointCurved(0., 0., 0.);
    if(neckLace.size() > 1) {
        getDetailedTrackLengthInXtals(trackExitPositionMap,
                                      trackCrossedXtalCurvedMap,
                                      totalLengthCurved,
                                      internalPointCurved,
                                      externalPointCurved,
                                      theCaloGeometry,
                                      theCaloTopology,
                                      neckLace);
    }

    float sumWeightedTime = 0;
    float sumTimeErrorSqr = 0;
    float sumEnergy = 0;
    float sumTrackLength = 0;
    std::vector<EcalRecHit> crossedRecHits;

    auto trackExitMapIt = trackExitPositionMap.begin();
    for(const auto& mapIt : trackCrossedXtalCurvedMap) {
        DetId detId(mapIt.first);
        EcalRecHit hit;
        if(detId.subdetId() == EcalBarrel) {
            EBDetId ebDetId(mapIt.first);
            auto it = ebRecHits->find(ebDetId);
            if(it == ebRecHits->end()) continue;
            hit = *it;
        } else { // Endcap
            EEDetId eeDetId(mapIt.first);
            auto it = eeRecHits->find(eeDetId);
            if(it == eeRecHits->end()) continue;
            hit = *it;
        }

        if(!hit.isTimeValid()) continue;
        uint32_t rhFlag = hit.recoFlag();
        if((rhFlag != EcalRecHit::kGood) && (rhFlag != EcalRecHit::kOutOfTime) && (rhFlag != EcalRecHit::kPoorCalib))
            continue;

        float errorOnThis = hit.timeError();
        sumTrackLength += mapIt.second;
        sumEnergy += hit.energy();
        crossedRecHits.push_back(hit);

        result.ecalTrackLengths.push_back(mapIt.second);
        result.ecalTrackExitPositions.push_back(trackExitMapIt->second);
        result.ecalEnergies.push_back(hit.energy());
        result.ecalTimes.push_back(hit.time());
        result.ecalTimeErrors.push_back(hit.timeError());
        result.ecalOutOfTimeEnergies.push_back(0.);
        result.ecalOutOfTimeChi2s.push_back(0.);
        result.ecalChi2s.push_back(hit.chi2());
        result.ecalDetIds.push_back(detId);

        if(hit.isTimeErrorValid()) {
            sumWeightedTime += hit.time() / (errorOnThis*errorOnThis);
            sumTimeErrorSqr += 1 / (errorOnThis*errorOnThis);
        }

        ++trackExitMapIt;
    }

    if(!crossedRecHits.empty()) {
        setCalo = true;
        std::sort(crossedRecHits.begin(), crossedRecHits.end(),
                  [](const auto& x, const auto& y){ return x.energy() > y.energy(); });

        result.ecalCrossedEnergy = sumEnergy;
        result.ecalCrysCrossed = crossedRecHits.size();
        result.ecalDeDx = sumEnergy / sumTrackLength;
        result.ecal3by3dir = info.nXnEnergy(TrackDetMatchInfo::EcalRecHits, 1);
        result.ecal5by5dir = info.nXnEnergy(TrackDetMatchInfo::EcalRecHits, 2);

        if(sumTimeErrorSqr > 0) {
            result.ecalTime = sumWeightedTime / sumTimeErrorSqr;
            result.ecalTimeError = std::sqrt(1 / sumTimeErrorSqr);

            DetId maxEnergyId = crossedRecHits.begin()->id();
            if(maxEnergyId != DetId()) {
                GlobalPoint position = info.getPosition(maxEnergyId);
                double frontFaceR = position.mag();
                double muonShowerMax = frontFaceR + 11.5;
                double gammaShowerMax = frontFaceR + 6.23;
                double speedOfLight = 29.979;

                result.ecalBeta = muonShowerMax / (result.ecalTime*speedOfLight + gammaShowerMax);
                result.ecalBetaError = (speedOfLight*muonShowerMax*result.ecalTimeError)
                                        / std::pow(speedOfLight*result.ecalTime + gammaShowerMax,2);
                result.ecalInvBetaError = speedOfLight*result.ecalTimeError/muonShowerMax;
            }
        }
    }

    if(!info.crossedHcalRecHits.empty()) {
        result.hcalCrossedEnergy = info.crossedEnergy(TrackDetMatchInfo::HcalRecHits);
        result.hoCrossedEnergy = info.crossedEnergy(TrackDetMatchInfo::HORecHits);
        result.hcal3by3dir = info.nXnEnergy(TrackDetMatchInfo::HcalRecHits, 1);
        result.hcal5by5dir = info.nXnEnergy(TrackDetMatchInfo::HcalRecHits, 2);
    }

    if(setCalo)
        caloInfo = result;
}


std::vector<SteppingHelixStateInfo> BetaCalculatorECAL::calcEcalDeposit(const FreeTrajectoryState* tkInnerState,
                                                                        const DetIdAssociator& associator,
                                                                        const MagneticField* bField)
{
    double minR = associator.volume().minR();
    double minZ = associator.volume().minZ();
    double maxR = associator.volume().maxR();
    double maxZ = associator.volume().maxZ();

    SteppingHelixStateInfo trackOrigin(*tkInnerState);

    SteppingHelixPropagator prop(bField, alongMomentum);
    prop.setMaterialMode(false);
    prop.applyRadX0Correction(true);

    return propagateThoughFromIP(trackOrigin, &prop, associator.volume(), 500, 0.1, minR, minZ, maxR, maxZ);
}


int BetaCalculatorECAL::getDetailedTrackLengthInXtals(std::map<int,GlobalPoint>& trackExitPositionMap,
                                                      std::map<int,float>& trackCrossedXtalMap,
                                                      double& totalLengthCurved,
                                                      GlobalPoint& internalPointCurved,
                                                      GlobalPoint& externalPointCurved,
                                                      const CaloGeometry* theGeometry,
                                                      const CaloTopology* theTopology,
                                                      const std::vector<SteppingHelixStateInfo>& neckLace)
{
    GlobalPoint origin(0.,0.,0.);
    internalPointCurved = origin;
    externalPointCurved = origin;
    bool firstPoint = false;
    trackCrossedXtalMap.clear();

    const CaloSubdetectorGeometry* theBarrelSubdetGeometry = theGeometry->getSubdetectorGeometry(DetId::Ecal,1);
    const CaloSubdetectorGeometry* theEndcapSubdetGeometry = theGeometry->getSubdetectorGeometry(DetId::Ecal,2);

    for(auto itr = neckLace.begin()+1; itr != neckLace.end(); ++itr)
    {
        GlobalPoint probe_gp = (*itr).position();
        std::vector<DetId> surroundingMatrix;

        EBDetId closestBarrelDetIdToProbe = theBarrelSubdetGeometry->getClosestCell(probe_gp).rawId();
        EEDetId closestEndcapDetIdToProbe = theEndcapSubdetGeometry->getClosestCell(probe_gp).rawId();

        if((closestEndcapDetIdToProbe) && (theGeometry->getSubdetectorGeometry(closestEndcapDetIdToProbe)->getGeometry(closestEndcapDetIdToProbe)->inside(probe_gp)))
        {
            double step = ((*itr).position() - (*(itr-1)).position()).mag();
            GlobalPoint point = itr->position();
            addStepToXtal(trackExitPositionMap, trackCrossedXtalMap, closestEndcapDetIdToProbe, step, point, theEndcapSubdetGeometry);
            totalLengthCurved += step;
            if(!firstPoint) { internalPointCurved = probe_gp; firstPoint = true; }
            externalPointCurved = probe_gp;
        }

        if((closestBarrelDetIdToProbe) && (theGeometry->getSubdetectorGeometry(closestBarrelDetIdToProbe)->getGeometry(closestBarrelDetIdToProbe)->inside(probe_gp)))
        {
            double step = ((*itr).position() - (*(itr-1)).position()).mag();
            GlobalPoint point = itr->position();
            addStepToXtal(trackExitPositionMap, trackCrossedXtalMap, closestBarrelDetIdToProbe, step, point, theBarrelSubdetGeometry);
            totalLengthCurved += step;
            if(!firstPoint) { internalPointCurved = probe_gp; firstPoint = true; }
            externalPointCurved = probe_gp;
        }
    }

    return 0;
}

void BetaCalculatorECAL::addStepToXtal(std::map<int,GlobalPoint>& trackExitPositionMap,
                                       std::map<int,float>& trackCrossedXtalMap,
                                       DetId aDetId,
                                       float step,
                                       GlobalPoint point,
                                       const CaloSubdetectorGeometry* theSubdetGeometry)
{
    trackExitPositionMap[aDetId.rawId()] = point;
    if(trackCrossedXtalMap.find(aDetId.rawId()) != trackCrossedXtalMap.end())
        trackCrossedXtalMap[aDetId.rawId()] += step;
    else
        trackCrossedXtalMap[aDetId.rawId()] = step;
}