#ifndef HSCPVERTEXSELECTOR_H
#define HSCPVERTEXSELECTOR_H

// -*- C++ -*-
// Class:      HSCPVertexSelector
// Original Author:  Emery Nibigira
//         Created:  Mon, 22 Jan 2024 17:47:22 GMT

// system include files
#include <memory>
#include <vector>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "DataFormats/VertexReco/interface/Vertex.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

//
// class declaration
//

class HSCPVertexSelector {
public:
  explicit HSCPVertexSelector(std::vector<reco::Vertex>  vertices);
  virtual ~HSCPVertexSelector();

  void process();

  bool isGoodVertex(const reco::Vertex vtx);
  reco::Vertex getBestVertex();
  std::vector<reco::Vertex> getGoodVertices();
  


private:
  std::vector<reco::Vertex>  vertices_;
  std::vector<reco::Vertex>  goodVertices_;
  reco::Vertex bestGoodVertex_;
};

#endif /* HSCPVERTEXSELECTOR_H */