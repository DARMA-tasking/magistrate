#include <gtest/gtest.h>

#include <mpich-clang39/mpi.h>

class MPIEnvironment : public ::testing::Environment
{
public:
  virtual void SetUp() override{
    char** argv;
    int argc = 0;
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);

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
    //ASSERT_FALSE(mpiError);
  }

  virtual void TearDown() override{
    int mpiError = MPI_Finalize();
   // ASSERT_FALSE(mpiError);
  }
  virtual ~MPIEnvironment() {std::cout << "SetUpSetUpSetUpSetUp" << std::endl;}
};
