/*
  Downsampling triggers prototype based on the Run1/Run2 skimming triggers
  .L $NOTES/JIRA/ATO-575/downsamplingTrigger.C+
   //testMCGener0(1000000);
   testMCGener1(10000000);
*/
#include "TDatabasePDG.h"
#include "TMath.h"
#include "TRandom.h"
#include "TTreeStream.h"
#include "TFile.h"
#include "TTree.h"
#include "TF1.h"
//#include "ROOT/Math.h"
#include "Math/PdfFuncMathCore.h"

/// Tsalis/Hagedorn function describing charged pt spectra (m s = 62.4 GeV to 13 TeV) as in https://iopscience.iop.org/article/10.1088/2399-6528/aab00f/pdf
/// https://github.com/alisw/AliPhysics/blob/523f2dc8b45d913e9b7fda9b27e746819cbe5b09/PWGPP/AliAnalysisTaskFilteredTree.h#L145
/// \param pt     - transverse momentum
/// \param mass   - mass of particle
/// \param sqrts  -
/// \return       - invariant yields of the charged particle *pt
///    n(sqrts)= a + b/sqrt(s)                             - formula 6
///    T(sqrts)= c + d/sqrt(s)                             - formula 7
///    a = 6.81 ± 0.06       and b = 59.24 ± 3.53 GeV      - for charged particles page 3
///    c = 0.082 ± 0.002 GeV and d = 0.151 ± 0.048 (GeV)   - for charged particles page 4
Double_t TsalisCharged(Double_t pt, Double_t mass, Double_t sqrts){
  const Double_t a=6.81,   b=59.24;
  const Double_t c=0.082,  d=0.151;
  Double_t mt=TMath::Sqrt(mass*mass+pt*pt);
  Double_t n=a+b/sqrts;
  Double_t T=c+d/sqrts;
  Double_t p0 = n*T;
  Double_t result=TMath::Power((1.+mt/p0),-n);
  result*=pt;
  return result;
}

/// Random downsampling trigger function using Tsalis/Hagedorn spectra fit (sqrt(s) = 62.4 GeV to 13 TeV) as in https://iopscience.iop.org/article/10.1088/2399-6528/aab00f/pdf
/// \param pt
/// \param mass
/// \param sqrts
/// \param factorPt
/// \param factor1Pt
/// \return trigger bitmask
///         bit 1 - flat pt   trigger
///         bit 2 - flat q/pt trigger
///         bit 3 - MB trigger
Int_t  DownsampleTsalisCharged(Double_t pt, Double_t factorPt, Double_t factor1Pt, Double_t sqrts, Double_t mass, Double_t *weight){
  Double_t prob=TsalisCharged(pt,mass,sqrts);
  Double_t probNorm=TsalisCharged(1.,mass,sqrts);
  Int_t triggerMask=0;
  (*weight)=prob/probNorm;
  if (gRandom->Rndm()*prob/probNorm<factorPt) triggerMask|=1;
  if ((gRandom->Rndm()*((prob/probNorm)*pt*pt))<factor1Pt) triggerMask|=2;
  if (gRandom->Rndm()<factorPt) triggerMask|=4;
  return triggerMask;
}

/// to be defined
/// here we are using dummy log normal for the moment
float multiplicityProb(Double_t mult, Double_t sqrts, Double_t etaRange){
  // TF1 *f1 = new TF1("logNormal","[0]*ROOT::Math::lognormal_pdf(x,log([1]),log([2]))",0,5);
  const double p1=2.40816e+01;
  const double p2=1.62;
  double pdf =ROOT::Math::lognormal_pdf(mult,log(p1),log(p2));
  return pdf;
}

void testMCGener0(Int_t nPoints=100000){
  // nPoints=100000
  TTreeSRedirector * pcstream = new TTreeSRedirector("testDownsamplingTrigger.root","recreate");
  Float_t mass=0.139;
  Float_t sqrts=14400;
  float factor1Pt=0.01;
  float factorPt=0.01;

  TF1 fts("fts"," TsalisCharged(x, [0], [1])/ TsalisCharged(1, [0], [1])",0.1,100);
  fts.SetNpx(100000);
  fts.SetParameters(mass,sqrts);
  for (Int_t iLoop=0; iLoop<nPoints; iLoop++) {
    float pt = fts.GetRandom();
    double weight = 0;
    Int_t triggerMask = DownsampleTsalisCharged(pt, factorPt, factor1Pt, sqrts, mass, &weight);
    //if (triggerMask<=0) continue;
    (*pcstream) << "trigger" <<
             "pt=" << pt <<                        // pt
             "mass="<<mass<<
             "sqrts="<<sqrts<<
             "triggerMask=" << triggerMask <<      // triggerMask
             "weight=" << weight <<
             "\n";
  }
  delete pcstream;
}


void makeUnitTests0(){
  //
  TFile *f = TFile::Open("testDownsamplingTrigger.root");
  TTree * tree = (TTree*)f->Get("trigger");
  // this spectra should be flat  as weight =1/prob  - OK
  tree->Draw("pt>>hisUni(50,0,5)","1./TsalisCharged(pt,mass,sqrts)","");
  //this spectra for MB trigger  should be flat  as weight =1/prob  - OK
  tree->Draw("pt>>hisUniMB(50,0,5)","(1./TsalisCharged(pt,mass,sqrts)*((triggerMask&0x4)>0))","");
  // this should be line  0 it is OK
  tree->Draw("weight:TsalisCharged(pt,mass,sqrts)","","",10000);
  // this should be flat untill some momenta where we will use all of tracks - OK
  tree->Draw("pt>>hisT1(50,0,5)","((triggerMask&0x1)>0))","");
  // this should be flat untill some momenta where we will use all of tracks - OK
  tree->Draw("1/pt>>hisT2(50,0,10)","((triggerMask&0x2)>0)","");
  // show that reweighted spectra are consistent with the MB and raw spectra
  // triggerMask1  - OK
  // triggerMask2  - OK
  tree->SetMarkerStyle(21);tree->SetMarkerSize(0.7);
  tree->SetLineColor(1);;tree->SetMarkerColor(1);
  tree->Draw("pt>>hisAll(50,0,5)","0.01","");
  tree->SetLineColor(2);tree->SetMarkerColor(2);
  tree->Draw("pt>>hisT1W(50,0,5)","((triggerMask&0x1)>0)*max(weight,0.01)","same");
  tree->SetLineColor(4);tree->SetMarkerColor(4);
  tree->Draw("pt>>hisT2W(50,0,5)","((triggerMask&0x2)>0)*max(weight*pt**2,0.01)","same");
}



void testMCGener1(Int_t nPoints=100000){
  /// https://alice.its.cern.ch/jira/browse/ATO-575?focusedCommentId=284080&page=com.atlassian.jira.plugin.system.issuetabpanels:comment-tabpanel#comment-284080
  // nPoints=100000
  TTreeSRedirector * pcstream = new TTreeSRedirector("testDownsamplingTrigger.root","recreate");
  Float_t mass=0.139;
  Float_t sqrts=14400;
  float factor1Pt=0.01;
  float factorPt=0.01;
  TDatabasePDG * pdg  = TDatabasePDG::Instance();

  TF1 fts("fts"," TsalisCharged(x, [0], [1])/ TsalisCharged(1, [0], [1])",0.1,20);
  fts.SetNpx(100000);
  fts.SetParameters(mass,sqrts);
  const Int_t particlesPDG[]={211,321,2212, 3122, 311};
  TF1 * ftsArray[5];
  float massPDG[5],probPDG[5];
  const float T=0.150;
  for (Int_t i=0; i<5; i++) {
    massPDG[i]=pdg->GetParticle(particlesPDG[i])->Mass();
    ftsArray[i] = new TF1("fts"," TsalisCharged(x, [0], [1])/ TsalisCharged(1, [0], [1])",0.1,100);
    ftsArray[i]->SetParameters(massPDG[i],sqrts);
    ftsArray[i]->SetNpx(1000);
  }

  for (Int_t iLoop=0; iLoop<nPoints; iLoop++) {
    float maxProb=0;
    int   maxIndex=-1;
    int pdgCode=-1;
    float massG=-1;
    for (Int_t i=0; i<5; i++) {
      probPDG[i]= gRandom->Exp(T/massPDG[i]);
      if (probPDG[i]>maxProb) {
        maxProb=probPDG[i];
        maxIndex=i;
        pdgCode=particlesPDG[i];
        massG=massPDG[i];
      }
    }
    float pt=ftsArray[maxIndex]->GetRandom();
    //float pt = fts.GetRandom();
    double weight = 0;
    Int_t triggerMask = DownsampleTsalisCharged(pt, factorPt, factor1Pt, sqrts, mass, &weight);  // sownsampling with the default  mass - pion
    if (triggerMask<=0) continue;
    (*pcstream) << "trigger" <<
             "pt=" << pt <<                        // pt
             "pidCode="<<maxIndex<<
             "pdgCode="<< pdgCode<<
             "massG="<<massG<<
             "sqrts="<<sqrts<<
             "triggerMask=" << triggerMask <<      // triggerMask
             "weight=" << weight <<
             "\n";
  }
  delete pcstream;
}
