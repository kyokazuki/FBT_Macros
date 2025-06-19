// #include <TCanvas.h>
#include <TH1F.h>

// #include <.h>

TH1F *hdx = new TH1F("hdx", "hdx", 32, -0.5, 31.5);

void gen() {
	for (Int_t i=1; i<=1000; i++) {
		Int_t rand1 = rand()%32 + 1;
		Int_t rand2 = rand()%32 + 1;
		hdx->Fill(abs(rand1 - rand2));
	}
	hdx->Draw();
}

