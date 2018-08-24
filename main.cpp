#include <mpi.h>
#include <iostream>
#include <random>
#include "gnuplot.h"


// Settings for the sampler
#define STARTING_SAMPLE -1.0
#define LOWLIMU 0.0
#define UPPLIMU 1.0
#define COUPLING true
#define COUPLING_INTERVAL 100
#define PROPOSALS 100000
#define WALKSTEP0 0.5
#define WALKSTEP1 0.05

// Target function
double doubleWell(double x, double gamma = 4) {
    return gamma * (x * x - 1) * (x * x - 1);
}

int main() {
    // MPI initialization
    MPI_Init(NULL, NULL);
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // RNG's for proposal and acceptance rule
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> nDis(0.0, (world_rank == 0 ? WALKSTEP0 : WALKSTEP1));
    std::uniform_real_distribution<double> uDis(LOWLIMU, UPPLIMU);

    // Variables for sampling
    unsigned long nproposals = PROPOSALS;
    double current;
    double proposal;
    double currentX;
    double proposalX;
    int accepted = 0;
    int acceptedSwitches = COUPLING ? 0 : int();

    // Storage objects
    std::vector<double> samples(nproposals);
    std::vector<double> misfits(nproposals);

    // Startings sample
    samples[0] = STARTING_SAMPLE;

    // Well function
    int gamma = (world_rank == 0) ? 1 : 30;

    currentX = doubleWell(samples[0], gamma);

    for (int i = 1; i < nproposals; i++) {
        // Random walk update
        current = samples[i - 1];
        proposal = current + nDis(gen);
        proposalX = doubleWell(proposal, gamma);
        double exponentWalk = exp((currentX - proposalX));
        if (exponentWalk > uDis(gen)) {
            samples[i] = proposal;
            misfits[i] = proposalX;
            current = proposal;
            currentX = proposalX;
            accepted++;
        } else {
            samples[i] = current;
            misfits[i] = currentX;
        }

        if (COUPLING and i % COUPLING_INTERVAL == 0) {
            // Coupling update
            i++;
            double incoming;
            double gamma1 = 1 * 20 + 1;

            if (world_rank == 0) {
                MPI_Recv(&incoming, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                double exponentCoupling = exp(doubleWell(current, gamma) + doubleWell(incoming, gamma1)
                                              - doubleWell(incoming, gamma) - doubleWell(current, gamma1));

                if (exponentCoupling > uDis(gen)) {
                    acceptedSwitches++;
                    // Replica exchange accepted
                    // Send own replica back, update current to incoming
                    MPI_Send(&current, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
                    current = incoming;
                    currentX = doubleWell(current, gamma);
                } else {
                    // Replica exchange rejected
                    // Send original back, don't update current sample.
                    MPI_Send(&incoming, 1, MPI_DOUBLE, 1, 0, MPI_COMM_WORLD);
                }
                samples[i] = current;
                misfits[i] = currentX;
            } else {
                MPI_Send(&samples[i - 1], 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
                MPI_Recv(&current, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                samples[i] = current;
                currentX = doubleWell(current, gamma);
                misfits[i] = currentX;
            }

            MPI_Barrier(MPI_COMM_WORLD);
        }
    }

    std::cout << "Chain " << world_rank << ": " << accepted << std::endl;
    if (COUPLING and world_rank == 0) std::cout << "Accepted switches: " << acceptedSwitches << std::endl;

    char samplesFilename[50];
    sprintf(samplesFilename, "samples%d.dat", world_rank);
    std::ofstream ofs(samplesFilename, std::ofstream::out);

    for (int i = 0; i < nproposals; ++i) {
        ofs << samples[i] << "\n";
    }

    ofs.close();


    MPI_Finalize();
    return 0;
}