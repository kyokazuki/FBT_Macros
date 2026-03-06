void plotEffcyRate() {
    // Create a canvas INSIDE the function
    TCanvas *c1 = new TCanvas("c1", "Detection Efficiency", 800, 600);

    // Z=1, th=20
    const int n1 = 21;
    double x1[n1] = {0.05, 0.15, 0.25, 0.35, 0.45, 0.75, 1.25, 1.75, 2.25, 2.75, 3.25, 3.75, 4.25, 4.75, 5.25, 5.75, 6.25, 6.75, 7.25, 7.75, 8.25};
    double y1[n1] = {0.953948, 0.948626, 0.948559, 0.949204, 0.949268, 0.944664, 0.932635, 0.926324, 0.922702, 0.919821, 0.927453, 0.931214, 0.946325, 0.949514, 0.942327, 0.938231, 0.901025, 0.902002, 0.845461, 0.763389, 0.727536};

    TGraph *g1 = new TGraph(n1, x1, y1);
    g1->SetLineColor(kRed);
    g1->SetMarkerColor(kRed);
    g1->SetMarkerStyle(20);
    g1->SetLineWidth(2);
    g1->Draw("ALP");

    // Title + axis labels (this is the ROOT convention)
    g1->SetTitle("Detection Efficiency;rate [MHz];Efficiency");
    // Legend
    // TLegend *leg = new TLegend(0.7, 0.75, 0.9, 0.9);
    // leg->AddEntry(g1, "Z=1; th=20", "lp");
    // leg->Draw();

	c1->SetGrid();
    c1->Modified();
    c1->Update();
}
