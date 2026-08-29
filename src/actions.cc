#include <tiramisu/tiramisu.h>
#include <string>
#include <regex>
#include <unordered_set>
#include <TiraLibCPP/utils.h>
#include <TiraLibCPP/dbhelpers.h>

static void parse_or_throw(const std::string &action_str, std::smatch &match,
                           const std::regex &re)
{
    if (!std::regex_search(action_str, match, re))
    {
        throw std::invalid_argument(
            "Failed to parse schedule token: " + action_str);
    }
}

bool apply_action(std::string action_str, tiramisu::function *implicit_function, Result &result)
{
    if (action_str.empty()) return true;
    bool is_legal = true;
    switch (action_str[0])
    {
    case 'P':
    {
        std::string regex_str = "P\\(L(\\d),comps=\\[([\\w', ]*)\\]\\)";
        std::regex re(regex_str);
        std::smatch match;
        parse_or_throw(action_str, match, re);
        int level = std::stoi(match[1]);
        std::string comps_str = match[2];
        comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
        auto parsed_comps = get_comps(comps_str, implicit_function);
        std::vector<tiramisu::computation *> comps;
        std::unordered_set<tiramisu::computation *> seen;
        for (auto comp : parsed_comps)
        {
            if (seen.insert(comp).second)
                comps.push_back(comp);
        }

        tiramisu::prepare_schedules_for_legality_checks(true);
        is_legal = tiramisu::loop_parallelization_is_legal(level, comps);

        for (auto comp : comps)
            comp->tag_parallel_level(level);
        break;
    }
    case 'U':
    {
        bool check_only = action_str.rfind("UCheck(", 0) == 0;
        std::string regex_str = check_only
            ? "UCheck\\(L(-?\\d+),(\\d+),comps=\\[([\\w', ]*)\\]\\)"
            : "U\\(L(-?\\d+),(\\d+),comps=\\[([\\w', ]*)\\]\\)";
        std::regex re(regex_str);
        std::smatch match;
        parse_or_throw(action_str, match, re);
        int level = std::stoi(match[1]);
        int factor = std::stoi(match[2]);
        std::string comps_str = match[3];
        comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
        auto comps = get_comps(comps_str, implicit_function);

        if (level == -1)
        {
            // Resolve "innermost loop" against the current schedule, which
            // reflects previously applied actions (e.g. tile). Cross-comp
            // mismatch is detected via the iteration domain (untouched by
            // align_schedules' padding) so we catch the gemver-style case
            // where A_hat is 2D and x is 1D even if their schedules have
            // already been padded to a common width by an earlier
            // prepare_schedules_for_legality_checks().
            // Schedule layout is [0, 0, i1, 0, i2, 0, ...]
            // (see loop_level_into_dynamic_dimension in tiramisu_core.cpp);
            // loop count = (out_dims - 2) / 2.
            if (comps.empty())
            {
                throw std::invalid_argument(
                    "U(L-1,...): no computations to unroll");
            }
            auto iter_dim_of = [](tiramisu::computation *c) {
                return isl_set_dim(c->get_iteration_domain(), isl_dim_set);
            };
            int reference_iter_dim = iter_dim_of(comps.front());
            for (auto comp : comps)
            {
                if (iter_dim_of(comp) != reference_iter_dim)
                {
                    throw std::invalid_argument(
                        "U(L-1,...) requires all target computations to "
                        "share the same innermost loop depth. Split "
                        "non-perfectly-nested computations into separate "
                        "U(...) actions.");
                }
            }
            int sched_dims =
                isl_map_dim(comps.front()->get_schedule(), isl_dim_out);
            level = (sched_dims - 2) / 2 - 1;
        }

        tiramisu::prepare_schedules_for_legality_checks(true);

        is_legal = loop_unrolling_is_legal(level, comps);

        if (check_only)
            break;

        for (auto comp : comps)
        {
            comp->unroll(level, factor);
        }

        // Unrolling splits the shared loop of each computation independently,
        // which fissions computations that were fused into one loop each. The
        // LOOPer autoscheduler re-establishes the intended fusion afterwards
        // (via order_computations_from_ast). Replicate that for the unrolled
        // group: re-issue the .after() ordering at their (now deeper) innermost
        // loop level so codegen keeps them fused. Without this, a subset-unroll
        // of mutually dependent computations (e.g. deriche's recursive filter)
        // is distributed and produces wrong results.
        if (comps.size() > 1)
        {
            int sched_dims = isl_map_dim(comps.front()->get_schedule(), isl_dim_out);
            int innermost_level = (sched_dims - 2) / 2 - 1;
            for (size_t i = 1; i < comps.size(); i++)
                comps[i]->after(*comps[i - 1], innermost_level);
        }
        break;
    }
    case 'I':
    {
        std::string regex_str = "I\\(L(\\d),L(\\d),comps=\\[([\\w', ]*)\\]\\)";
        std::regex re(regex_str);
        std::smatch match;
        parse_or_throw(action_str, match, re);
        int level1 = std::stoi(match[1]);
        int level2 = std::stoi(match[2]);
        std::string comps_str = match[3];
        comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
        auto comps = get_comps(comps_str, implicit_function);
        for (auto comp : comps)
        {
            comp->interchange(level1, level2);
        }
        break;
    }
    case 'R':
    {
        std::string regex_str = "R\\(L(\\d),comps=\\[([\\w', ]*)\\]\\)";
        std::regex re(regex_str);
        std::smatch match;
        parse_or_throw(action_str, match, re);
        int level = std::stoi(match[1]);
        std::string comps_str = match[2];
        comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
        auto comps = get_comps(comps_str, implicit_function);
        for (auto comp : comps)
        {
            comp->loop_reversal(level);
        }
        break;
    }
    case 'S':
    {
        // For [i', j']^T = [[alpha, beta], [gamma, sigma]] [i, j]^T,
        // the four-factor form supplies the complete matrix in row-major order.
        // The legacy two-factor form supplies alpha and beta; Tiramisu computes
        // gamma and sigma such that alpha * sigma - beta * gamma = 1.
        std::regex four_factor_re(
            "S\\(L(\\d),L(\\d),(-?\\d+),(-?\\d+),(-?\\d+),(-?\\d+),comps=\\[([\\w', ]*)\\]\\)");
        std::regex two_factor_re(
            "S\\(L(\\d),L(\\d),(-?\\d+),(-?\\d+),comps=\\[([\\w', ]*)\\]\\)");
        std::smatch match;
        bool has_four_factors = std::regex_search(action_str, match, four_factor_re);
        if (!has_four_factors)
        {
            parse_or_throw(action_str, match, two_factor_re);
        }
        int level1 = std::stoi(match[1]);
        int level2 = std::stoi(match[2]);
        int factor1 = std::stoi(match[3]);
        int factor2 = std::stoi(match[4]);
        int factor3 = has_four_factors ? std::stoi(match[5]) : 0;
        int factor4 = has_four_factors ? std::stoi(match[6]) : 0;
        std::string comps_str = match[has_four_factors ? 7 : 5];
        comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
        auto comps = get_comps(comps_str, implicit_function);
        if (!has_four_factors && factor1 == 0 && factor2 == 0)
        {
            auto auto_skewing_result = implicit_function->skewing_local_solver(comps, level1, level2, 1);

            // check outer parallelism factors
            auto outer_factors = std::get<0>(auto_skewing_result);
            if (outer_factors.size() > 0)
            {
                factor1 = outer_factors.at(0).first;
                factor2 = outer_factors.at(0).second;
            }
            else
            {
                // check the inner parallelism factors
                auto inner_factors = std::get<1>(auto_skewing_result);
                if (inner_factors.size() > 0)
                {
                    factor1 = inner_factors.at(0).first;
                    factor2 = inner_factors.at(0).second;
                }
                else
                {
                    // no factors found
                    is_legal = false;
                }
            }
        }
        if (is_legal)
        {
            result.additional_info = "skewing_factors:" + std::to_string(factor1) + "," + std::to_string(factor2);
            if (has_four_factors)
            {
                result.additional_info += "," + std::to_string(factor3) + "," + std::to_string(factor4);
            }
            for (auto comp : comps)
            {
                if (has_four_factors)
                {
                    comp->skew(level1, level2, factor1, factor2, factor3, factor4);
                }
                else
                {
                    comp->skew(level1, level2, factor1, factor2);
                }
            }
        }

        break;
    }
    case 'F':
    {
        std::string regex_str = "F\\(L(\\d),comps=\\[([\\w', ]*)\\]\\)";
        std::regex re(regex_str);
        std::smatch match;
        parse_or_throw(action_str, match, re);
        int level = std::stoi(match[1]);
        std::string comps_str = match[2];
        comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
        auto comps = get_comps(comps_str, implicit_function);
        // only accept fusion of two computations
        assert(comps.size() == 2);
        implicit_function->fuse_comps_sched_graph(comps[0], comps[1], level);

        tiramisu::prepare_schedules_for_legality_checks(true);
        std::vector<int> levels = {};
        for (int i = 0; i <= level; i++)
        {
            levels.push_back(i);
        }
        std::vector<std::tuple<tiramisu::var, int>> factors = tiramisu::global::get_implicit_function()->correcting_loop_fusion_with_shifting({comps[0]}, *comps[1], levels);
        for (const auto &tuple : factors)
        {
            tiramisu::var var = std::get<0>(tuple);
            int value = std::get<1>(tuple);

            if (value != 0)
            {
                comps[1]->shift(var, value);
            }
        }

        is_legal &= factors.size() > 0;
        break;
    }
    case 'T':
    {
        if (action_str[1] == '1')
        {
            std::string regex_str = "T1\\(L(\\d),(\\d+),comps=\\[([\\w', ]*)\\]\\)";
            std::regex re(regex_str);
            std::smatch match;
            parse_or_throw(action_str, match, re);
            int level1 = std::stoi(match[1]);
            int factor1 = std::stoi(match[2]);
            std::string comps_str = match[3];
            comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
            // COMPS NEED TO BE ORDERED BY APPEARANCE
            auto comps = get_comps(comps_str, implicit_function);
            for (auto comp : comps)
            {
                comp->tile(level1, factor1);
            }
            if (comps.size() > 1)
                implicit_function->fuse_comps_after_tiling(comps, 1);
        }
        else if (action_str[1] == '2')
        {
            std::string regex_str = "T2\\(L(\\d),L(\\d),(\\d+),(\\d+),comps=\\[([\\w', ]*)\\]\\)";
            std::regex re(regex_str);
            std::smatch match;
            parse_or_throw(action_str, match, re);
            int level1 = std::stoi(match[1]);
            int level2 = std::stoi(match[2]);
            int factor1 = std::stoi(match[3]);
            int factor2 = std::stoi(match[4]);
            std::string comps_str = match[5];
            comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
            // COMPS NEED TO BE ORDERED BY APPEARANCE
            auto comps = get_comps(comps_str, implicit_function);
            for (auto comp : comps)
            {
                comp->tile(level1, level2, factor1, factor2);
            }
            if (comps.size() > 1)
                implicit_function->fuse_comps_after_tiling(comps, 2);
        }
        else if (action_str[1] == '3')
        {
            std::string regex_str = "T3\\(L(\\d),L(\\d),L(\\d),(\\d+),(\\d+),(\\d+),comps=\\[([\\w', ]*)\\]\\)";
            std::regex re(regex_str);
            std::smatch match;
            parse_or_throw(action_str, match, re);
            int level1 = std::stoi(match[1]);
            int level2 = std::stoi(match[2]);
            int level3 = std::stoi(match[3]);
            int factor1 = std::stoi(match[4]);
            int factor2 = std::stoi(match[5]);
            int factor3 = std::stoi(match[6]);
            std::string comps_str = match[7];
            comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
            auto comps = get_comps(comps_str, implicit_function);
            for (auto comp : comps)
            {
                comp->tile(level1, level2, level3, factor1, factor2, factor3);
            }
            if (comps.size() > 1)
                implicit_function->fuse_comps_after_tiling(comps, 3);
        }
        else
        {
            throw std::invalid_argument("Tiling only supports 1D, 2D, and 3D");
        }
        break;
    }
    case 'M':
    {
        std::string regex_str = "M\\(\\[([\\d, ]+)\\],comps=\\[([\\w', ]*)\\]\\)";
        std::regex re(regex_str);
        std::smatch match;
        parse_or_throw(action_str, match, re);
        std::string factors_str = match[1];
        factors_str.erase(std::remove_if(factors_str.begin(), factors_str.end(), isSingleQuoteOrWhiteSpace), factors_str.end());
        std::vector<int> factors;
        // go through the string and extract the factors
        std::stringstream ss(factors_str);
        int i;
        while (ss >> i)
        {
            factors.push_back(i);
            if (ss.peek() == ',')
                ss.ignore();
        }

        // get the square root of the number of factors
        int num_factors = factors.size();
        int num_dims = (int)std::sqrt(num_factors);

        // check that the number of factors is a perfect square
        assert(num_dims * num_dims == num_factors);

        // seperate the factors into the different dimensions
        std::vector<std::vector<int>> factors_2d;
        for (int i = 0; i < num_dims; i++)
        {
            std::vector<int> row;
            for (int j = 0; j < num_dims; j++)
            {
                row.push_back(factors[i * num_dims + j]);
            }
            factors_2d.push_back(row);
        }

        std::string comps_str = match[2];
        comps_str.erase(std::remove_if(comps_str.begin(), comps_str.end(), isSingleQuoteOrWhiteSpace), comps_str.end());
        auto comps = get_comps(comps_str, implicit_function);

        // check that there is only one computation
        assert(comps.size() == 1);

        // apply the transformation
        comps[0]->matrix_transform(factors_2d);
        break;
    }

    default:
        throw std::invalid_argument(
            "Unknown schedule token: " + action_str);
    }

    return is_legal;
}

bool apply_actions_from_schedule_str(std::string schedule_str, tiramisu::function *implicit_function, Result &result)
{
    // Normalize: drop all whitespace and canonicalize quotes so the
    // per-action regexes can stay strict. Tiramisu identifiers are \w+,
    // so stripping whitespace inside the schedule string is lossless.
    schedule_str.erase(std::remove_if(schedule_str.begin(), schedule_str.end(),
                                      [](unsigned char c) { return std::isspace(c); }),
                       schedule_str.end());
    std::replace(schedule_str.begin(), schedule_str.end(), '"', '\'');

    std::string delimiter = "|";
    size_t pos = 0;
    std::string token;
    bool is_legal = true;

    while ((pos = schedule_str.find(delimiter)) != std::string::npos)
    {
        token = schedule_str.substr(0, pos);
        // std::cout << token << std::endl;
        is_legal &= apply_action(token, implicit_function, result);
        schedule_str.erase(0, pos + delimiter.length());
    }

    // std::cout << schedule_str << std::endl;
    is_legal &= apply_action(schedule_str, implicit_function, result);

    return is_legal;
}

// Phase timing markers (stderr), enabled with TIRALIB_TIMING=1. Used to attribute
// the wall-clock of one server operation to its phases; off by default (zero cost).
namespace
{
struct PhaseTimer
{
    bool on;
    std::chrono::steady_clock::time_point t;
    PhaseTimer()
    {
        const char *e = getenv("TIRALIB_TIMING");
        on = e && e[0] == '1';
        t = std::chrono::steady_clock::now();
    }
    void mark(const char *phase)
    {
        if (!on)
            return;
        auto now = std::chrono::steady_clock::now();
        double ms = std::chrono::duration<double, std::milli>(now - t).count();
        fprintf(stderr, "TIRALIB_TIMING %s %.3f ms\n", phase, ms);
        t = now;
    }
};
} // namespace

Result schedule_str_to_result(std::string function_name, std::string schedule_str, Operation operation, std::vector<tiramisu::buffer *> buffers)
{
    Result result = {
        .name = function_name,
        .legality = false,
        .exec_times = "",
        .additional_info = "",
        .success = true,
    };

    PhaseTimer pt;
    auto implicit_function = tiramisu::global::get_implicit_function();

    tiramisu::prepare_schedules_for_legality_checks();
    tiramisu::perform_full_dependency_analysis();
    pt.mark("prepare_and_dependency_analysis");
    bool is_legal = true;

    is_legal &= apply_actions_from_schedule_str(schedule_str, implicit_function, result);
    pt.mark("apply_actions");

    tiramisu::prepare_schedules_for_legality_checks();
    is_legal &= tiramisu::check_legality_of_function();
    // Re-verify parallelization on the final schedule: a parallel tag applied
    // earlier may have been invalidated by a later interchange/tiling.
    is_legal &= tiramisu::check_legality_of_parallelism();
    result.legality = is_legal;
    pt.mark("legality_checks");
    // Code-generation AST construction is only valid after the transformed
    // schedule passes legality.  Besides avoiding work for rejected schedules,
    // this keeps illegal helper/update domains out of ISL's AST builder.
    if (is_legal)
    {
        implicit_function->gen_time_space_domain();
        implicit_function->gen_isl_ast();
        result.isl_ast =
            implicit_function->generate_isl_ast_representation_string(
                nullptr, 0, "");
        pt.mark("isl_ast_generation");
    }

    bool should_execute = operation == Operation::execution ||
                          operation == Operation::execution_no_check;
    bool legality_required = operation != Operation::execution_no_check;
    if (should_execute && (is_legal || !legality_required))
    {
        tiramisu::codegen(buffers, function_name + ".o");
        pt.mark("halide_codegen_obj");

        std::string gpp_command = "g++";
        std::string wrapper_cmd = "./" + function_name + "_wrapper";

        std::string gcc_cmd = gpp_command + " -shared -o " + function_name + ".o.so " + function_name + ".o";
        // run the command and retrieve the execution status
        int status = system(gcc_cmd.c_str());
        assert(status != 139 && "Segmentation Fault when trying to execute schedule");
        pt.mark("link_shared_obj");
        // write the wrapper to a file if it does not exist
        if (!file_exists(function_name + "_wrapper"))
        {
// if USE_SQLITE is defined, write the wrapper to a file else raise an error
#ifdef USE_SQLITE
            if (write_wrapper_from_db(function_name))
            {
                std::cout << "Error: could not write wrapper to file" << std::endl;
                // exit with error
                exit(1);
            };
#else
            compile_wrapper(function_name);
            pt.mark("wrapper_compile");
#endif
        }
        // run the wrapper
        auto res_tuple = exec(wrapper_cmd.c_str());
        result.success = std::get<0>(res_tuple);
        result.exec_times = std::get<1>(res_tuple);
        pt.mark("wrapper_run");
        // remove new line character
        if (!result.exec_times.empty() && result.exec_times[result.exec_times.length() - 1] == '\n')
        {
            result.exec_times.erase(result.exec_times.length() - 1);
        }
    }
    return result;
}

void schedule_str_to_result_str(std::string function_name, std::string schedule_str, Operation operation, std::vector<tiramisu::buffer *> buffers)
{
    if (operation == Operation::annotations)
    {
        auto ast = tiramisu::auto_scheduler::syntax_tree(tiramisu::global::get_implicit_function(), {});
        std::string program_json = tiramisu::auto_scheduler::evaluate_by_learning_model::get_program_json(ast);
        std::cout << program_json;
        return;
    }

    auto result = schedule_str_to_result(function_name, schedule_str, operation, buffers);
    std::cout << serialize_result(result) << std::endl;
}
