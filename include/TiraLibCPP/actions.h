#pragma once

#include <tiramisu/tiramisu.h>
#include <TiraLibCPP/utils.h>

bool apply_action(std::string action_str, tiramisu::function *implicit_function, Result &result);

bool apply_actions_from_schedule_str(std::string schedule_str, tiramisu::function *implicit_function, Result &result);

Result schedule_str_to_result(std::string function_name, std::string schedule_str, Operation operation, std::vector<tiramisu::buffer *> buffers, bool skip_prep = false, const std::string &obj_tag = "");

void schedule_str_to_result_str(std::string function_name, std::string schedule_str, Operation operation, std::vector<tiramisu::buffer *> buffers);

// Persistent fork-server: the parent builds the function, runs prepare + full
// dependency analysis ONCE, then serves one request per stdin line, forking a
// fresh child per request (identical isolation semantics to one process per
// operation, without re-paying process spawn, dylib load, function construction
// and dependency analysis every time).
int run_server_loop(std::string function_name, std::vector<tiramisu::buffer *> buffers);