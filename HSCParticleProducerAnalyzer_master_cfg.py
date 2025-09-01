import sys, os
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

options = VarParsing('analysis')

# defaults
options.outputFile = 'Histos_.root'
options.maxEvents = -1 # -1 means all events


options.register('GTAG', '106X_dataRun2_v36',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Global Tag"
)
options.register('SAMPLE', 'isData',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Sample Type. Use: isSignal or isBckg or isData"
)
options.register('YEAR', '2018',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Year. Use: 2017 or 2018"
)
options.register('ERA', 'D', 
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string, 
    'Sample Type. Use: A,B,C,D,E,F,G,H, PERSO'
)
options.register('isSkimmedSample', False,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "is sample Skimmed? True or False"
)
options.register('LUMITOPROCESS', '',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Lumi to process"
)
options.parseArguments()


process = cms.Process("HSCPAnalysis")
print('Global Tag    : {}'.format(options.GTAG))
if options.SAMPLE=='isData':
   print('Lumi File     : {}'.format(options.LUMITOPROCESS))
print('Sample Type   : {}'.format(options.SAMPLE))
print('is skimmed    : {}'.format(options.isSkimmedSample))
print('Output File   : {}'.format(options.outputFile))
print('Input Files   : {}'.format(options.inputFiles))

process.load("FWCore.MessageService.MessageLogger_cfi")
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load('Configuration.StandardSequences.Services_cff')
process.load("SUSYBSMAnalysis.Analyzer.metFilters_cff")

process.options   = cms.untracked.PSet(
#      wantSummary = cms.untracked.bool(False),
)
process.MessageLogger.cerr.FwkReport.reportEvery = 1000

process.maxEvents = cms.untracked.PSet( input = cms.untracked.int32(options.maxEvents) )
process.source = cms.Source("PoolSource",
   fileNames = cms.untracked.vstring(
    "/store/data/Run2018D/SingleMuon/AOD/15Feb2022_UL2018-v1/2430000/02A7E98B-8CEF-7941-AD47-1124EF69146E.root",
    "/store/data/Run2018B/SingleMuon/AOD/15Feb2022_UL2018-v1/2430000/10846C6A-28A0-CA41-B480-4DB55CF6B6AA.root",
    "/store/data/Run2018B/SingleMuon/AOD/15Feb2022_UL2018-v1/2430000/4A293281-A249-D347-897E-5B943D6C8555.root",
    "/store/data/Run2018B/SingleMuon/AOD/15Feb2022_UL2018-v1/2430000/5BFFDC75-48B1-6F4A-8050-E2EF89599D66.root"
    #"/store/data/Run2018A/MET/AOD/15Feb2022_UL2018-v1/2430000/0080E97A-1A09-0D47-B1F3-FEAEB694D9C8.root"
    ),
   inputCommands = cms.untracked.vstring("keep *", "drop *_MEtoEDMConverter_*_*")
)

#The duplicateCheckMode works only if we submit with Condor - not with Crab - checks process history, run number, lumi number
process.source.duplicateCheckMode = cms.untracked.string("checkAllFilesOpened")

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, options.GTAG, '')

process.HSCPTuplePath = cms.Path()

########################################################################
#Run the Skim sequence if necessary
if(not options.isSkimmedSample):
   process.nEventsBefSkim  = cms.EDProducer("EventCountProducer")

   process.load('HLTrigger.HLTfilters.hltHighLevel_cfi')
   process.HSCPTrigger = process.hltHighLevel.clone()
   process.HSCPTrigger.TriggerResultsTag = cms.InputTag( "TriggerResults", "", "HLT" )
   process.HSCPTrigger.andOr = cms.bool( True ) #OR
   process.HSCPTrigger.throw = cms.bool( False )
   if(options.SAMPLE=='isData' and options.YEAR==2017):
      process.HSCPTrigger.HLTPaths = [ #check triggers
          "HLT_PFMET120_PFMHT120_IDTight_v*",
          "HLT_Mu50_v*",
          "HLT_PFHT500_PFMET100_PFMHT100_IDTight_v*",
          "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_PFHT60_v*",
          "HLT_MET105_IsoTrk50_v*",
          "HLT_IsoMu27_v*"
      ]
   elif(options.SAMPLE=='isData' and options.YEAR==2018):
      process.HSCPTrigger.HLTPaths = [ #check triggers
          "HLT_PFMET120_PFMHT120_IDTight_v*",
          "HLT_Mu50_v*",
          "HLT_PFHT500_PFMET100_PFMHT100_IDTight_v*",
          "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_PFHT60_v*",
          "HLT_MET105_IsoTrk50_v*",
          "HLT_IsoMu24_v*",
      ]
   else:
      #do not apply trigger filter on signal
      process.HSCPTrigger.HLTPaths = ["*"]

   process.HSCPTuplePath += process.nEventsBefSkim + process.HSCPTrigger + process.metFilters

########################################################################

#Run the HSCP EDM-tuple Sequence on skimmed sample
process.nEventsBefEDM   = cms.EDProducer("EventCountProducer")
process.load("SUSYBSMAnalysis.HSCP.HSCParticleProducer_cff")
process.HSCPTuplePath += process.nEventsBefEDM + process.HSCParticleProducerSeq

########################################################################  
# Only for MC samples, save skimmed genParticles

if(options.SAMPLE=='isSignal' or options.SAMPLE=='isBckg'):
   process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
   process.genParticlesSkimmed = cms.EDFilter("GenParticleSelector",
        filter = cms.bool(False),
        src = cms.InputTag("genParticles"),
        cut = cms.string('pt > 5.0'),
        stableOnly = cms.bool(True)
   )

   process.HSCPTuplePath += process.genParticlesSkimmed

########################################################################

from PhysicsTools.SelectorUtils.tools.vid_id_tools import *
electron_id_config = cms.PSet(electron_ids = cms.vstring([                   
                    'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Summer16_80X_V1_cff',
                    'RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Spring16_GeneralPurpose_V1_cff',
                    'RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Spring16_HZZ_V1_cff',
                    'RecoEgamma.ElectronIdentification.Identification.cutBasedElectronID_Fall17_94X_V2_cff',
                    'RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Fall17_noIso_V2_cff', 
                    'RecoEgamma.ElectronIdentification.Identification.mvaElectronID_Fall17_iso_V2_cff',
                    ]))  


                 
switchOnVIDElectronIdProducer(process,DataFormat.AOD)
for idmod in electron_id_config.electron_ids.value():
    setupAllVIDIdsInModule(process,idmod,setupVIDElectronSelection)

process.egmGsfElectronIDs.physicsObjectSrc = cms.InputTag("gedGsfElectrons")
process.electronMVAValueMapProducer.src   = cms.InputTag("gedGsfElectrons")

process.HSCPTuplePath += process.egmGsfElectronIDSequence

#make the pool output
process.Out = cms.OutputModule("PoolOutputModule",
     outputCommands = cms.untracked.vstring(
         "drop *",
         "keep EventAux_*_*_*",
         "keep LumiSummary_*_*_*",
         "keep edmMergeableCounter_*_*_*",
         "keep GenRunInfoProduct_*_*_*",
         "keep GenEventInfoProduct_generator_*_*",
         "keep *_genParticlesSkimmed_*_*",
         "keep *_genParticlePlusGeant_*_*",
         "keep *_offlinePrimaryVertices_*_*",
         "keep recoTracks_generalTracks_*_*",
         "keep recoTracks_standAloneMuons_*_*",
         "keep recoTrackExtras_standAloneMuons_*_*",
         "keep TrackingRecHitsOwned_standAloneMuons_*_*",
         "keep recoTracks_globalMuons_*_*",
         "keep recoTrackExtras_globalMuons_*_*",
         "keep recoMuons_muons_*_*",
         "keep recoMuonTimeExtraedmValueMap_muons_*_*",
         "keep edmTriggerResults_TriggerResults_*_*",
         "keep *_ak4PFJetsCHS__*",
         "keep recoPFMETs_pfMet__*",
         "keep *_HSCParticleProducer_*_*",
         "keep *_HSCPIsolation*_*_*",
         "keep *_dedxHitInfo*_*_*",
         "keep triggerTriggerEvent_hltTriggerSummaryAOD_*_*",
         "keep *_offlineBeamSpot_*_*",
         "keep *_MuonSegmentProducer_*_*",
         "keep *_g4SimHits_StoppedParticles*_*",
         "keep PileupSummaryInfos_addPileupInfo_*_*",
         "keep *_dt4DSegments__*",
         "keep *_cscSegments__*",
         "keep *_scalersRawToDigi_*_*",
         "keep *_caloMet_*_*",
    ),
    fileName = cms.untracked.string(options.outputFile),
    SelectEvents = cms.untracked.PSet(
       SelectEvents = cms.vstring('*')
    ),
)

if(options.SAMPLE=='isData' and len(options.LUMITOPROCESS)>0):
   import FWCore.PythonUtilities.LumiList as LumiList
   process.source.lumisToProcess = LumiList.LumiList(filename = options.LUMITOPROCESS).getVLuminosityBlockRange()
   #process.source.lumisToProcess = LumiList.LumiList(url = https://cms-service-dqm.web.cern.ch/cms-service-dqm/CAF/certification/Collisions17/13TeV/ReReco/Cert_294927-306462_13TeV_EOY2017ReReco_Collisions17_JSON.txt).getVLuminosityBlockRange()

if(options.SAMPLE=='isBckg' or options.SAMPLE=='isData'):
   process.Out.SelectEvents.SelectEvents =  cms.vstring('HSCPTuplePath')  #take just the skimmed ones
   process.Out.outputCommands.extend(["drop triggerTriggerEvent_hltTriggerSummaryAOD_*_*",
                                    "keep edmMergeableCounter_nEventsBefSkim__*",
                                    "keep edmMergeableCounter_nEventsBefEDM__*"])
else:
   process.Out.SelectEvents = cms.untracked.PSet()


########################################################################

if options.SAMPLE == 'isData':
    SampleType = 0
    if options.YEAR == '2016':
        K = 2.3
        C = 3.17
        SF0 = 1.0
        SF1 = 1.0325
        if options.ERA == 'A':
            IasTemplate = 'template_2016B_v5.root'
        if options.ERA == 'B':
            IasTemplate = 'template_2016B_v5.root'
        if options.ERA == 'C':
            IasTemplate = 'template_2016C_v5.root'
        if options.ERA == 'D':
            IasTemplate = 'template_2016D_v5.root'
        if options.ERA == 'E':
            IasTemplate = 'template_2016E_v5.root'
        if options.ERA == 'F':
            IasTemplate = 'template_2016F_v5.root'
        if options.ERA == 'G':
            IasTemplate = 'template_2016G_v5.root'
        if options.ERA == 'H':
            IasTemplate = 'template_2016H_v5.root'
    if options.YEAR == '2017':
        K = 2.54
        C = 3.14
        SF0 = 1.0
        SF1 = 0.990
        if options.ERA == 'A':
            IasTemplate = 'template_2017B_v5.root'
        if options.ERA == 'B':
            IasTemplate = 'template_2017B_v5.root'
        if options.ERA == 'C':
            IasTemplate = 'template_2017C_v5.root'
        if options.ERA == 'D':
            IasTemplate = 'template_2017D_v5.root'
        if options.ERA == 'E':
            IasTemplate = 'template_2017E_v5.root'
        if options.ERA == 'F':
            IasTemplate = 'template_2017F_v5.root'
        if options.ERA == 'G':
            IasTemplate = 'template_2017F_v5.root'
        if options.ERA == 'H':
            IasTemplate = 'template_2017F_v5.root'
    if options.YEAR == '2018':
        K = 2.55
        C = 3.14
        SF0 = 1.0
        SF1 = 1.035
        if options.ERA == 'A':
            IasTemplate = 'template_2018A_v5.root'
        if options.ERA == 'B':
            IasTemplate = 'template_2018B_v5.root'
        if options.ERA == 'C':
            IasTemplate = 'template_2018C_v5.root'
        if options.ERA == 'D':
            IasTemplate = 'template_2018D_v5.root'
        if options.ERA == 'PERSO':
            IasTemplate = 'GiTemplate_EtaExtension.root'
else:
    if options.SAMPLE == 'isBckg':
        SampleType = 1
        if options.YEAR == '2017':
            K = 2.48
            C = 3.19
            SF0 = 1.009
            SF1 = 1.044
            IasTemplate = 'template_2017MC_v5.root'
        if options.YEAR == '2018':
            K = 2.49
            C = 3.18
            SF0 = 1.006
            SF1 = 1.097
            IasTemplate = 'template_2018MC_v5.root'
    else:
        SampleType = 2
        if options.YEAR == '2017':
            K = 2.48
            C = 3.19
            SF0 = 1.009
            SF1 = 1.044
            IasTemplate = 'template_2017MC_v5.root'
        if options.YEAR == '2018':
            K = 2.49
            C = 3.18
            SF0 = 1.006
            SF1 = 1.097
            IasTemplate = 'template_2018MC_v5.root'

process.load("SUSYBSMAnalysis.Analyzer.HSCParticleAnalyzer_cfi")
#process.HSCParticleAnalyzer.TypeMode = 2 # 0: Tracker only
process.HSCParticleAnalyzer.SampleType = SampleType
process.HSCParticleAnalyzer.SaveTree = 6 #6 is all saved, 0 is none
process.HSCParticleAnalyzer.DeDxTemplate = 'GiTemplate_EtaExtension_SatNewCorr.root'
process.HSCParticleAnalyzer.DeDxTemplate_OldSatCorr = 'GiTemplate_EtaExtension.root'
process.HSCParticleAnalyzer.TimeOffset = "MuonTimeOffset_2018A_Zeros.txt"
process.HSCParticleAnalyzer.DebugLevel = -11
process.HSCParticleAnalyzer.Period = options.YEAR
process.HSCParticleAnalyzer.DeDxK = K
process.HSCParticleAnalyzer.DeDxC = C
process.HSCParticleAnalyzer.DeDxSF_0 = SF0
process.HSCParticleAnalyzer.DeDxSF_1 = SF1
process.HSCParticleAnalyzer.GlobalMinIh = C
process.HSCParticleAnalyzer.GlobalMaxEta = 2.4
print ('DeDx Template : {}\n'.format(process.HSCParticleAnalyzer.DeDxTemplate))

process.TFileService = cms.Service("TFileService",
                                       fileName = cms.string(options.outputFile)
                                   )

process.analysis = cms.Path(process.HSCParticleAnalyzer)

process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load("RecoLocalTracker.SiPixelRecHits.PixelCPEGeneric_cfi")
process.load("RecoLocalTracker.SiPixelRecHits.SiPixelRecHits_cfi")
process.endjob_step = cms.EndPath(process.endOfProcess)

process.HSCPTuplePath += process.HSCParticleAnalyzer

process.PixelCPEGenericESProducer = cms.ESProducer(
    "PixelCPEGenericESProducer",
    LoadTemplatesFromDB       = cms.bool(True),
    UseErrorsFromTemplates    = cms.bool(True),
    TruncatePixelCharge       = cms.bool(True),
    IrradiationBiasCorrection = cms.bool(False),
    DoCosmics                 = cms.bool(False),
    ComponentName             = cms.string("PixelCPEGeneric")
)

########################################################################

process.tsk = cms.Task()
#for mod in process.producers_().itervalues():
for mod in process.producers_().values():
    process.tsk.add(mod)
#for mod in process.filters_().itervalues():
for mod in process.filters_().values():
    process.tsk.add(mod)

#schedule the sequence
process.endPath1 = cms.EndPath(process.Out)
process.schedule = cms.Schedule(process.HSCPTuplePath, process.endjob_step)

process.options.numberOfConcurrentLuminosityBlocks = cms.untracked.uint32(1)
process.options.numberOfThreads=cms.untracked.uint32(4)
