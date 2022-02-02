import ROOT
import pandas as pd

def convertNtuple(nt):
    data, columns = nt.AsMatrix(return_labels=True)
    df = pd.DataFrame(data=data, columns=columns)
    return df

class prepDataObject():
    
    def __init__(self):
        self.f = ROOT.TFile.Open("Train_554.root")
        d = self.f.Get("AntiHe3HM")
        self.Ntuple_triton  = d.Get("MyOutputContainerHighMult").At(1).At(28)
        self.Ntuple_antitriton  = d.Get("MyOutputContainerHighMult").At(2).At(28)
        self.df_triton = convertNtuple(self.Ntuple_triton)
        self.df_antitriton = convertNtuple(self.Ntuple_antitriton)


        
    def drawNtupleTPC(self, string="h"):
        self.h = ROOT.TH2F(string, string, 200, 0, 10, 200, 0, 1500)
        self.Ntuple_triton.Draw("TPCSignal:p>>%s" % string)
        self.c = ROOT.TCanvas("c", "c", 700, 700)
        self.h.Draw("colz")
        self.c.Draw()
        return self.c