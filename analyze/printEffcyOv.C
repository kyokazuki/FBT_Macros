void printEffcyOv() {
    // Create a canvas INSIDE the function
    TCanvas *c1 = new TCanvas("c1", "Detection Efficiency", 1500, 400);
	c1->Divide(3, 1);

	// S074 Z=1
    const int points = 12;
	Double_t ov[points] = {2.2, 2.4, 2.6, 2.7, 2.8, 2.9, 3.1, 3.2, 3.3, 3.4, 3.5, 3.6};
	vector<vector<Double_t>> effcy = {
		{0.0216, 0.1066, 0.3236, 0.4552, 0.5837, 0.6795, 0.8005, 0.8308, 0.8525, 0.8682, 0.8727, 0.8781}, 
		{0.0806, 0.2795, 0.5362, 0.6415, 0.7197, 0.778, 0.8403, 0.8511, 0.8641, 0.8687, 0.8725, 0.8736},
		{0.0361, 0.1414, 0.3389, 0.4503, 0.5485, 0.631, 0.7428, 0.7778, 0.8013, 0.8158, 0.8266, 0.8341}
	};
	vector<vector<Double_t>> effcyHodo = {
		{0.0117, 0.0985, 0.3283, 0.4680, 0.6047, 0.7064, 0.8352, 0.8689, 0.8917, 0.9065, 0.9127, 0.9179},
		{0.0744, 0.2849, 0.5576, 0.6678, 0.7508, 0.8132, 0.8789, 0.8920, 0.9041, 0.9084, 0.9135, 0.9144},
		{0.0258, 0.1347, 0.3444, 0.4613, 0.5656, 0.6536, 0.7737, 0.8126, 0.8365, 0.8514, 0.8637, 0.8711}
	};
	vector<vector<Double_t>> effcyScaled = {
		{0.1276, 0.4146, 0.6903, 0.7695, 0.8252, 0.8524, 0.8767, 0.8795, 0.8845, 0.8879, 0.8866, 0.8879},
		{0.3376, 0.6355, 0.7964, 0.8323, 0.8524, 0.8653, 0.8760, 0.8746, 0.8801, 0.8798, 0.8799, 0.8802},
		{0.2316, 0.5071, 0.6998, 0.7551, 0.7929, 0.8149, 0.8373, 0.8424, 0.8476, 0.8473, 0.8483, 0.8496}
	};
	vector<vector<Double_t>> effcyScaledHodo = {
		{0.1214, 0.4258, 0.7195, 0.8037, 0.8622, 0.8904, 0.9163, 0.9211, 0.9254, 0.9274, 0.9275, 0.9282},
		{0.3461, 0.6630, 0.8343, 0.8712, 0.8917, 0.9057, 0.9163, 0.9169, 0.9210, 0.9201, 0.9212, 0.9213},
		{0.2306, 0.5239, 0.7300, 0.7879, 0.8276, 0.8508, 0.8741, 0.8817, 0.8858, 0.8849, 0.8868, 0.8873}
	};

	vector<TGraph*> gEffcy(3);
	vector<TGraph*> gEffcyHodo(3);
	vector<TGraph*> gEffcyScaled(3);
	vector<TGraph*> gEffcyScaledHodo(3);
	const char LAYERS[3] = {'X', 'Y', 'U'};
	for (Int_t i = 0; i < 3; i++) {
		c1->cd(i + 1);
		gPad->SetGrid();

		gEffcy[i] = new TGraph(points, ov, effcy[i].data());
		gEffcyHodo[i] = new TGraph(points, ov, effcyHodo[i].data());
		gEffcyScaled[i] = new TGraph(points, ov, effcyScaled[i].data());
		gEffcyScaledHodo[i] = new TGraph(points, ov, effcyScaledHodo[i].data());

		gEffcy[i]->SetTitle(Form("Effcy%c vs OV (th=20, tot={50e3, 1e6} or {0.5, 10}, id=6, q={2, 5});Overvoltage [V];Efficiency", LAYERS[i]));
		gEffcy[i]->GetYaxis()->SetRangeUser(0, 1);
		gEffcy[i]->SetLineColor(kGreen);
		gEffcy[i]->SetMarkerStyle(20);
		gEffcy[i]->SetLineWidth(1);
		gEffcy[i]->Draw("ALP");

		gEffcyHodo[i]->SetLineColor(kRed);
		gEffcyHodo[i]->SetMarkerStyle(20);
		gEffcyHodo[i]->SetLineWidth(1);
		gEffcyHodo[i]->Draw("LP SAME");

		gEffcyScaled[i]->SetLineColor(kOrange);
		gEffcyScaled[i]->SetMarkerStyle(20);
		gEffcyScaled[i]->SetLineWidth(1);
		gEffcyScaled[i]->Draw("LP SAME");

		gEffcyScaledHodo[i]->SetLineColor(kBlue);
		gEffcyScaledHodo[i]->SetMarkerStyle(20);
		gEffcyScaledHodo[i]->SetLineWidth(1);
		gEffcyScaledHodo[i]->Draw("LP SAME");

		TLegend *leg = new TLegend(0.60, 0.15, 0.88, 0.30);
		leg->AddEntry(gEffcy[i],     "No Hodo", "lp");
		leg->AddEntry(gEffcyHodo[i], "With Hodo", "lp");
		leg->AddEntry(gEffcyScaled[i], "No Hodo (scaled)", "lp");
		leg->AddEntry(gEffcyScaledHodo[i], "With Hodo (scaled)", "lp");

		leg->Draw();
	}
}
