#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>
#include "TFile.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"

using namespace std;

void analyze_phsd() {
    gStyle->SetOptFit(1111);

    ifstream file("phsd.dat");
    if (!file.is_open()) {
        cerr << "Error: Cannot open phsd.dat file" << endl;
        return;
    }

    TH1D *h_b = new TH1D("h_b", "Impact parameter b distribution;b [fm];Counts", 50, 0, 6);
    TH1D *h_y_pi = new TH1D("h_y_pi", "Rapidity distribution of pi+;y;Counts", 100, -3, 3);
    TH2D *h_pt_y_L = new TH2D("h_pt_y_L", "p_{T} vs y distribution for Lambda;y;p_{T} [GeV/c]", 100, -2, 2, 100, 0, 3);
    TH1D *h_ekin_pi0 = new TH1D("h_ekin_pi0", "E_{kin} distribution for pi0;E_{kin} [GeV];dN/dE_{kin}", 100, 0, 2);

    string line;
    double last_b = -1.0;

    while (getline(file, line)) {
        istringstream iss(line);
        vector<string> tokens;
        string token;
        while (iss >> token) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 8) {
            // Identify the event header based on '1' at indices 2 and 4
            if (tokens[2] == "1" && tokens[4] == "1") {
                double b = stod(tokens[3]); // index 3 is the fourth column (b)
                if (b != last_b) {
                    h_b->Fill(b);
                    last_b = b;
                }
            } 
            // Particle data line
            else if (tokens.size() == 8) {
                try {
                    int pdg = stoi(tokens[0]);
                    double px = stod(tokens[2]);
                    double py = stod(tokens[3]);
                    double pz = stod(tokens[4]);
                    double E  = stod(tokens[5]);

                    if (pdg == 211) { // pi+
                        if (E > abs(pz)) {
                            double y = 0.5 * log((E + pz) / (E - pz));
                            h_y_pi->Fill(y);
                        }
                    } else if (pdg == 3122) { // Lambda
                        if (E > abs(pz)) {
                            double y = 0.5 * log((E + pz) / (E - pz));
                            double pt = sqrt(px*px + py*py);
                            h_pt_y_L->Fill(y, pt);
                        }
                    } else if (pdg == 111) { // pi0
                        double m = 0.134976; 
                        double ekin = E - m;
                        if (ekin > 0) {
                            h_ekin_pi0->Fill(ekin);
                        }
                    }
                } catch (...) {
                    // Ignore string->double conversion issues in malformed lines
                }
            }
        }
    }

    // Boltzmann distribution fit
    TF1 *f_boltz = new TF1("f_boltz", "[0] * sqrt((x+[2])*(x+[2]) - [2]*[2]) * (x+[2]) * exp(-(x+[2])/[1])", 0.05, 1.5);
    f_boltz->SetParameter(0, 1e5);
    f_boltz->SetParameter(1, 0.1); 
    f_boltz->FixParameter(2, 0.134976); 
    f_boltz->SetParName(0, "Normalization N");
    f_boltz->SetParName(1, "Temperature T");

    TCanvas *c1 = new TCanvas("c1", "PHSD Analysis Results", 1200, 1000);
    c1->Divide(2, 2);

    c1->cd(1); h_b->Draw();
    c1->cd(2); h_y_pi->Draw();
    c1->cd(3); h_pt_y_L->Draw("COLZ");
    c1->cd(4); h_ekin_pi0->Fit(f_boltz, "R"); h_ekin_pi0->Draw();

    double T_val = f_boltz->GetParameter(1);
    double T_err = f_boltz->GetParError(1);
    double chi2 = f_boltz->GetChisquare();
    double ndf = f_boltz->GetNDF();

    cout << "\n=======================================" << endl;
    cout << " Boltzmann Fit Results (pi0) " << endl;
    cout << "=======================================" << endl;
    cout << "Temperature T = " << T_val << " +- " << T_err << " GeV" << endl;
    cout << "chi2 / ndf    = " << chi2 << " / " << ndf << " = " << chi2/ndf << endl;
    cout << "=======================================\n" << endl;

    c1->SaveAs("phsd_plots.png");
    TFile *fout = new TFile("phsd_analysis.root", "RECREATE");
    h_b->Write(); h_y_pi->Write(); h_pt_y_L->Write(); h_ekin_pi0->Write(); c1->Write();
    fout->Close();
}