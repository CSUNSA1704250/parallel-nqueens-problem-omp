#include <iostream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <algorithm>

using namespace std;

int N;

bool found = false;

char *get_cmd_option(char **begin, char **end, const std::string &option)
{
    char **itr = find(begin, end, option);
    if (itr != end && ++itr != end)
    {
        return *itr;
    }
    return 0;
}

bool cmd_option_exists(char **begin, char **end, const std::string &option)
{
    return std::find(begin, end, option) != end;
}

void generate_dot(int *&queens)
{
    int cantidad = N;
    vector<vector<int>> matriz(cantidad);
    for (int i = 0; i < cantidad; i++)
    {
        matriz[i].resize(cantidad);
    }

    for (int i = 0; i < cantidad; i++)
    {
        matriz[queens[i]][i] = 1;
    }
    string salida = "digraph structs {\n";

    salida = salida + "    node [shape=record];\n     struct3 [label=\"{";

    for (int i = 0; i < cantidad; i++)
    {
        salida = salida + " { ";
        for (int j = 0; j < (cantidad - 1); j++)
        {
            salida = salida + to_string(matriz[i][j]) + "|";
        }
        if (i == (cantidad - 1))
        {
            salida = salida + to_string(matriz[i][cantidad - 1]) + " } ";
        }
        else
        {
            salida = salida + to_string(matriz[i][cantidad - 1]) + " } |";
        }
    }

    salida = salida + "}\"];\n }";
    cout << salida << endl;
}

bool is_safe(int *&queens, int &row, int &col)
{
    for (int i = 0; i < col; i++)
    {
        if (queens[i] == row)
            return false;
        if (abs(queens[i] - row) == col - i)
            return false;
    }

    return true;
}

void print_solution(vector<int> &queens)
{
    cout << "Solution:" << endl;
    for (int i = 0; i < queens.size(); i++)
    {
        for (int j = 0; j < queens.size(); j++)
            if (queens[i] == j)
                cout << "x"
                     << " ";
            else
                cout << "_"
                     << " ";
        cout << endl;
    }
}

void try_queen(int *&queens, int col, int &solutions_count)
{
    if (col == N)
    {
        ++solutions_count;
        /* print_solution(queens); */
        generate_dot(queens);
        return;
    }

    for (int i = 0; i < N; i++)
        if (is_safe(queens, i, col))
        {
            queens[col] = i;
            try_queen(queens, col + 1, solutions_count);
        }
}

int find_all_solutions()
{
    int num_solutions = 0;
    int col = 0;
#pragma omp parallel
    {
        int *priv_queens = new int[N];
        int priv_solutions = 0;
#pragma omp for nowait
        for (int i = 0; i < N; i++)
        {
            if (is_safe(priv_queens, i, col))
            {
                priv_queens[col] = i;
                try_queen(priv_queens, col + 1, priv_solutions);
            }
        }
#pragma omp atomic
        num_solutions += priv_solutions;
        delete[] priv_queens;
    }
    return num_solutions;
}

void try_queen_one_solution(int *&queens, int col)
{
    if (found)
        return;
    if (col == N && !found)
    {
        /* print_solution(queens); */
        found = true;
        generate_dot(queens);
        return;
    }

    for (int i = 0; i < N; i++)
        if (is_safe(queens, i, col))
        {
            queens[col] = i;
            try_queen_one_solution(queens, col + 1);
        }
}

void find_a_solution()
{
    int col = 0;
#pragma omp parallel
    {
        int *priv_queens = new int[N];
#pragma omp for nowait
        for (int i = 0; i < N; i++)
        {
            if (is_safe(priv_queens, i, col))
            {
                priv_queens[col] = i;
                try_queen_one_solution(priv_queens, col + 1);
            }
        }
        delete[] priv_queens;
    }
}

int main(int argc, char *argv[])
{
    omp_set_num_threads(4);

    string problemType = get_cmd_option(argv, argc + argv, "-problemType");
    int n = stoi(get_cmd_option(argv, argc + argv, "-N"));
    /* No error checking */

    N = n;
    auto timer_start = chrono::high_resolution_clock::now();

    if (problemType == "all")
    {
        int solutions_count = find_all_solutions();
        cout << "Number of solutions: " << solutions_count << endl;
    }
    else if (problemType == "find")
    {
        find_a_solution();
    }
    else
    {
        cout << "problem type not valid" << endl;
        return -1;
    }

    auto timer_end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> total_time = timer_end - timer_start;

    cout << "Time: " << total_time.count() << "ms" << endl;

    return 0;
}
