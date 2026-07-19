#pragma once

#include <tiramisu/tiramisu.h>
#include <tiramisu/auto_scheduler/evaluator.h>
#include <tiramisu/auto_scheduler/search_method.h>

#include <chrono>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

enum Operation
{
    legality = 0,
    execution = 1,
    annotations = 2,
    skewing_solver = 3,
    execution_no_check = 4,
};

struct Result
{
    std::string name;
    bool legality;
    std::string isl_ast;
    std::string exec_times;
    std::string additional_info;
    bool success;
};

tiramisu::computation *get_computation_by_name(std::string comp_name, tiramisu::function *implicit_function);

bool isSingleQuoteOrWhiteSpace(char c);

std::string get_first_comp(std::string comps_str);

std::vector<tiramisu::computation *> get_comps(std::string comps_str, tiramisu::function *implicit_function);

Operation get_operation_from_string(std::string operation_str);

bool file_exists(const std::string &name);

std::tuple<bool, std::string> exec(const char *cmd);

int compile_wrapper(std::string function_name);

std::string serialize_result(Result &result);
