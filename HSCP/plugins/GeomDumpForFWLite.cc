// -*- C++ -*-
//
// Package:    GeomDumpForFWLite
// Class:      GeomDumpForFWLite
//
/**\class GeomDumpForFWLite GeomDumpForFWLite.cc 

 Description: <one line class summary>

 Implementation:
     <Notes on implementation>
*/
//

#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"

#include "Geometry/TrackerGeometryBuilder/interface/TrackerGeometry.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"
#include "Geometry/Records/interface/TrackerDigiGeometryRecord.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/DTGeometry/interface/DTGeometry.h"
#include "Geometry/CSCGeometry/interface/CSCGeometry.h"
#include "Geometry/RPCGeometry/interface/RPCGeometry.h"

#include "DataFormats/GeometrySurface/interface/BoundSurface.h"
#include "DataFormats/GeometrySurface/interface/TrapezoidalPlaneBounds.h"
#include "DataFormats/GeometrySurface/interface/RectangularPlaneBounds.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "TTree.h"
#include "TVector3.h"

using namespace edm;
using namespace std;

//
// class declaration
//

class GeomDumpForFWLite : public edm::one::EDAnalyzer<edm::one::SharedResources, edm::one::WatchRuns> {
public:
  explicit GeomDumpForFWLite(const edm::ParameterSet&);
  ~GeomDumpForFWLite() override;
  void endRun(const edm::Run&, const edm::EventSetup&) override;

private:
  void beginRun(const edm::Run& run, const edm::EventSetup&) override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  bool isInitialized;

  // --- ESGetTokens ---
  edm::ESGetToken<TrackerGeometry, TrackerDigiGeometryRecord> tkGeomToken_;
  edm::ESGetToken<DTGeometry, MuonGeometryRecord> dtGeomToken_;
  edm::ESGetToken<CSCGeometry, MuonGeometryRecord> cscGeomToken_;
  edm::ESGetToken<RPCGeometry, MuonGeometryRecord> rpcGeomToken_;
};

//
// constructors and destructor
//
GeomDumpForFWLite::GeomDumpForFWLite(const edm::ParameterSet& iConfig)
    : isInitialized(false) {
  tkGeomToken_ = esConsumes();
  dtGeomToken_ = esConsumes();
  cscGeomToken_ = esConsumes();
  rpcGeomToken_ = esConsumes();
}

GeomDumpForFWLite::~GeomDumpForFWLite() {}

void GeomDumpForFWLite::endRun(const edm::Run&, const edm::EventSetup&) {}

void GeomDumpForFWLite::beginRun(const edm::Run& run, const edm::EventSetup& iSetup) {}

void GeomDumpForFWLite::endJob() {}

void GeomDumpForFWLite::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  if (isInitialized)
    return;
  isInitialized = true;

  unsigned int rawId;
  float trapezeParam;
  TVector3* posV = new TVector3();
  TVector3* widthV = new TVector3();
  TVector3* lengthV = new TVector3();
  TVector3* thickV = new TVector3();

  edm::Service<TFileService> tfs;
  TTree* outtree = tfs->make<TTree>("geom", "geom");
  outtree->Branch("rawId", &rawId, "rawId/l");
  outtree->Branch("trapezeParam", &trapezeParam, "trapezeParam/f");
  outtree->Branch("pos", &posV, 32000, 0);
  outtree->Branch("width", &widthV, 32000, 0);
  outtree->Branch("length", &lengthV, 32000, 0);
  outtree->Branch("thick", &thickV, 32000, 0);

  const TrackerGeometry& tkGeom = iSetup.getData(tkGeomToken_);
  const DTGeometry& dtGeom = iSetup.getData(dtGeomToken_);
  const CSCGeometry& cscGeom = iSetup.getData(cscGeomToken_);
  const RPCGeometry& rpcGeom = iSetup.getData(rpcGeomToken_);

  vector<const GeomDet*> TkDets = tkGeom.dets();
  vector<const GeomDet*> DtDets = dtGeom.dets();
  vector<const GeomDet*> CscDets = cscGeom.dets();
  vector<const GeomDet*> RpcDets = rpcGeom.dets();
  vector<const GeomDet*> MuonDets;

  MuonDets.insert(MuonDets.end(), TkDets.begin(), TkDets.end());
  MuonDets.insert(MuonDets.end(), DtDets.begin(), DtDets.end());
  MuonDets.insert(MuonDets.end(), CscDets.begin(), CscDets.end());
  MuonDets.insert(MuonDets.end(), RpcDets.begin(), RpcDets.end());

  for (const auto* DetUnit : MuonDets) {
    if (!DetUnit) continue;

    DetId Detid = DetUnit->geographicalId();
    unsigned int SubDet = Detid.subdetId();
    if (Detid.det() == 1 && (SubDet < 1 || SubDet > 6))
      continue;

    const BoundPlane plane = DetUnit->surface();
    const TrapezoidalPlaneBounds* trapezoidalBounds(dynamic_cast<const TrapezoidalPlaneBounds*>(&(plane.bounds())));
    const RectangularPlaneBounds* rectangularBounds(dynamic_cast<const RectangularPlaneBounds*>(&(plane.bounds())));

    rawId = Detid.rawId();
    trapezeParam = 0;

    float width = 0;
    float length = 0;
    float thickness = 0;
    if (trapezoidalBounds) {
      auto const& parameters = trapezoidalBounds->parameters();
      width = parameters[0] * 2;
      length = parameters[3] * 2;
      thickness = trapezoidalBounds->thickness();
      trapezeParam = parameters[1] / parameters[0];
    } else if (rectangularBounds) {
      width = DetUnit->surface().bounds().width();
      length = DetUnit->surface().bounds().length();
      thickness = DetUnit->surface().bounds().thickness();
      trapezeParam = 1;
    }

    Surface::GlobalPoint WidthVector = plane.toGlobal(LocalPoint(width / 2, 0, 0));
    Surface::GlobalPoint LengthVector = plane.toGlobal(LocalPoint(0, length / 2, 0));
    Surface::GlobalPoint ThickVector = plane.toGlobal(LocalPoint(0, 0, thickness / 2));
    GlobalVector Pos = GlobalVector(DetUnit->position().basicVector());

    posV->SetX(Pos.x());
    posV->SetY(Pos.y());
    posV->SetZ(Pos.z());
    widthV->SetX(WidthVector.x() - Pos.x());
    widthV->SetY(WidthVector.y() - Pos.y());
    widthV->SetZ(WidthVector.z() - Pos.z());
    lengthV->SetX(LengthVector.x() - Pos.x());
    lengthV->SetY(LengthVector.y() - Pos.y());
    lengthV->SetZ(LengthVector.z() - Pos.z());
    thickV->SetX(ThickVector.x() - Pos.x());
    thickV->SetY(ThickVector.y() - Pos.y());
    thickV->SetZ(ThickVector.z() - Pos.z());

    outtree->Fill();
  }
}

//define this as a plug-in
DEFINE_FWK_MODULE(GeomDumpForFWLite);
