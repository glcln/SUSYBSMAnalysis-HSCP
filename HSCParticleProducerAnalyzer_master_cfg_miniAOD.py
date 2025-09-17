import os
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing
from Configuration.Eras.Era_Run2_2018_cff import Run2_2018

options = VarParsing('analysis')

# defaults
options.outputFile = 'Histos_.root'
options.maxEvents = -1 # -1 means all events


options.register('GTAG', '106X_upgrade2018_realistic_v11BasedCandidateTmp_2022_08_09_01_32_34',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Global Tag")

options.register ('outputEvery', 1000,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.int,
    "Controls the frequency with which the framework's progress messages are displayed")

options.register ('isData', True,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "")

options.register ('year', 2018,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.int,
    "Year. Use: 2017 or 2018")

options.register ('isAOD', False,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "")

options.register ('triggerFilter', False,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "")

options.register('LUMITOPROCESS', '',
    VarParsing.multiplicity.singleton,
    VarParsing.varType.string,
    "Lumi to process")

options.parseArguments()


if options.isData:
    if options.year>2018:
        options.GTAG = '124X_dataRun3_v15'
    else:
        options.GTAG = '106X_dataRun2_v24'

isAOD = options.isAOD

## print configuration
###############################################################
print('\n')
print('CMSSW version : {}'.format(os.environ['CMSSW_VERSION']))
print('Global Tag    : {}'.format(options.GTAG))
print('is AOD        : {}'.format(options.isAOD))
print('is Data       : {}'.format(options.isData))
print('Year          : {}'.format(options.year))
print('TriggerFilter : {}'.format(options.triggerFilter))
print('Output File   : {}'.format(options.outputFile))
print('Input Files   : {}'.format(options.inputFiles))
print('\n')
#_____________________________________________________________#

process = cms.Process("HSCPAnalysis")

if options.isData: process.load("Configuration.Geometry.GeometryIdeal_cff")
process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load("Configuration.StandardSequences.Reconstruction_cff")
process.load('Configuration.StandardSequences.Services_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load("SUSYBSMAnalysis.Analyzer.metFilters_cff")

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cerr.FwkReport.reportEvery = options.outputEvery
process.MessageLogger.cerr.threshold = "INFO"

process.options   = cms.untracked.PSet(wantSummary = cms.untracked.bool(False))

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(options.maxEvents))

# Define files of dataset
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(
    "/store/data/Run2018D/SingleMuon/MINIAOD/UL2018_MiniAODv2-v3/120000/005B1128-17E2-9148-A9E4-DCF8F1A550CB.root"
    ),
   inputCommands = cms.untracked.vstring("keep *", "drop *_MEtoEDMConverter_*_*")
)
# Number of events to be skipped (0 by default)
process.source.skipEvents = cms.untracked.uint32(0)
# Register fileservice for output file
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)

## Conditions data
###############################################################
from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, options.GTAG, '')
#process.GlobalTag.globaltag = GTAG
### this is necessary to get the simulation geometry
if not options.isData:
    process.GlobalTag.toGet = cms.VPSet(
    cms.PSet(record = cms.string("GeometryFileRcd"),
            tag = cms.string("XMLFILE_Geometry_101YV4_Extended2018_mc"),
            label = cms.untracked.string('Extended'),
            )
    )

if (options.isData and len(options.LUMITOPROCESS)>0):
   import FWCore.PythonUtilities.LumiList as LumiList
   process.source.lumisToProcess = LumiList.LumiList(filename = options.LUMITOPROCESS).getVLuminosityBlockRange()

########################################################################
triggerList=["*"]
if options.year == 2017:
    triggerList=[
        "HLT_PFMET120_PFMHT120_IDTight_v*",
        "HLT_Mu50_v*",
        "HLT_PFHT500_PFMET100_PFMHT100_IDTight_v*",
        "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_PFHT60_v*",
        "HLT_MET105_IsoTrk50_v*",
        "HLT_IsoMu27_v*"
    ]
elif options.year == 2018:
    triggerList=[
        "HLT_PFMET120_PFMHT120_IDTight_v*",
        "HLT_Mu50_v*",
        "HLT_PFHT500_PFMET100_PFMHT100_IDTight_v*",
        "HLT_PFMETNoMu120_PFMHTNoMu120_IDTight_PFHT60_v*",
        "HLT_MET105_IsoTrk50_v*",
        "HLT_IsoMu24_v*"
    ]
else:
    #do not apply trigger filter on signal
    triggerList=["*"]

triggerFilter=options.triggerFilter

if not options.isData and isAOD:
   process.load("SimGeneral.HepPDTESSource.pythiapdt_cfi")
   process.genParticlesSkimmed = cms.EDFilter("GenParticleSelector",
        filter = cms.bool(False),
        src = cms.InputTag("genParticles"),
        cut = cms.string('pt > 5.0'),
        stableOnly = cms.bool(True)
   )

########################################################################

if options.isData:
    if options.year == 2017:
        K = 2.54
        C = 3.14
        SF0 = 1.0
        SF1 = 0.990
    if options.year == 2018:
        K = 2.55
        C = 3.14
        SF0 = 1.0
        SF1 = 1.035
else:
    if options.year == 2017:
        K = 2.48
        C = 3.19
        SF0 = 1.009
        SF1 = 1.044
    if options.year == 2018:
        K = 2.49
        C = 3.18
        SF0 = 1.006
        SF1 = 1.097
########################################################################

process.load("SUSYBSMAnalysis.Analyzer.HSCParticleAnalyzer_cfi")

process.HSCParticleAnalyzer.AddStripClusterInfo = False
process.HSCParticleAnalyzer.TriggerPaths = triggerList
process.HSCParticleAnalyzer.TriggerFilter = triggerFilter
process.HSCParticleAnalyzer.DeDxK = K
process.HSCParticleAnalyzer.DeDxC = C
process.HSCParticleAnalyzer.DeDxSF_0 = SF0
process.HSCParticleAnalyzer.DeDxSF_1 = SF1
process.HSCParticleAnalyzer.DeDxTemplate = 'GiTemplate_EtaExtension_SatNewCorr.root'


process.HSCPTuplePath = cms.Path()
if isAOD:
    process.HSCPTuplePath += process.metFilters
    if not options.isData:
        process.HSCPTuplePath += process.genParticlesSkimmed
    process.HSCPTuplePath += process.HSCParticleProducer
    process.HSCPTuplePath += process.HSCParticleAnalyzer
else:
    process.HSCPTuplePath += process.HSCParticleProducer
    process.HSCPTuplePath += process.HSCParticleAnalyzer

process.endjob_step = cms.EndPath(process.endOfProcess)

# Schedule definition
#process.endPath1 = cms.EndPath(process.Out)
process.schedule = cms.Schedule(process.HSCPTuplePath, process.endjob_step)

process.options.numberOfConcurrentLuminosityBlocks = cms.untracked.uint32(1)
process.options.numberOfThreads=cms.untracked.uint32(4)
