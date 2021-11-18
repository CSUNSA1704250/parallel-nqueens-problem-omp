#include <iostream>
#include <fstream>
#include <vector>
#include <omp.h>
#include <chrono>
#include <algorithm>
#include <list>
#include <bitset>

#define LOG2(X) ((unsigned) (8*sizeof (unsigned long long) - __builtin_clzll((X)) - 1))
#define ll long long
#define ull unsigned long long

using namespace std;

ll N;
ll all_queens_placed;
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
    cout<<"found solution"<<endl;
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

void try_queen(vector<int> &queens, ll rowmask, ll ldmask, ll rdmask, int col, ull &solutions_count)
{
    if ( rowmask == all_queens_placed )
    {
        ++solutions_count;
        #pragma critical
        {
            solutions.push_back(queens);
        }
        return;
    }

    ll safe = all_queens_placed & (~(rowmask | ldmask | rdmask));
    while (safe)
    {
        ll p = safe & (-safe);
        queens[col] = LOG2(p);
        try_queen(queens, rowmask | p, (ldmask | p) << 1, (rdmask | p) >> 1, col + 1, solutions_count);
        safe = safe & (safe - 1);
    }
}
size_t find_all_solutions()
{
    size_t num_solutions = 0;
    int col = 0;

    #pragma omp parallel
    #pragma omp single
    {
        ll safe = all_queens_placed;
        while (safe)
        {
            ll p = safe & (-safe);
            #pragma omp task
            {
                vector<int> priv_queens(N);
                ll rowmask, ldmask, rdmask;
                ull priv_num_solutions = 0;
                priv_queens[0] = LOG2(p);
                rowmask = ldmask = rdmask = 0;
                try_queen(priv_queens, rowmask | p, (ldmask | p) << 1, (rdmask | p) >> 1, col + 1, priv_num_solutions);

                #pragma omp atomic
                num_solutions += priv_num_solutions;
            }
            safe = safe & (safe - 1);
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

void try_queen_one_solution(vector<int> &queens, ll rowmask, ll ldmask, ll rdmask, int col)
{
    if (found)
        return;
    if ( rowmask == all_queens_placed && !found )
    {
        found = true;
        generate_dot(queens);
        return;
    }

    ll safe = all_queens_placed & (~(rowmask | ldmask | rdmask));
    while (safe)
    {
        ll p = safe & (-safe);
        queens[col] = LOG2(p);
        try_queen_one_solution(queens, rowmask | p, (ldmask | p) << 1, (rdmask | p) >> 1, col + 1);
        safe = safe & (safe - 1);
    }
}

void find_a_solution()
{
    int col = 0;

    #pragma omp parallel
    #pragma omp single
    {
        ll safe = all_queens_placed;
        while (safe)
        {
            ll p = safe & (-safe);
            #pragma omp task
            {
                vector<int> priv_queens(N);
                ll rowmask, ldmask, rdmask;
                ull priv_num_solutions = 0;
                priv_queens[0] = LOG2(p);
                rowmask = ldmask = rdmask = 0;
                try_queen_one_solution(priv_queens, rowmask | p, (ldmask | p) << 1, (rdmask | p) >> 1, col + 1);
            }
            safe = safe & (safe - 1);
        }
    }
}

int main(int argc, char *argv[])
{
    string problemType = get_cmd_option(argv, argc + argv, "-problemType");
    ll n = stoi(get_cmd_option(argv, argc + argv, "-N"));
    /* No error checking */

    N = n;
    all_queens_placed = (1 << N) - 1;
    /* omp_set_num_threads(4); */
    /* auto timer_start = chrono::high_resolution_clock::now(); */

    if (problemType == "all")
    {
        size_t solutions_count = find_all_solutions();
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
