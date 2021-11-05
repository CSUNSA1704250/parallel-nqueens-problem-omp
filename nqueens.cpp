#include <iostream>
#include <fstream>
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
    vector<vector<string>> matriz(cantidad);
    for (int i = 0; i < cantidad; i++)
    {
        matriz[i].resize(cantidad);
    }

    for (int i = 0; i < cantidad; i++)
    {
        matriz[queens[i]][i] = "&#9813;";
    }

    string salida = "digraph D {\n";

    salida = salida + "    node [shape=plaintext]\n  some_node [ \n  label=< \n  <table border=\"0\" cellborder=\"1\" cellspacing=\"0\"> \n ";

    for (int i = 0; i < cantidad; i++)
    {
        salida = salida + " <tr>";
        for (int j = 0; j < cantidad; j++)
        {
            salida = salida + "<td>" + matriz[i][j] + " </td>";
        }
        salida = salida + " </tr>\n";
    }

    salida = salida + "</table>>];\n }";

    ofstream file;
    file.open("graph.dot");
    file << salida;
    file.close();
}

bool is_safe(int *&queens, int &row, int &col)
{
    for (int i = 0; i < col; ++i)
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
    for (int i = 0; i < queens.size(); ++i)
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

void try_queen(int *&queens, int col, int &solutions_count, string &solutions)
{
    if (col == N)
    {
        ++solutions_count;
        string temp = "";
        for (int i = 0; i < N; ++i)
            temp += to_string(queens[i] + 1) + " ";
        solutions += temp + "\n";
        return;
    }

    for (int i = 0; i < N; ++i)
        if (is_safe(queens, i, col))
        {
            queens[col] = i;
            try_queen(queens, col + 1, solutions_count, solutions);
        }
}

size_t find_all_solutions()
{
    size_t num_solutions = 0;
    string solutions = "";
    int col = 0;
    #pragma omp parallel
    {
        int *priv_queens = new int[N];
        int priv_num_solutions = 0;
        string priv_solutions = "";
        #pragma omp for nowait
        for (int i = 0; i < N; ++i)
        {
            priv_queens[col] = i;
            try_queen(priv_queens, col + 1, priv_num_solutions, priv_solutions);
        }
        #pragma omp atomic
        num_solutions += priv_num_solutions;
        solutions += priv_solutions;
        delete[] priv_queens;
    }

    string solutions_content = "#Solutions for " + to_string(N) + " queens\n";
    solutions_content += to_string(num_solutions) + "\n";

    ofstream solutions_file;
    solutions_file.open("solutions.txt");
    solutions_file << solutions_content;
    solutions_file << solutions;
    solutions_file.close();
    return num_solutions;
}

void try_queen_one_solution(int *&queens, int col)
{
    if (found)
        return;
    if (col == N && !found)
    {
        found = true;
        generate_dot(queens);
        return;
    }

    for (int i = 0; i < N; ++i)
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
        for (int i = 0; i < N; ++i)
        {
            priv_queens[col] = i;
            try_queen_one_solution(priv_queens, col + 1);
        }
        delete[] priv_queens;
    }
}

int main(int argc, char *argv[])
{

    string problemType = get_cmd_option(argv, argc + argv, "-problemType");
    int n = stoi(get_cmd_option(argv, argc + argv, "-N"));
    /* No error checking */

    N = n;
    /* omp_set_num_threads(4); */
    /* auto timer_start = chrono::high_resolution_clock::now(); */

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

    /* auto timer_end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> total_time = timer_end - timer_start;

    cout << "Time: " << total_time.count() << "ms" << endl; */

    return 0;
}
