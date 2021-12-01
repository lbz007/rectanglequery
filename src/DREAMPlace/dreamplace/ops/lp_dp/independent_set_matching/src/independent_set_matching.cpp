/**
 * @file   independent_set_matching.cpp
 * @author Chenzhong, Fly
 * @date   Nov 2020
 */
#include <limits>
#include <chrono>
#include <omp.h>

#include "utility/src/Msg.h"
#include "lp_dp/independent_set_matching/src/independent_set_matching.h"
#include "independent_set_matching/src/hungarian_cpu.h"
#include "independent_set_matching/src/min_cost_flow_cpu.h"
#include "independent_set_matching/src/auction_cpu.h"

#define LAP_SOLVER AuctionAlgorithmCPULauncher

#include "utility/src/DetailedPlaceDB.h"
#include "utility/src/diamond_search.h"
#include "independent_set_matching/src/bin2node_3d_map.h"
#include "independent_set_matching/src/bin2node_map.h"
#include "independent_set_matching/src/construct_spaces.h"
#include "independent_set_matching/src/maximal_independent_set.h"
#include "independent_set_matching/src/collect_independent_sets.h"
#include "independent_set_matching/src/cost_matrix_construction.h"
#include "independent_set_matching/src/apply_solution.h"

DREAMPLACE_BEGIN_NAMESPACE

template <typename T>
struct IndependentSetMatchingState
{
    std::vector<int> ordered_nodes; 
    std::vector<std::vector<int> > independent_sets; 
    std::vector<unsigned char> dependent_markers; 
    std::vector<unsigned char> selected_markers; 
    std::vector<int> num_selected_markers; 
    std::vector<GridIndex<int> > search_grids; 

    std::vector<std::vector<int> > bin2node_map; ///< the first dimension is size, all the cells are categorized by width 
    std::vector<BinMapIndex> node2bin_map;  
    std::vector<Space<T> > spaces; ///< not used yet 

    std::vector<std::vector<int> > cost_matrices; ///< the convergence rate is related to numerical scale 
    std::vector<std::vector<int> > solutions; 
    std::vector<int> orig_costs; ///< original cost before matching 
    std::vector<int> target_costs; ///< target cost after matching
    std::vector<std::vector<T> > target_pos_x; ///< temporary storage of cell locations 
    std::vector<std::vector<T> > target_pos_y; 
    std::vector<std::vector<Space<T> > > target_spaces; ///< not used yet 

    int batch_size; 
    int set_size; 
    int grid_size; 
    int max_diamond_search_sequence; 
    int num_moved; 
    T large_number; 
    T skip_threshold; ///< ignore connections if cells are far apart 
    int num_threads; 
};

template <typename T>
void independentSetMatchingCPU(DetailedPlaceDB<T> db, 
     int batch_size, int set_size, int max_iters, int num_threads)
{
    // fix random seed 
    std::srand(1000);
    //const double threshold = 0.00001/100; 
    IndependentSetMatchingState<T> state; 
    state.batch_size = batch_size; 
    state.set_size = set_size; 
    state.num_moved = 0; 
    state.large_number = (db.xh-db.xl + db.yh-db.yl)*10; 
    state.skip_threshold = (db.xh-db.xl+db.yh-db.yl)*0.01;
    state.num_threads = std::max(num_threads, 1);

    state.bin2node_map.resize(db.num_bins_x*db.num_bins_y);
    state.node2bin_map.resize(db.num_movable_nodes);
    //make_bin2node_map(db, db.x, db.y, db.node_size_x, db.node_size_y, state);
    construct_spaces(db, db.x, db.y, state.spaces, state.num_threads);
    state.grid_size = ceil_power2(std::max(db.num_bins_x, db.num_bins_y)/8);
    state.max_diamond_search_sequence = state.grid_size*state.grid_size/2; 
    dreamplacePrint(kINFO, "diamond search grid size %d, sequence length %d\n", state.grid_size, state.max_diamond_search_sequence);

    state.ordered_nodes.resize(db.num_movable_nodes); 
    std::iota(state.ordered_nodes.begin(), state.ordered_nodes.end(), 0);
    state.independent_sets.resize(state.batch_size, std::vector<int>(state.set_size)); 
    state.dependent_markers.assign(db.num_nodes, 0); 
    state.selected_markers.assign(db.num_movable_nodes, 0); 
    state.num_selected_markers.assign(db.num_movable_nodes, 0);
    state.search_grids = diamond_search_sequence(state.grid_size, state.grid_size); 

    state.cost_matrices.resize(state.batch_size);
    state.solutions.resize(state.batch_size);
    state.orig_costs.resize(state.batch_size); 
    state.target_costs.resize(state.batch_size); 
    state.target_pos_x.resize(state.batch_size); 
    state.target_pos_y.resize(state.batch_size); 
    state.target_spaces.resize(state.batch_size);
    std::vector<LAP_SOLVER<int> > solvers(state.num_threads); 

    bool major = false; // row major 

    // runtime profiling 
    hr_clock_rep iter_timer_start, iter_timer_stop; 
    hr_clock_rep timer_start, timer_stop; 
    int random_shuffle_runs = 0, maximal_independent_set_runs=0, collect_independent_sets_runs = 0, cost_matrix_construction_runs = 0, hungarian_runs = 0, apply_solution_runs = 0; 
    hr_clock_rep random_shuffle_time = 0, maximal_independent_set_time = 0, collect_independent_sets_time = 0, cost_matrix_construction_time = 0, hungarian_time = 0, apply_solution_time = 0; 

    std::vector<int64_t> hpwls (max_iters+1); 
    hpwls.at(0) = db.compute_total_hpwl_edi();
    dreamplacePrint(kINFO, "initial hpwl %g\n", static_cast<double>(hpwls.at(0)));
    for (int iter = 0; iter < max_iters; ++iter)
    {
        iter_timer_start = get_globaltime();

        timer_start = get_globaltime();
        //std::random_shuffle(state.ordered_nodes.begin(), state.ordered_nodes.end()); 
        std::sort(state.ordered_nodes.begin(), state.ordered_nodes.end(), 
                [&](int node_id1, int node_id2){return state.num_selected_markers.at(node_id1) < state.num_selected_markers.at(node_id2);});
        timer_stop = get_globaltime();
        random_shuffle_time += timer_stop-timer_start; 
        random_shuffle_runs += 1; 
        std::fill(state.selected_markers.begin(), state.selected_markers.end(), 0);

        timer_start = get_globaltime();
        // for small benchmarks, sequential version is faster 
        // as the parallel algorithm needs to run at most 10 times, 
        // there will be no benefit with 10 or fewer threads 
        if (state.num_threads < 10)
        {
            maximal_independent_set_sequential(db, state);
        }
        else 
        {
            maximal_independent_set_parallel(db, state);
        }
        timer_stop = get_globaltime();
        maximal_independent_set_time += timer_stop-timer_start; 
        maximal_independent_set_runs += 1; 

        timer_start = get_globaltime(); 
        int num_independent_sets = collect_independent_sets(db, state);
#pragma omp parallel for num_threads(state.num_threads) 
        for (int i = 0; i < num_independent_sets; ++i)
        {
            for (auto node_id : state.independent_sets.at(i))
            {
                if (node_id < db.num_movable_nodes)
                {
                    state.num_selected_markers.at(node_id) += 1; 
                }
            }
        }
        timer_stop = get_globaltime(); 
        collect_independent_sets_time += timer_stop-timer_start; 
        collect_independent_sets_runs += 1; 

        if (num_independent_sets > state.batch_size)
        {
            state.cost_matrices.resize(num_independent_sets); 
            state.solutions.resize(num_independent_sets);
            state.orig_costs.resize(num_independent_sets); 
            state.target_costs.resize(num_independent_sets); 
            state.target_pos_x.resize(num_independent_sets); 
            state.target_pos_y.resize(num_independent_sets); 
            state.target_spaces.resize(num_independent_sets);
        }

        timer_start = get_globaltime();
#pragma omp parallel for num_threads(state.num_threads) 
        for (int i = 0; i < num_independent_sets; ++i)
        {
            auto const& independent_set = state.independent_sets.at(i);
            auto& cost_matrix = state.cost_matrices.at(i);
            cost_matrix.resize(independent_set.size()*independent_set.size());

            cost_matrix_construction(db, state, major, i);
        }
        timer_stop = get_globaltime();
        cost_matrix_construction_time += timer_stop-timer_start; 
        cost_matrix_construction_runs += 1; 

        timer_start = get_globaltime();
#pragma omp parallel for num_threads(state.num_threads) 
        for (int i = 0; i < num_independent_sets; ++i)
        {
            auto const& independent_set = state.independent_sets.at(i);
            auto& cost_matrix = state.cost_matrices.at(i);
            auto& solution = state.solutions.at(i);
            auto& orig_cost = state.orig_costs.at(i);
            auto& target_cost = state.target_costs.at(i);
            solution.resize(independent_set.size());

            // solve bipartite assignment problem 
            // compute initial cost 
            orig_cost = 0; 
            for (unsigned int j = 0; j < independent_set.size(); ++j)
            {
                orig_cost += cost_matrix.at(j*independent_set.size()+j);
            }
            int tid = omp_get_thread_num();
            target_cost = solvers.at(tid).run(cost_matrix.data(), solution.data(), independent_set.size());
        }
        timer_stop = get_globaltime();
        hungarian_time += timer_stop-timer_start; 
        hungarian_runs += 1; 

        timer_start = get_globaltime();
#pragma omp parallel for num_threads(state.num_threads) 
        for (int i = 0; i < num_independent_sets; ++i)
        {
            apply_solution(db, state, i);
        }
        timer_stop = get_globaltime();
        apply_solution_time += timer_stop-timer_start; 
        apply_solution_runs += 1; 

        iter_timer_stop = get_globaltime(); 
        hpwls.at(iter+1) = db.compute_total_hpwl_edi(); 
        if ((iter%(std::max(max_iters/10, 1))) == 0 || iter+1 == max_iters)
        {
            state.num_moved = 0; 
            for (int i = 0; i < db.num_movable_nodes; ++i)
            {
                if (db.x[i] != db.init_x[i] || db.y[i] != db.init_y[i])
                {
                    state.num_moved += 1; 
                }
            }
            dreamplacePrint(kINFO, "iteration %d, target hpwl %g, delta %g(%g%%), solved %d sets, moved %g%% cells, runtime %g ms\n", 
                    iter, 
                    static_cast<double>(hpwls.at(iter+1)), static_cast<double>(hpwls.at(iter+1)-hpwls.at(0)), static_cast<double>(hpwls.at(iter+1)-hpwls.at(0))/hpwls.at(0)*100, 
                    num_independent_sets, 
                    state.num_moved/(double)db.num_movable_nodes*100, 
                    get_timer_period()*(iter_timer_stop-iter_timer_start)
                    );
        }

        //if (iter && hpwls.at(iter)-hpwls.at(iter+1) < threshold*hpwls.at(iter))
        //{
        //    break; 
        //}
    }
    dreamplacePrint(kDEBUG, "random_shuffle takes %g ms, %d runs, average %g ms\n", 
            get_timer_period()*random_shuffle_time, random_shuffle_runs, get_timer_period()*random_shuffle_time/random_shuffle_runs);
    dreamplacePrint(kDEBUG, "maximal_independent_set takes %g ms, %d runs, average %g ms\n", 
            get_timer_period()*maximal_independent_set_time, maximal_independent_set_runs, get_timer_period()*maximal_independent_set_time/maximal_independent_set_runs);
    dreamplacePrint(kDEBUG, "collect_independent_sets takes %g ms, %d runs, average %g ms\n", 
            get_timer_period()*collect_independent_sets_time, collect_independent_sets_runs, get_timer_period()*collect_independent_sets_time/collect_independent_sets_runs);
    dreamplacePrint(kDEBUG, "cost_matrix_construction takes %g ms, %d runs, average %g ms\n", 
            get_timer_period()*cost_matrix_construction_time, cost_matrix_construction_runs, get_timer_period()*cost_matrix_construction_time/cost_matrix_construction_runs);
    dreamplacePrint(kDEBUG, "%s takes %g ms, %d runs, average %g ms\n", 
            solvers.front().name(), 
            get_timer_period()*hungarian_time, hungarian_runs, get_timer_period()*hungarian_time/hungarian_runs);
    dreamplacePrint(kDEBUG, "apply solution takes %g ms, %d runs, average %g ms\n", 
            get_timer_period()*apply_solution_time, apply_solution_runs, get_timer_period()*apply_solution_time/apply_solution_runs);

}

void IndependentSetMatching::run_cpu()
{
    //make DetailedPlaceDB db.
    DetailedPlaceDB<int> db;
    db_->initDetailedPlaceDB(db);
    if (alg_concurrent_) { //concurrent
      independentSetMatchingCPU(db, batch_size_, set_size_, max_iters_, num_threads_);
    } else {
      independentSetMatchingCPU(db, set_size_, max_iters_);
    }
    db_->updateXY(db.x, db.y);
    db_->freeDetailedPlaceDB(db);

    return;
}

void IndependentSetMatching::run_gpu()
{
    //make DetailedPlaceDB db.
    DetailedPlaceDB<int> db;
    db_->initDetailedPlaceDB(db);
#ifdef _CUDA_FOUND
    independentSetMatchingCUDARun(db, batch_size_, set_size_, max_iters_, num_threads_);
    db_->updateXYGPU(db.x, db.y);
#endif
    db_->freeDetailedPlaceDB(db);

    return;
}

void IndependentSetMatching::run()
{
    if (!isCommonDBReady()) {
      return;
    }
    hr_clock_rep total_time_start, total_time_stop;
    total_time_start = get_globaltime(); 
    if (is_gpu_)
    {
        run_gpu();
    }
    else
    {
        run_cpu();
    }
    total_time_stop = get_globaltime();
    dreamplacePrint(kINFO, "Independent set matcing time: %g ms\n", get_timer_period()*(total_time_stop-total_time_start));
    return;
}

DREAMPLACE_END_NAMESPACE
