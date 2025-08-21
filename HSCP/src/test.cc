#include "SUSYBSMAnalysis/HSCP/interface/BetaCalculatorRPC.h"
using namespace susybsm;

BetaCalculatorRPC::BetaCalculatorRPC(const edm::ParameterSet& iConfig, edm::ConsumesCollector&& iC) {
  rpcRecHitsToken = iC.consumes<RPCRecHitCollection>( iConfig.getParameter<edm::InputTag>("rpcRecHits") );
  // Nouveau en Run3 : ESGetToken via esConsumes
  rpcGeomToken_   = iC.esConsumes<RPCGeometry, MuonGeometryRecord>();
}

void BetaCalculatorRPC::addInfoToCandidate(HSCParticle& candidate,
                                           const edm::Event& iEvent,
                                           const edm::EventSetup& iSetup) {
  // AVANT (obsolete):
  // edm::ESHandle<RPCGeometry> rpcGeo;
  // iSetup.get<MuonGeometryRecord>().get(rpcGeo);

  // MAINTENANT :
  auto const& rpcGeo = iSetup.getData(rpcGeomToken_);

  edm::Handle<RPCRecHitCollection> rpcHits;
  iEvent.getByToken(rpcRecHitsToken, rpcHits);

  RPCBetaMeasurement result;
  std::vector<RPCHit4D> hits;

  trackingRecHit_iterator start, stop;
  reco::Track track;

  // FIXME AOD: recHits tracking ne sont plus là en miniAOD; ok pour l’instant si tu tournes AOD.
  if (candidate.hasMuonRef() && candidate.muonRef()->combinedMuon().isNonnull()) {
    start = candidate.muonRef()->combinedMuon()->recHitsBegin();
    stop  = candidate.muonRef()->combinedMuon()->recHitsEnd();
  } else if (candidate.hasMuonRef() && candidate.muonRef()->standAloneMuon().isNonnull()) {
    track = *(candidate.muonRef()->standAloneMuon());
    start = candidate.muonRef()->standAloneMuon()->recHitsBegin();
    stop  = candidate.muonRef()->standAloneMuon()->recHitsEnd();
  } else {
    return;
  }

  for (trackingRecHit_iterator recHit = start; recHit != stop; ++recHit) {
    if ((*recHit)->geographicalId().det() != DetId::Muon) continue;
    if ((*recHit)->geographicalId().subdetId() != MuonSubdetId::RPC) continue;
    if (!(*recHit)->isValid()) continue;

    RPCDetId rollId = (RPCDetId)(*recHit)->geographicalId();

    using rangeRecHits = std::pair<RPCRecHitCollection::const_iterator, RPCRecHitCollection::const_iterator>;
    rangeRecHits recHitCollection = rpcHits->get(rollId);

    int size = 0, clusterS = 0;
    for (auto recHitC = recHitCollection.first; recHitC != recHitCollection.second; ++recHitC) {
      clusterS = recHitC->clusterSize();
      size++;
    }
    if (size > 1) continue;     // un seul rechit dans ce roll ?
    if (clusterS > 4) continue; // cluster size ≤ 4

    LocalPoint recHitPos = (*recHit)->localPosition();
    // NOTE: rpcGeo est une référence, plus besoin de flèche ->
    const RPCRoll* rollasociated = rpcGeo.roll(rollId);
    const BoundPlane& RPCSurface  = rollasociated->surface();

    RPCHit4D ThisHit;
    ThisHit.bx = static_cast<const RPCRecHit&>(**recHit).BunchX(); // identique à ton cast
    ThisHit.gp = RPCSurface.toGlobal(recHitPos);
    ThisHit.id = (RPCDetId)(*recHit)->geographicalId().rawId();
    hits.push_back(ThisHit);
  }

  // Suite inchangée…
  std::sort(hits.begin(), hits.end());
  int lastbx = -7;
  bool increasing = true, outOfTime = false;
  for (auto it = hits.begin(); it < hits.end(); ++it) {
    outOfTime |= (it->bx != 0);
    increasing &= (it->bx >= lastbx);
    lastbx = it->bx;
  }
  result.isCandidate = (outOfTime && increasing);

  algo(hits);
  result.beta = beta();
  candidate.setRpc(result);
}
