#ifndef MPIINIT_H
#define MPIINIT_H

#include <gtest/gtest.h>
#include <test_harness.h>
#include <mpi.h>

class MPIEnvironment : public ::testing::Environment
{
public:
  MPIEnvironment (int *argc, char **argv) :argc_(argc), argv_(argv) {}
  virtual void SetUp() override{
    // Initialize the MPI environment
    MPI_Init(argc_, &argv_);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world from processor %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);
  }

  virtual void TearDown() override{
    int mpiError = MPI_Finalize();
  }
  virtual ~MPIEnvironment() {}


  static bool isRankValid(int rank)
  {
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);

    return rank < world_size;
  }


private:
  int *argc_;
  char **argv_;

};

#endif // MPIINIT_H
