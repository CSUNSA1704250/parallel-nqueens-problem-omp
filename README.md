## Integrantes:
- Jhoel Salomon Tapara Quispe
- Diego Portocarrero Espirilla

## Compile
g++ -o nqueens nqueens.cpp -fopenmp -O2

## Execute
./nqueens -problemType [all,find] -N <num_queens>

## Ejemplo de ejecución
./nqueens -problemType all -N 13

## Generate solution plot
dot -Tpng -O graph.dot
