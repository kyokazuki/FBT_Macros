#include <TCanvas.h>
#include <TH1D.h>
#include <TRandom3.h>
#include <iostream>
#include <cmath>

TH1D *hStdDev;

void simGausStdDev(Double_t sigma = 1.0, Double_t mu = 0.0, Int_t NMC = 1000000) {
    // ------------------------------------------------------------
    // Parameters
    // ------------------------------------------------------------

    const int N   = 3;        // Only 3 events per trial

    // Random number generator
    TRandom3 rng(0);

    // ------------------------------------------------------------
    // Histogram of calculated standard deviations
    // ------------------------------------------------------------

	delete hStdDev;
    hStdDev = new TH1D(
        "hStdDev",
        "Standard Deviation from 3 Events;"
        "Measured Standard Deviation;Number of Trials",
        200,
        0.0,
		1000
        // sigma * 3
    );

    // ------------------------------------------------------------
    // Monte Carlo
    // ------------------------------------------------------------

    for (int trial = 0; trial < NMC; trial++)
    {
        // Generate exactly 3 events from one Gaussian
        double x[3];

        for (int i = 0; i < N; i++)
        {
            x[i] = rng.Gaus(mu, sigma);
        }

        // Calculate the mean of the 3 events
        double mean = 0.0;

        for (int i = 0; i < N; i++)
        {
            mean += x[i];
        }

        mean /= N;

        // Calculate sample variance
        double variance = 0.0;

        for (int i = 0; i < N; i++)
        {
            variance += (x[i] - mean) * (x[i] - mean);
        }

        // variance /= (N - 1);
        variance /= N;

        // Standard deviation
        double measuredSigma = sqrt(variance);

        // Fill histogram
        hStdDev->Fill(measuredSigma);
    }

    // ------------------------------------------------------------
    // Print results
    // ------------------------------------------------------------

    std::cout << "========================================" << std::endl;
    std::cout << "Monte Carlo Standard Deviation" << std::endl;
    std::cout << "========================================" << std::endl;

    std::cout << "True sigma              = "
              << sigma << std::endl;

    std::cout << "Number of events/trial  = "
              << N << std::endl;

    std::cout << "Number of MC trials     = "
              << NMC << std::endl;

    std::cout << "Mean measured sigma     = "
              << hStdDev->GetMean() << std::endl;

    std::cout << "RMS of sigma distribution = "
              << hStdDev->GetStdDev() << std::endl;

    // ------------------------------------------------------------
    // Plot
    // ------------------------------------------------------------

    TCanvas *c1 = new TCanvas(
        "c1",
        "Monte Carlo Standard Deviation",
        800,
        600
    );

    hStdDev->SetLineWidth(2);
    hStdDev->SetStats(1);

	c1->SetGrid();
    hStdDev->Draw();

    c1->Update();
}
