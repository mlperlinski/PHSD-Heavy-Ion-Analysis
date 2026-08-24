# PHSD Heavy Ion Collisions Analysis

This repository contains a complete simulation and data analysis workflow for heavy-ion collisions using the PHSD (Parton-Hadron-String Dynamics) transport model and ROOT.

## Project Overview
The project involves simulating **63Cu + 63Cu** collisions with a beam kinetic energy of **3.5 AGeV** and an impact parameter range of **[0, 5] fm**. The simulation generates 500 main events, each consisting of 50 parallel ensembles, running up to a maximum time of $t_{max} = 35$ fm/c.

The pipeline consists of two main parts:
1. Shell scripting (`awk`) for parsing raw textual outputs, extracting event statistics, and calculating particle production rates with Poisson uncertainties.
2. C++ ROOT macro for extracting kinematic distributions (rapidity, transverse momentum) and applying a Boltzmann distribution fit to derive thermal properties (Temperature).

## Requirements
- **PHSD Transport Model** executable for data generation.
- **Bash / awk** for text file processing.
- **CERN ROOT** (C++ framework) for data visualization and fitting.

## Running the Simulation
Use the provided `inputPHSD` parameters and run the transport model in the background:

```bash
nohup ./phsd > simulation.log 2>&1 &

```

This generates the primary output file, typically named `phsd.dat`.

## Shell/Awk Pre-Analysis

Before diving into ROOT, we can extract basic multiplicity numbers directly from the `phsd.dat` file.

**Confirming the number of events (removing 50x parallel duplicates):**

```bash
awk '$3 == "1" && $5 == "1" {print $4}' phsd.dat | uniq | wc -l

```

**Counting generated K+, Sigma0, and eta0 particles with Poisson uncertainties:**

```bash
awk '
BEGIN { k=0; s=0; e=0; }
NF > 5 {
    if ($1 == 321) k++;
    if ($1 == 3212) s++;
    if ($1 == 221) e++;
}
END {
    printf("N(K+) = %d +- %.1f\n", k, sqrt(k));
    printf("N(Sigma0) = %d +- %.1f\n", s, sqrt(s));
    printf("N(eta0) = %d +- %.1f\n", e, sqrt(e));
}' phsd.dat

```

*Example Results:*

* N(K+) = 18032 +- 134.3
* N(Sigma0) = 9273 +- 96.3
* N(eta0) = 26315 +- 162.2

## Kinematic Analysis with ROOT

The `analyze_phsd.cpp` macro parses the output file to generate the following kinematic representations:

* **Impact Parameter (b):** Distribution verifying the proper generation of collision geometries.
* **pi+ Rapidity (y):** Verifying if the rapidity distribution is centered at 0.
* **Lambda Hyperons (pT vs y):** 2D population map in transverse momentum vs rapidity space.
* **pi0 Kinetic Energy (E_kin):** Boltzmann distribution fit applied to extract the apparent temperature (T), derived via:
$dN/dE_{kin} = N \cdot p \cdot E \cdot \exp(-E/T)$

### Running the Macro

Launch ROOT to execute the analysis script:

```bash
root -l -b -q analyze_phsd.cpp

```

### Outputs

The macro generates:

* `phsd_analysis.root`: A ROOT file containing all histogram objects.
* `phsd_plots.png`: A 4-panel image showcasing the distributions and the Boltzmann fit.
