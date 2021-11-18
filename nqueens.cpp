#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <algorithm>
#include <list>
#include <bitset>

using namespace std;

int N;
list<vector<int>> solutions;

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

void generate_dot(vector<int> &queens)
{
    ofstream file;
    file.open("graph.dot");
    file << "digraph D{\n";

    file << "    node [shape=plaintext]\n  some_node [ \n  label=< \n  <table border=\"0\" cellborder=\"1\" cellspacing=\"0\"> \n ";

    for (int i = 0; i < N; i++)
    {
        file << " <tr>";
        for (int j = 0; j < N; j++)
        {
            file << "<td>";
            if (i == queens[j])
                file << "&#9813;";
            file << "</td>";
        }
        file << " </tr>\n";
    }

    file << "</table>>];\n }";

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
        for (int j = 0; j < queens.size(); ++j)
            if (queens[i] == j)
                cout << "x"
                     << " ";
            else
                cout << "_"
                     << " ";
        cout << endl;
    }
}

void try_queen(vector<int> &queens, int col, int &solutions_count)
{
    if (col == N)
    {
        ++solutions_count;
        #pragma critical
        {
            solutions.push_back(queens);
        }
        return;
    }

    bool safe;
    for (int i = 0; i < N; ++i)
    {
        safe = true;
        for (int j = 0; j < col; ++j)
        {
            if (queens[j] == i)
            {
                safe = false;
                break;
            }
            if (abs(queens[j] - i) == col - j)
            {
                safe = false;
                break;
            }
        }
        if (safe)
        {
            queens[col] = i;
            try_queen(queens, col + 1, solutions_count);
        }
    }
}

void try_queen_bs(vector<int> &queens, bitset<40> &rows, bitset<40> &d1, bitset<40> &d2, int col, int &solutions_count)
{
    if (col == N)
    {
        ++solutions_count;
        #pragma critical
        {
            solutions.push_back(queens);
        }
        return;
    }

    for (int r = 0; r < N; ++r)
        if (!rows[r] && !d1[r - col + N - 1] && !d2[r + col])
        {
            queens[col] = r;
            rows[r] = d1[r - col + N - 1] = d2[r + col] = 1;
            try_queen_bs(queens, rows, d1, d2, col + 1, solutions_count);
            rows[r] = d1[r - col + N - 1] = d2[r + col] = 0;
        }
    
}

size_t find_all_solutions()
{
    size_t num_solutions = 0;
    string solutions_txt = "";
    int col = 0;
    #pragma omp parallel
    #pragma omp single
    {
        for (int i = 0; i < N; ++i)
        {
            #pragma omp task
            {
                vector<int> priv_queens(N);
                int priv_num_solutions = 0;
                priv_queens[col] = i;
                try_queen(priv_queens, col + 1, priv_num_solutions);
                
                #pragma omp atomic
                num_solutions += priv_num_solutions;
            }
        }
    }

    ofstream solutions_file;
    solutions_file.open("solutions.txt");

    solutions_file << "#Solutions for " << N << " queens\n";
    solutions_file << num_solutions << "\n";
    for (auto &&i : solutions)
    {
        for (auto &&j : i)
            solutions_file << j << " ";
        solutions_file << "\n";
    }
    solutions_file.close();
    return num_solutions;
}

size_t find_all_solutions_bs()
{
    size_t num_solutions = 0;
    int col = 0;
    #pragma omp parallel
    #pragma omp single
    {
        for (int i = 0; i < N; ++i)
        {
            #pragma omp task
            {
                vector<int> priv_queens(N);
                int priv_num_solutions = 0;
                bitset<40> rows, d1, d2;

                priv_queens[col] = i;
                rows[i] = d1[i - col + N - 1] = d2[i + col] = 1;
                try_queen_bs(priv_queens, rows, d1, d2, col + 1, priv_num_solutions);
                
                #pragma omp atomic
                num_solutions += priv_num_solutions;
            }
        }
    }

    ofstream solutions_file;
    solutions_file.open("solutions.txt");

    solutions_file << "#Solutions for " << N << " queens\n";
    solutions_file << num_solutions << "\n";
    for (auto &&i : solutions)
    {
        for (auto &&j : i)
            solutions_file << j << " ";
        solutions_file << "\n";
    }
    solutions_file.close();
    return num_solutions;
}

void try_queen_one_solution(vector<int> &queens, bitset<40> &rows, bitset<40> &d1, bitset<40> &d2, int col)
{
    if (found)
        return;
    if (col == N && !found)
    {
        found = true;
        generate_dot(queens);
        return;
    }

    for (int r = 0; r < N; ++r)
        if (!rows[r] && !d1[r - col + N - 1] && !d2[r + col])
        {
            queens[col] = r;
            rows[r] = d1[r - col + N - 1] = d2[r + col] = 1;
            try_queen_one_solution(queens, rows, d1, d2, col + 1);
            rows[r] = d1[r - col + N - 1] = d2[r + col] = 0;
        }
}

void find_a_solution()
{
    int col = 0;
    #pragma omp parallel
    {
        vector<int> priv_queens(N);
        bitset<40> rows, d1, d2;
        #pragma omp for nowait
        for (int i = 0; i < N; ++i)
        {
            priv_queens[col] = i;
            rows[i] = d1[i - col + N - 1] = d2[i + col] = 1;
            try_queen_one_solution(priv_queens, rows, d1, d2, col + 1);
        }
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
        size_t solutions_count = find_all_solutions_bs();
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
