#include "iddfs.h"
#include <omp.h>
#include <unordered_set>
#include <atomic>
#include <iostream>

using namespace std;
void iddfs_run_rec(std::shared_ptr<const state> node, unsigned int treshold, std::shared_ptr<const state> & result, atomic<unsigned int> & next_treshold);

// Naimplementujte efektivni algoritmus pro nalezeni nejkratsi (respektive nej-
// levnejsi) cesty v grafu. V teto metode mate ze ukol naimplementovat pametove
// efektivni algoritmus pro prohledavani velkeho stavoveho prostoru. Pocitejte
// s tim, ze Vami navrzeny algoritmus muze bezet na stroji s omezenym mnozstvim
// pameti (radove nizke stovky megabytu). Vhodnym pristupem tak muze byt napr.
// iterative-deepening depth-first search.
//
// Metoda ma za ukol vratit ukazatel na cilovy stav, ktery je dosazitelny pomoci
// nejkratsi/nejlevnejsi cesty.

void iddfs_run_rec(std::shared_ptr<const state> node, unsigned int treshold, std::shared_ptr<const state> & result, atomic<unsigned int> & next_treshold){
    if(result != nullptr){
        return; // cancel mechanism
    }

    const unsigned int current_cost = node->current_cost(); 

    if(node->is_goal() && current_cost == treshold){
        shared_ptr<const state> null_state;
        atomic_compare_exchange_weak(&result, &null_state, node);
        return;
        // #pragma omp cancel parallel
    } else if(current_cost > treshold){
        // try to push the treshold in case of not uniform case
        // will be done in the next version
        unsigned int temp = next_treshold;
        while((temp == treshold || current_cost < temp) && !next_treshold.compare_exchange_weak(temp, current_cost)) ;
        return;
    }

    const unsigned int expected_treshold = treshold - current_cost;
    vector<shared_ptr<const state>> children = node->next_states();
    unordered_set<unsigned long> closed_list (0);
    shared_ptr<const state> & tmp = node;
    while(tmp != nullptr){
        closed_list.emplace(tmp->get_identifier());
        tmp = tmp->get_predecessor();
    } 

    for(int i = 0; i < children.size(); ++i){
        auto & child = children[i];
        if(closed_list.count(child->get_identifier()) > 0){
            continue;
        }
        
        #pragma omp task shared(result, next_treshold) if(expected_treshold > 2)
        iddfs_run_rec(child, treshold, result, next_treshold);
    }
}


std::shared_ptr<const state> iddfs(std::shared_ptr<const state> root) {
    std::shared_ptr<const state> result = nullptr;
    atomic<unsigned int> treshold { 1 };

    int i = 0;
    while(result == nullptr){
        #pragma omp parallel
        {
            #pragma omp single
            {
                iddfs_run_rec(root, treshold, result, treshold);                
            }

            #pragma omp barrier
        }  

        // cout << "iteration done " << i++ << " with treshold:" << treshold <<"\n";
    }

    return result;
}