#include <iostream>
#include <mpi.h>
int main(int argc, char *argv[]) {
  int rank, size, N = 100;
  int local_sum = 0, global_sum = 0;
  int start, end;
  // Inicializar MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);
  // Dividir el trabajo entre los procesos
  int range = N / size;
  start = rank * range + 1;
  end = (rank == size - 1) ? N : (rank + 1) * range;
  // Cada proceso calcula su suma local
  for (int i = start; i <= end; ++i) {
    local_sum += i;
  }
  // Reducir las sumas locales a la suma global
  MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
  // Imprimir la suma global en el proceso maestro (rank 0)
  if (rank == 0) {
    std::cout << "La suma de los números enteros de 1 a " << N << " es " << global_sum << std::endl;
  }

  // Finalizar MPI
  MPI_Finalize();

  return 0;
}

