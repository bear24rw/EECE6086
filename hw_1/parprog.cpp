#include <iostream>
#include <unordered_set>
#include <string.h>
#include <climits>
#include <ctime>

//#define DEBUG 1
#define GROUP_A 0
#define GROUP_B 1

int main() {

    #ifdef GENERIC
    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    fprintf(stderr, "This binary was compiled for a generic CPU. It is highly\n");
    fprintf(stderr, "recommended you re-run 'make' in order to build with\n");
    fprintf(stderr, "optimizations specific to your CPU.\n");
    fprintf(stderr, "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    #endif

    clock_t time_start = std::clock();

    //
    // Read in the connection pairs
    //

    int num_verts;
    int num_edges;

    std::cin >> num_verts;
    std::cin >> num_edges;

    fprintf(stderr, "Num verts: %d\n", num_verts);
    fprintf(stderr, "Num edges: %d\n", num_edges);

    std::unordered_set<int> *connections = new std::unordered_set<int>[num_verts + 1];

    for (int n1, n2; std::cin >> n1 >> n2;) {
        connections[n1].insert(n2);
        connections[n2].insert(n1);
    }

    //
    // Split the nodes into two even groups
    //

    int *group = new int[num_verts + 1];
    int *tmp_group = new int[num_verts + 1];

    for (int n = 1; n <= num_verts; n++) {
        group[n] = (n + 1) % 2;  // 0 = GROUP_A 1 = GROUP_B
    }

    // keep a list of which nodes we have swapped at each iteration
    int *swap_a = new int[num_verts / 2];
    int *swap_b = new int[num_verts / 2];
    int *swap_g = new int[num_verts / 2];

    // flag all nodes that have swapped
    int *marked = new int[num_verts + 1];

    // cost of each node
    int *D = new int[num_verts + 1];

    int iteration = 0;

    while (1) {

        fprintf(stderr, "========== Iteration %d ==========\n", ++iteration);

        // make a temporary copy of our group vector that we can modify
        memcpy(tmp_group, group, (num_verts + 1) * sizeof(int));

        // reset the cost list
        memset(D, 0, (num_verts + 1) * sizeof(int));

        // reset the marked node list
        memset(marked, 0, (num_verts + 1) * sizeof(int));

        //
        // Compute the intial cost of all nodes
        //
        for (int n1 = 1; n1 <= num_verts; n1++) {
            for (auto &n2 : connections[n1]) {
                // if nodes are in the same group their cost is decreased
                if (tmp_group[n1] == tmp_group[n2])
                    D[n1] -= 1;
                else
                    D[n1] += 1;
            }
        }

        int cost_sum = 0;
        int cost_max = INT_MIN;
        int k_max = 0;

        for (int k = 0; k < (num_verts / 2); k++) {

            // reset the most costly nodes and the total cost
            int a_name = -1;
            int b_name = -1;
            int a_cost = INT_MIN;
            int b_cost = INT_MIN;
            int cost = INT_MIN;

            // loop through all possible nodes and
            // find the maximum from each group
            for (int n1 = 1; n1 <= num_verts; n1++) {

                // if this node is marked we already processed it so skip it
                if (marked[n1]) continue;

                //
                // See if it is the new maximum for its group
                //

                if (tmp_group[n1] == GROUP_A) {
                    if (D[n1] > a_cost) {
                        a_name = n1;
                        a_cost = D[n1];
                    }
                } else {
                    if (D[n1] > b_cost) {
                        b_name = n1;
                        b_cost = D[n1];
                    }
                }
            }

            //
            // Calculate the total cost of this swap
            //

            if (connections[a_name].find(b_name) != connections[a_name].end())
                cost = a_cost + b_cost - 2;
            else
                cost = a_cost + b_cost;

            #ifdef DEBUG
            fprintf(stderr, "[%d][%d/%d] Cost (%d, %d) = %d\n", iteration, k, (num_verts / 2) - 1, a_name, b_name, cost);
            #endif

            //
            // Check to see if this is a new maximum subset length
            //

            cost_sum += cost;
            if (cost_sum > cost_max) {
                cost_max = cost_sum;
                k_max = k;
            }

            //
            // Record what nodes we swapped
            //

            swap_a[k] = a_name;
            swap_b[k] = b_name;
            swap_g[k] = cost;

            //
            // Swap the nodes in the temorary groups
            //

            tmp_group[a_name] = tmp_group[a_name] ? GROUP_A : GROUP_B;
            tmp_group[b_name] = tmp_group[b_name] ? GROUP_A : GROUP_B;

            //
            // Mark the nodes are swapped
            //

            marked[a_name] = 1;
            marked[b_name] = 1;

            // if cheat mode is enabled we want to
            // exit as soon as we see a negative cost
            #ifdef CHEAT
            if (cost <= 0) break;
            #endif

            //
            // Update the costs of all the effected nodes
            //

            for (auto &n2 : connections[a_name]) {
                // if the nodes are now in the same group decrease the score
                if (tmp_group[a_name] == tmp_group[n2])
                    D[n2] -= 2;
                else
                    D[n2] += 2;
            }

            for (auto &n2 : connections[b_name]) {
                // if the nodes are now in the same group decrease the score
                if (tmp_group[b_name] == tmp_group[n2])
                    D[n2] -= 2;
                else
                    D[n2] += 2;
            }
        }

        fprintf(stderr, "k_max: %d cost_max: %d\n", k_max, cost_max);

        //
        // If there were no swaps that resulted in cost decrease we are done
        //

        if (cost_max <= 0) {
            fprintf(stderr, "Done\n");
            break;
        }

        //
        // Exchange up to k_max nodes
        //

        for (int k = 0; k <= k_max; k++) {
            group[swap_a[k]] = group[swap_a[k]] ? GROUP_A : GROUP_B;
            group[swap_b[k]] = group[swap_b[k]] ? GROUP_A : GROUP_B;
        }

        fprintf(stderr, "Elapsed time: %f seconds\n", double(std::clock() - time_start) / CLOCKS_PER_SEC);
    }

    //
    // Calculate final cutsize
    //

    int cut_size = 0;
    for (int n1 = 1; n1 <= num_verts; n1++) {
        // only process one side
        if (group[n1] == GROUP_B) continue;

        // loop through all connections to this node
        for (auto &n2 : connections[n1]) {
            // if this other node is in group B then it is cut
            if (group[n2] == GROUP_B) cut_size++;
        }
    }

    printf("%d\n", cut_size);

    //
    // Print out the two groups
    //

    for (int n = 1; n <= num_verts; n++) {
        if (group[n] == GROUP_A) printf("%d ", n);
    }
    printf("\n");

    for (int n = 1; n <= num_verts; n++) {
        if (group[n] == GROUP_B) printf("%d ", n);
    }
    printf("\n");

    //
    // Print the total CPU time it took
    //

    printf("%f\n", double(std::clock() - time_start) / CLOCKS_PER_SEC);

    delete[] group;
    delete[] tmp_group;
    delete[] D;
    delete[] swap_a;
    delete[] swap_b;
    delete[] swap_g;
    delete[] marked;
    delete[] connections;

    return 0;
}
