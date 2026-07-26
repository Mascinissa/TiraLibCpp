#include <tiramisu/tiramisu.h>
#include <TiraLibCPP/utils.h>
#include <unordered_set>
// #include "function_floyd_warshall_MINI_wrapper.h"

using namespace tiramisu;

// Tiramisu Helpers
tiramisu::computation *get_computation_by_name(std::string comp_name, tiramisu::function *implicit_function)
{
    auto comps = implicit_function->get_computation_by_name(comp_name);
    if (comps.size() == 0)
    {
        // std::cout << "No computation with name " << first_comp << std::endl;
        // throw error
        throw std::invalid_argument("No computation with name " + comp_name);
        return nullptr;
    }
    else if (comps.size() > 1)
    {
        // std::cout << "More than one computation with name " << first_comp << std::endl;
        // throw error
        throw std::invalid_argument("More than one computation with name " + comp_name);
        return nullptr;
    }
    else
    {
        return comps[0];
    }
}

// String Parsing Helpers
bool isSingleQuoteOrWhiteSpace(char c)
{
    return (c == '\'' || std::isspace(c));
}

std::string get_first_comp(std::string comps_str)
{
    std::string delimiter = ",";
    size_t pos = 0;
    std::string token;
    pos = comps_str.find(delimiter);
    token = comps_str.substr(0, pos);
    return token;
}

std::vector<tiramisu::computation *> get_comps(std::string comps_str, tiramisu::function *implicit_function)
{
    std::vector<tiramisu::computation *> comps;
    std::unordered_set<tiramisu::computation *> seen;
    std::string delimiter = ",";
    size_t pos = 0;
    std::string token;
    while ((pos = comps_str.find(delimiter)) != std::string::npos)
    {
        token = comps_str.substr(0, pos);
        auto comp = get_computation_by_name(token, implicit_function);
        if (seen.insert(comp).second)
            comps.push_back(comp);
        comps_str.erase(0, pos + delimiter.length());
    }
    auto comp = get_computation_by_name(comps_str, implicit_function);
    if (seen.insert(comp).second)
        comps.push_back(comp);
    return comps;
}

Operation get_operation_from_string(std::string operation_str)
{
    if (operation_str == "legality")
        return Operation::legality;
    else if (operation_str == "execution")
        return Operation::execution;
    else if (operation_str == "annotations")
        return Operation::annotations;
    else if (operation_str == "execution_no_check")
        return Operation::execution_no_check;
    else
        assert(false && "Unknown operation");
}

// Compile and Exec Helpers
bool file_exists(const std::string &name)
{
    if (FILE *file = fopen(name.c_str(), "r"))
    {
        fclose(file);
        return true;
    }
    else
    {
        return false;
    }
}

std::tuple<bool, std::string> exec(const char *cmd)
{
    std::array<char, 128> buffer;
    std::string result;
    // std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    auto pipe = popen(cmd, "r");
    if (!pipe)
    {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
    {
        result += buffer.data();
    }
    auto rc = pclose(pipe);
    return std::make_tuple(rc == 0, result);
}

int compile_wrapper(std::string function_name)
{
    if (file_exists(function_name + "_wrapper.cpp"))
    {
        std::string compile_wrapper_command = "c++ -std=c++17 -fno-rtti -I${TIRAMISU_ROOT}/include -I${TIRAMISU_ROOT}/3rdParty/Halide/install/include -I${TIRAMISU_ROOT}/3rdParty/isl/include/ -I${TIRAMISU_ROOT}/benchmarks -L${TIRAMISU_ROOT}/build -L${TIRAMISU_ROOT}/3rdParty/Halide/install/lib64/ -L${TIRAMISU_ROOT}/3rdParty/isl/build/lib -o " + function_name + "_wrapper -ltiramisu -lHalide -ldl -lpthread -fopenmp -lm -Wl,-rpath,${TIRAMISU_ROOT}/build ./" + function_name + "_wrapper.cpp ./" + function_name + ".o.so -ltiramisu -lHalide -ldl -lpthread -fopenmp -lm -lisl";

        // run the command to compile the wrapper
        int status = system(compile_wrapper_command.c_str());
        assert(status != 139 && "Segmentation Fault when trying to execute schedule");
        return 0;
    }
    return -1;
}

// Serialization Helpers
std::string serialize_result(Result &result)
{
    std::string result_str = "{";
    result_str += "\"name\": \"" + result.name + "\",";
    result_str += "\"legality\": " + std::to_string(result.legality) + ",";
    result_str += "\"isl_ast\": \"" + result.isl_ast + "\",";
    result_str += "\"exec_times\": \"" + result.exec_times + "\",";
    result_str += "\"success\": " + std::to_string(result.success) + ",";
    result_str += "\"additional_info\": \"" + result.additional_info + "\"";
    result_str += "}";
    return result_str;
}
