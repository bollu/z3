/*++
Copyright (c) 2011 Microsoft Corporation

Module Name:

    sat_config.cpp

Abstract:

    SAT configuration options

Author:

    Leonardo de Moura (leonardo) 2011-05-21.

Revision History:

--*/
#include "sat/sat_config.h"
#include "sat/sat_types.h"
#include "sat/sat_params.hpp"
#include "sat/sat_simplifier_params.hpp"


namespace sat {

    config::config(params_ref const & p) {
        m_incremental = false; // ad-hoc parameter
        updt_params(p); 
    }

    void config::updt_params(params_ref const & _p) {
        sat_params p(_p);
        m_max_memory  = megabytes_to_bytes(p.max_memory());

        symbol s = p.restart();
        if (s == symbol("luby"))
            m_restart = RS_LUBY;
        else if (s == symbol("geometric"))
            m_restart = RS_GEOMETRIC;
        else
            throw sat_param_exception("invalid restart strategy");

        s = p.phase();
        if (s == symbol("always_false")) 
            m_phase = PS_ALWAYS_FALSE;
        else if (s == symbol("always_true"))
            m_phase = PS_ALWAYS_TRUE;
        else if (s == symbol("caching"))
            m_phase = PS_CACHING;
        else if (s == symbol("random"))
            m_phase = PS_RANDOM;
        else
            throw sat_param_exception("invalid phase selection strategy");

        m_phase_caching_on  = p.phase_caching_on();
        m_phase_caching_off = p.phase_caching_off();

        m_restart_initial = p.restart_initial();
        m_restart_factor  = p.restart_factor();
        m_restart_max     = p.restart_max();
        m_inprocess_max   = p.inprocess_max();

        m_random_freq     = p.random_freq();
        m_random_seed     = p.random_seed();
        if (m_random_seed == 0) 
            m_random_seed = _p.get_uint("random_seed", 0);
        
        m_burst_search    = p.burst_search();
        
        m_max_conflicts   = p.max_conflicts();
        m_num_threads     = p.threads();
        m_local_search    = p.local_search();
        m_local_search_threads = p.local_search_threads();
        m_lookahead_simplify = p.lookahead_simplify();
        m_lookahead_simplify_bca = p.lookahead_simplify_bca();
        if (p.lookahead_reward() == symbol("heule_schur")) 
            m_lookahead_reward = heule_schur_reward;
        else if (p.lookahead_reward() == symbol("heuleu")) 
            m_lookahead_reward = heule_unit_reward;
        else if (p.lookahead_reward() == symbol("ternary")) 
            m_lookahead_reward = ternary_reward;
        else if (p.lookahead_reward() == symbol("unit")) 
            m_lookahead_reward = unit_literal_reward;
        else if (p.lookahead_reward() == symbol("march_cu")) 
            m_lookahead_reward = march_cu_reward;
        else  
            throw sat_param_exception("invalid reward type supplied: accepted heuristics are 'ternary', 'heuleu', 'unit' or 'heule_schur'");

        m_lookahead_cube_fraction = p.lookahead_cube_fraction();
        m_lookahead_cube_cutoff = p.lookahead_cube_cutoff();
        m_lookahead_global_autarky = p.lookahead_global_autarky();

        // These parameters are not exposed
        m_simplify_mult1  = _p.get_uint("simplify_mult1", 300);
        m_simplify_mult2  = _p.get_double("simplify_mult2", 1.5);
        m_simplify_max    = _p.get_uint("simplify_max", 500000);
        // --------------------------------

        s = p.gc();
        if (s == symbol("dyn_psm")) 
            m_gc_strategy = GC_DYN_PSM;
        else if (s == symbol("glue_psm"))
            m_gc_strategy = GC_GLUE_PSM;
        else if (s == symbol("glue"))
            m_gc_strategy = GC_GLUE;
        else if (s == symbol("psm"))
            m_gc_strategy = GC_PSM;
        else if (s == symbol("psm_glue"))
            m_gc_strategy = GC_PSM_GLUE;
        else 
            throw sat_param_exception("invalid gc strategy");
        m_gc_initial      = p.gc_initial();
        m_gc_increment    = p.gc_increment();
        m_gc_small_lbd    = p.gc_small_lbd();
        m_gc_k            = std::min(255u, p.gc_k());

        m_minimize_lemmas = p.minimize_lemmas();
        m_core_minimize   = p.core_minimize();
        m_core_minimize_partial   = p.core_minimize_partial();
        m_drat_check_unsat  = p.drat_check_unsat();
        m_drat_check_sat  = p.drat_check_sat();
        m_drat_file       = p.drat_file();
        m_drat            = (m_drat_check_unsat || m_drat_file != symbol("") || m_drat_check_sat) && p.threads() == 1;
        m_dyn_sub_res     = p.dyn_sub_res();

        // Parameters used in Liang, Ganesh, Poupart, Czarnecki AAAI 2016.
        m_branching_heuristic = BH_VSIDS;
        if (p.branching_heuristic() == symbol("vsids")) 
            m_branching_heuristic = BH_VSIDS;
        else if (p.branching_heuristic() == symbol("chb")) 
            m_branching_heuristic = BH_CHB;
        else if (p.branching_heuristic() == symbol("lrb")) 
            m_branching_heuristic = BH_LRB;
        else 
            throw sat_param_exception("invalid branching heuristic: accepted heuristics are 'vsids', 'lrb' or 'chb'");

        m_anti_exploration = p.branching_anti_exploration();
        m_step_size_init = 0.40;
        m_step_size_dec  = 0.000001;
        m_step_size_min  = 0.06;
        m_reward_multiplier = 0.9;
        m_reward_offset = 1000000.0;

        m_variable_decay = p.variable_decay();

        // PB parameters
        s = p.pb_solver();
        if (s == symbol("circuit")) 
            m_pb_solver = PB_CIRCUIT;
        else if (s == symbol("sorting")) 
            m_pb_solver = PB_SORTING;
        else if (s == symbol("totalizer")) 
            m_pb_solver = PB_TOTALIZER;
        else if (s == symbol("solver")) 
            m_pb_solver = PB_SOLVER;
        else 
            throw sat_param_exception("invalid PB solver: solver, totalizer, circuit, sorting");

        sat_simplifier_params sp(_p);
        m_elim_vars = sp.elim_vars();
    }

    void config::collect_param_descrs(param_descrs & r) {
        sat_params::collect_param_descrs(r);
    }

};
