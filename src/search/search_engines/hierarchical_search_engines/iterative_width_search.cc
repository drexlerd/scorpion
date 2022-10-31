#include "iterative_width_search.h"

#include "goal_test.h"

#include "../../option_parser.h"
#include "../../plugin.h"
#include "../../tasks/root_task.h"
#include "../../tasks/propositional_task.h"
#include "../../task_utils/successor_generator.h"
#include "../../task_utils/task_properties.h"
#include "../../utils/logging.h"
#include "../../utils/memory.h"

#include <cassert>
#include <cstdlib>

using namespace std;
using namespace hierarchical_search_engine;

namespace iw_search {
IWSearch::IWSearch(const Options &opts)
    : HierarchicalSearchEngine(opts),
      width(opts.get<int>("width")),
      debug(opts.get<utils::Verbosity>("verbosity") == utils::Verbosity::DEBUG),
      m_initial_state_id(-1),
      m_novelty_table(0) {
    switch (opts.get<GoalTestEnum>("goal_test")) {
        case GoalTestEnum::TOP_GOAL: {
            goal_test = utils::make_unique_ptr<goal_test::TopGoal>(opts);
            break;
        }
        case GoalTestEnum::SKETCH_SUBGOAL: {
            goal_test = utils::make_unique_ptr<goal_test::SketchSubgoal>(opts);
            break;
        }
        case GoalTestEnum::INCREMENT_GOAL_COUNT: {
            goal_test = utils::make_unique_ptr<goal_test::IncrementGoalCount>(opts);
            break;
        }
    }
    utils::g_log << "Setting up iterative width search." << endl;
}

void IWSearch::initialize() {
    utils::g_log << "Starting iterative width search." << endl;
    if (!propositional_task) {
        propositional_task = std::make_shared<extra_tasks::PropositionalTask>(tasks::g_root_task);
    }
    m_novelty_base = std::make_shared<dlplan::novelty::NoveltyBase>(propositional_task->get_num_facts(), std::max(1, width));
    m_novelty_table = dlplan::novelty::NoveltyTable(m_novelty_base->get_num_tuples());
    std::cout << "Num facts:" << propositional_task->get_num_facts() << std::endl;
    std::cout << "Num entries in novelty table:" << m_novelty_base->get_num_tuples() << std::endl;
    State initial_state = state_registry.get_initial_state();
    m_initial_state_id = initial_state.get_id();
    statistics.inc_generated();
    SearchNode node = search_space.get_node(initial_state);
    node.open_initial();
    open_list.push_back(initial_state.get_id());
    bool novel = is_novel(initial_state);
    utils::unused_variable(novel);
    assert(novel);
}

bool IWSearch::is_novel(const State &state) {
    return m_novelty_table.insert(dlplan::novelty::TupleIndexGenerator(m_novelty_base, propositional_task->get_fact_ids(state)), true);
}

bool IWSearch::is_novel(const OperatorProxy &op, const State &succ_state) {
    return m_novelty_table.insert(dlplan::novelty::TupleIndexGenerator(m_novelty_base, propositional_task->get_fact_ids(op, succ_state)), true);
}

void IWSearch::print_statistics() const {
    statistics.print_detailed_statistics();
    search_space.print_statistics();
}

SearchStatus IWSearch::step() {
    if (open_list.empty()) {
        utils::g_log << "Completely explored state space -- no solution!" << endl;
        return FAILED;
    }
    StateID id = open_list.front();
    open_list.pop_front();
    State state = state_registry.lookup_state(id);
    SearchNode node = search_space.get_node(state);
    node.close();
    assert(!node.is_dead_end());
    statistics.inc_expanded();

    /* Goal check in initial state. */
    if (id == m_initial_state_id) {
        if (check_goal_and_set_plan(state)) {
            return SOLVED;
        }
    }

    vector<OperatorID> applicable_ops;
    successor_generator.generate_applicable_ops(state, applicable_ops);
    for (OperatorID op_id : applicable_ops) {
        OperatorProxy op = task_proxy.get_operators()[op_id];
        if (node.get_real_g() + op.get_cost() >= bound) {
            continue;
        }

        int old_num_states = state_registry.size();
        State succ_state = state_registry.get_successor_state(state, op);
        int new_num_states = state_registry.size();
        bool is_new_state = (new_num_states > old_num_states);
        if (!is_new_state) {
            continue;
        }

        statistics.inc_generated();
        bool novel = is_novel(succ_state);
        if (!novel) {
            continue;
        }

        std::cout << propositional_task->compute_dlplan_state(succ_state).str() << std::endl;

        SearchNode succ_node = search_space.get_node(succ_state);
        assert(succ_node.is_new());
        succ_node.open(node, op, get_adjusted_cost(op));
        open_list.push_back(succ_state.get_id());

        /* Goal check after generating new node to save one g layer.*/
        if (check_goal_and_set_plan(succ_state)) {
            return SOLVED;
        }
    }

    /* Width 0 problems must be solved after at most one expansion step. */
    if (width == 0) {
        return FAILED;
    }

    return IN_PROGRESS;
}

void IWSearch::dump_search_space() const {
    search_space.dump(task_proxy);
}

static shared_ptr<SearchEngine> _parse(OptionParser &parser) {
    parser.document_synopsis("Iterated width search", "");
    parser.add_option<int>(
        "width", "maximum conjunction size", "2");
    SearchEngine::add_options_to_parser(parser);
    add_goal_test_option_to_parser(parser);

    Options opts = parser.parse();
    if (parser.dry_run()) {
        return nullptr;
    }
    return make_shared<IWSearch>(opts);
}

// ./fast-downward.py domain.pddl instance_2_1_0.pddl --translate-options --dump-predicates --dump-constants --dump-static-atoms --dump-goal-atoms --search-options --search "iw(width=2)"
static Plugin<SearchEngine> _plugin("iw", _parse);
}
