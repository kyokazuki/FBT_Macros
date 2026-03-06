void plotEffcyOV() {
    // Create a canvas INSIDE the function
    TCanvas *c1 = new TCanvas("c1", "Detection Efficiency", 800, 600);

    // Z=1, th=20
    const int n1 = 12;
    double x1[n1] = {1.0, 1.2, 1.4, 1.6, 1.8, 2.0, 2.4, 2.6, 2.8, 3.0, 3.2, 3.4};
    double y1[n1] = {0.000000, 0.000084, 0.001322, 0.039624, 0.266364, 0.609585, 0.894215, 0.921347, 0.928024, 0.931789, 0.930643, 0.922593};

    TGraph *g1 = new TGraph(n1, x1, y1);
    g1->SetLineColor(kOrange);
    g1->SetMarkerColor(kOrange);
    g1->SetMarkerStyle(20);
    g1->SetLineWidth(2);
    g1->Draw("ALP");

    // Z=1, th=40
    // const int n2 = 8;
    // double x2[n2] = {1.6, 1.8, 2.2, 2.4, 2.6, 3.0, 3.2, 3.4};
    // double y2[n2] = {0.001163, 0.010912, 0.263520, 0.515752, 0.720129, 0.885578, 0.909029, 0.919384};
    //
    // TGraph *g2 = new TGraph(n2, x2, y2);
    // g2->SetLineColor(kBlue);
    // g2->SetMarkerColor(kBlue);
    // g2->SetMarkerStyle(21);
    // g2->SetLineWidth(2);
    // g2->Draw("LP SAME");

    // Z=2, th=20
    const int n3 = 19;
    double x3[n3] = {1.1, 1.2, 1.3, 1.4, 1.5, 1.6, 1.7, 1.8, 1.9, 2.0, 2.1, 2.2, 2.3, 2.4, 2.5, 2.6, 2.7, 2.8, 2.9};
    double y3[n3] = {0.000025, 0.000344, 0.046683, 0.248138, 0.634113, 0.875600, 0.918262, 0.923775, 0.924426, 0.924419, 0.921804, 0.925887, 0.925461, 0.926731, 0.924666, 0.924652, 0.923664, 0.923914, 0.924391};

    TGraph *g3 = new TGraph(n3, x3, y3);
    g3->SetLineColor(kRed);
    g3->SetMarkerColor(kRed);
    g3->SetMarkerStyle(22);
    g3->SetLineWidth(2);
    g3->Draw("LP SAME");

    // Title + axis labels (this is the ROOT convention)
    g1->SetTitle("Detection Efficiency;Overvoltage[V];Efficiency");
    // Legend
    TLegend *leg = new TLegend(0.7, 0.75, 0.9, 0.9);
    leg->AddEntry(g1, "Z=1; th=20", "lp");
    // leg->AddEntry(g2, "Z=1; th=40", "lp");
    leg->AddEntry(g3, "Z=2; th=20", "lp");
    leg->Draw();

	c1->SetGrid();
    c1->Modified();
    c1->Update();
}
