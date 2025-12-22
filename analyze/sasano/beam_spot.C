#include <TTree.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <iostream>

void beam_spot(TTree* events){

  gStyle->SetOptStat(11111111);
  TCanvas *c2 = new TCanvas("c2","c2"); 
  c2->Divide(2,2);
  c2->cd(1);
  events->Draw("(xiX+xiY)/sqrt(2)-xiU:xiU>>h1(400,0,400,400,-200,200)","","colz");
  c2->cd(2);
  events->Draw("(xiX+xiY)/sqrt(2)-xiU:xiU>>h2(400,0,400,400,-200,200)","abs((xiX+xiY)/sqrt(2)-xiU-30)<25","colz");
  c2->cd(3);
  events->Draw("(xiX-xiY)/sqrt(2):xiU>>h3(400,0,400,400,-200,200)","","col");
  c2->cd(4);
  events->Draw("(xiX-xiY)/sqrt(2):xiU>>h4(400,0,400,400,-200,200)","abs((xiX+xiY)/sqrt(2)-xiU-30)<25","colz");

  Int_t num_total = events->GetEntries();
  Int_t num_detected = events->GetEntries("abs((xiX+xiY)/sqrt(2)-xiU-30)<25");

  std::cout << "Number of total events : " << num_total << std::endl;
  std::cout << "Number of detected events : " << num_detected << std::endl;
  std::cout << "Ratio : " << Form("%7.3f",100.*Double_t(num_detected)/Double_t(num_total)) << std::endl;

}
