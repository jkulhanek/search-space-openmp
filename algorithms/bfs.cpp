#include "bfs.h"
#include <omp.h>
#include <unordered_set>
#include <atomic>
#include <iostream>
#include <mutex>

using namespace std;

// Naimplementujte efektivni algoritmus pro nalezeni nejkratsi cesty v grafu.
// V teto metode nemusite prilis optimalizovat pametove naroky, a vhodnym algo-
// ritmem tak muze byt napriklad pouziti prohledavani do sirky (breadth-first
// search.
//
// Metoda ma za ukol vratit ukazatel na cilovy stav, ktery je dosazitelny pomoci
// nejkratsi cesty.
std::shared_ptr<const state> bfs(std::shared_ptr<const state> root) {  
    if(root->is_goal())
        return root;

    int thread_len = omp_get_max_threads();
    vector<shared_ptr<const state>> layer(0);
    vector<shared_ptr<const state>> next_layer(0);
    unordered_set<unsigned long long> closed_list(0);
    mutex lock;
    vector<int> histogram(thread_len);

    layer.push_back(root);
    closed_list.insert(root->get_identifier());

    vector<vector<shared_ptr<const state>>> local_layers(thread_len);
    shared_ptr<const state> result;

    int i = 0;
    while(true){
        #pragma omp parallel shared(layer, next_layer, closed_list)
        {
            //vector<shared_ptr<const state>> &local_layer = local_layers[omp_get_thread_num()];

            #pragma omp for schedule(dynamic)
            for(int i = 0; i < layer.size();++i){
                #pragma omp cancellation point for
                // is is final
                shared_ptr<const state> & node = layer[i];

              /*  bool is_closed = false;
                #pragma omp critical
                {
                    //is_closed = closed_list.count(node->get_identifier()) != 0;
                    auto res = closed_list.insert(node->get_identifier());
                    is_closed = !res.second;
                }

                if(is_closed){
                    continue;
                }   */      
                
              /*
                bool is_closed = false;
                #pragma omp critical
                {
                    is_closed = closed_list.count(node->get_identifier()) != 0;
                    if(!is_closed) closed_list.emplace(node->get_identifier());
                }

                if(is_closed){
                    continue;
                }
*/
                
                vector<shared_ptr<const state>> children = node->next_states();
                for(auto & child : children){
                    if(child->is_goal()) {
                        shared_ptr<const state> temp = nullptr;
                        atomic_compare_exchange_weak(&result, &temp, child);
                        #pragma omp cancel for
                    }
                    else{
                        #pragma omp critical
                        {
                            unsigned long long id = child->get_identifier();
                            if(closed_list.find(id) == closed_list.end()){
                                auto res = closed_list.insert(id);
                                next_layer.push_back(child);
                            }                        
                        } 
                    }                   
                }
            }    
        }

        if(result != nullptr) {
            return result;
        }       

        // cout << "iter " << i++ << " with size: " << next_layer.size() <<"\n";
      /*  layer.clear();
        for(unsigned int i = 0; i < thread_len; ++i){
            for(unsigned int j = 0; j < local_layers[i].size();++j){
                shared_ptr<const state> & node = local_layers[i][j];
                if(closed_list.count(node->get_identifier()) == 0){
                    layer.push_back(node);
                }
            }

            local_layers[i].clear();
        }
*/
        //join layers

        layer.swap(next_layer);
        next_layer.clear();
        /*histogram[0] = 0;
        for(int j = 1; j < thread_len; ++j){
            histogram[j] = histogram[j - 1] + local_layers[j - 1].size();        
        }

        layer.resize(histogram[thread_len - 1] + local_layers[thread_len - 1].size());

        #pragma omp parallel for
        for(int j = 0; j < thread_len; ++j){
            for(int i = 0;i<local_layers[j].size();++i){
                layer[histogram[j] + i] = local_layers[j][i];
            }
            local_layers[j].clear();
        }*/
    }

    return root;
}