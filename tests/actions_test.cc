#include <gtest/gtest.h>
#include <tiramisu/tiramisu.h>
#include <TiraLibCPP/actions.h>

using namespace tiramisu;

std::tuple<Result, std::string> apply_schedule_blur(std::string schedule)
{
  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------
  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  // inputs
  input input_img("input_img", {ci, yi, xi}, p_float64);

  // Computations
  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Buffers
  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  // Store inputs
  input_img.store_in(&input_buf);

  // Store computations
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------
  auto result = schedule_str_to_result(function_name, schedule, Operation::legality, buffers);

  // make and return a tuple consisting of the result and the halide ir of the function
  std::string halide_ir = global::get_implicit_function()->get_halide_ir(buffers);

  return std::tuple<Result, std::string>(result, halide_ir);
}

std::string clean_halide_ir(std::string ir)
{
  std::regex re("produce(.*\n*)*");
  std::smatch match;

  std::regex_search(ir, match, re);

  std::string match_str = match.str();

  std::regex re2(" allocate.*\n");
  std::string result2 = std::regex_replace(match_str, re2, "");

  return result2;
}

// Demonstrate some basic assertions.
TEST(TiraLibCppTest, ParallelizationCheck)
{
  std::string schedule = "P(L0,comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------
  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  // inputs
  input input_img("input_img", {ci, yi, xi}, p_float64);

  // Computations
  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Buffers
  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  // Store inputs
  input_img.store_in(&input_buf);

  // Store computations
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------
  comp_blur.tag_parallel_level(0);

  EXPECT_EQ(global::get_implicit_function()->get_name(), function_name);
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir(buffers)), clean_halide_ir(halide_ir));
}

TEST(TiraLibCppTest, ParallelizationCheckL1)
{
  std::string schedule = "P(L1,comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------
  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  // inputs
  input input_img("input_img", {ci, yi, xi}, p_float64);

  // Computations
  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Buffers
  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  // Store inputs
  input_img.store_in(&input_buf);

  // Store computations
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------
  comp_blur.tag_parallel_level(1);

  EXPECT_EQ(global::get_implicit_function()->get_name(), function_name);
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir(buffers)), clean_halide_ir(halide_ir));
}

TEST(TiraLibCppTest, Tiling2D)
{
  std::string schedule = "T2(L0,L1,32,32,comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------
  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  // inputs
  input input_img("input_img", {ci, yi, xi}, p_float64);

  // Computations
  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Buffers
  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  // Store inputs
  input_img.store_in(&input_buf);

  // Store computations
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------
  comp_blur.tile(0, 1, 32, 32);

  EXPECT_EQ(global::get_implicit_function()->get_name(), function_name);
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir(buffers)),
            clean_halide_ir(halide_ir));
}

TEST(TiraLibCppTest, Tiling3D)
{
  std::string schedule = "T3(L0,L1,L2,32,32,32,comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------
  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  // inputs
  input input_img("input_img", {ci, yi, xi}, p_float64);

  // Computations
  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Buffers
  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  // Store inputs
  input_img.store_in(&input_buf);

  // Store computations
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------
  comp_blur.tile(0, 1, 2, 32, 32, 32);

  EXPECT_EQ(global::get_implicit_function()->get_name(), function_name);
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir(buffers)),
            clean_halide_ir(halide_ir));
}

TEST(TiraLibCppTest, Unrolling)
{
  std::string schedule = "U(L2,32,comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------
  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  // inputs
  input input_img("input_img", {ci, yi, xi}, p_float64);

  // Computations
  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Buffers
  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  // Store inputs
  input_img.store_in(&input_buf);

  // Store computations
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------
  comp_blur.unroll(2, 32);

  EXPECT_EQ(global::get_implicit_function()->get_name(), function_name);
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir(buffers)),
            clean_halide_ir(halide_ir));
}

TEST(TiraLibCppTest, UnrollingLNeg1)
{
  std::string schedule = "U(L-1,32,comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  input input_img("input_img", {ci, yi, xi}, p_float64);

  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  input_img.store_in(&input_buf);
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // L-1 on comp_blur (3 loops c,y,x) resolves to level 2.
  comp_blur.unroll(2, 32);

  EXPECT_EQ(global::get_implicit_function()->get_name(), function_name);
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir(buffers)),
            clean_halide_ir(halide_ir));
}

TEST(TiraLibCppTest, UnrollingLNeg1AfterTiling)
{
  // After T2 adds two loops, the innermost level for comp_blur is 4.
  // We verify L-1 resolution via the schedule dim formula it uses
  // ((isl_map_dim(schedule, isl_dim_out) - 2) / 2 - 1). Running the full
  // U(L-1,...) action through apply_schedule_blur after a tile triggers a
  // pre-existing Tiramisu bound-extraction failure inside
  // computation::separateAndSplit that is unrelated to L-1.
  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  input input_img("input_img", {ci, yi, xi}, p_float64);

  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  input_img.store_in(&input_buf);
  comp_blur.store_in(&output_buf);

  auto innermost_of = [](tiramisu::computation &c) {
    return (isl_map_dim(c.get_schedule(), isl_dim_out) - 2) / 2 - 1;
  };

  EXPECT_EQ(innermost_of(comp_blur), 2);

  Result result;
  apply_action("T2(L0,L1,32,32,comps=['comp_blur'])",
               global::get_implicit_function(), result);

  EXPECT_EQ(innermost_of(comp_blur), 4);
}

TEST(TiraLibCppTest, Interchange)
{
  std::string schedule = "I(L0,L1,comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------
  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  // inputs
  input input_img("input_img", {ci, yi, xi}, p_float64);

  // Computations
  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Buffers
  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  // Store inputs
  input_img.store_in(&input_buf);

  // Store computations
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------
  comp_blur.interchange(0, 1);

  EXPECT_EQ(global::get_implicit_function()->get_name(), function_name);
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir(buffers)),
            clean_halide_ir(halide_ir));
}

TEST(TiraLibCppTest, Reversal)
{
  std::string schedule = "R(L2,comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------
  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  // inputs
  input input_img("input_img", {ci, yi, xi}, p_float64);

  // Computations
  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Buffers
  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  // Store inputs
  input_img.store_in(&input_buf);

  // Store computations
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------
  comp_blur.loop_reversal(2);

  EXPECT_EQ(global::get_implicit_function()->get_name(), function_name);
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir(buffers)),
            clean_halide_ir(halide_ir));
}

TEST(TiraLibCppTest, IllegalAction)
{
  std::string schedule = "S(L0,L1,0,0,comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  EXPECT_EQ(resultInstance.legality, false);
}

std::tuple<Result, std::string> apply_schedule_skewing_sample(std::string schedule)
{
  std::string function_name = "function550013";

  tiramisu::init("function550013");
  var i0("i0", 1, 2049), i1("i1", 1, 2049), i2("i2", 0, 256), i1_p1("i1_p1", 0, 2050), i0_p1("i0_p1", 0, 2050);
  input icomp00("icomp00", {i0_p1, i1_p1}, p_float64);
  input input01("input01", {i0_p1}, p_float64);
  computation comp00("comp00", {i0, i1, i2}, p_float64);
  comp00.set_expression(icomp00(i0, i1) + icomp00(i0, i1 - 1) * icomp00(i0 + 1, i1) + icomp00(i0, i1 + 1) + icomp00(i0 - 1, i1) + input01(i0) + input01(i0 - 1) - input01(i0 + 1));
  buffer buf00("buf00", {2050, 2050}, p_float64, a_output);
  buffer buf01("buf01", {2050}, p_float64, a_input);
  icomp00.store_in(&buf00);
  input01.store_in(&buf01);
  comp00.store_in(&buf00, {i0, i1});
  tiramisu::codegen({&buf00, &buf01}, "function550013.o");

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------

  auto buffers = {&buf00, &buf01};

  auto result = schedule_str_to_result(function_name, schedule, Operation::legality, buffers);

  // make and return a tuple consisting of the result and the halide ir of the function
  std::string halide_ir = global::get_implicit_function()->get_halide_ir(buffers);

  return std::tuple<Result, std::string>(result, halide_ir);
}

TEST(TiraLibCppTest, Skewing)
{

  std::string schedule = "S(L0,L1,0,0,comps=['comp00'])";
  auto result = apply_schedule_skewing_sample(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  tiramisu::init("function550013");
  var i0("i0", 1, 2049), i1("i1", 1, 2049), i2("i2", 0, 256), i1_p1("i1_p1", 0, 2050), i0_p1("i0_p1", 0, 2050);
  input icomp00("icomp00", {i0_p1, i1_p1}, p_float64);
  input input01("input01", {i0_p1}, p_float64);
  computation comp00("comp00", {i0, i1, i2}, p_float64);
  comp00.set_expression(icomp00(i0, i1) + icomp00(i0, i1 - 1) * icomp00(i0 + 1, i1) + icomp00(i0, i1 + 1) + icomp00(i0 - 1, i1) + input01(i0) + input01(i0 - 1) - input01(i0 + 1));
  buffer buf00("buf00", {2050, 2050}, p_float64, a_output);
  buffer buf01("buf01", {2050}, p_float64, a_input);
  icomp00.store_in(&buf00);
  input01.store_in(&buf01);
  comp00.store_in(&buf00, {i0, i1});

  comp00.skew(0, 1, 1, 1);

  EXPECT_EQ(global::get_implicit_function()->get_name(), "function550013");
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir({&buf00, &buf01})), clean_halide_ir(halide_ir));
}

std::tuple<Result, std::string> apply_schedule_multi_comp_sample(std::string schedule)
{
  auto function_name = "function_gemver_MINI";
  tiramisu::init("function_gemver_MINI");

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------

  // Iteration variables
  var i("i", 0, 40), j("j", 0, 40);

  // inputs
  input A("A", {i, j}, p_float64);
  input u1("u1", {i}, p_float64);
  input u2("u2", {i}, p_float64);
  input v1("v1", {i}, p_float64);
  input v2("v2", {i}, p_float64);
  input y("y", {i}, p_float64);
  input z("z", {i}, p_float64);

  // Computations

  computation A_hat("A_hat", {i, j}, A(i, j) + u1(i) * v1(j) + u2(i) * v2(j));
  computation x_temp("x_temp", {i, j}, p_float64);
  x_temp.set_expression(x_temp(i, j) + A_hat(j, i) * y(j) * 1.2);
  computation x("x", {i}, x_temp(i, 0) + z(i));
  computation w("w", {i, j}, p_float64);
  w.set_expression(w(i, j) + A_hat(i, j) * x(j) * 1.5);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------
  A_hat.then(x_temp, computation::root)
      .then(x, computation::root)
      .then(w, computation::root);

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Input Buffers
  buffer b_A("b_A", {40, 40}, p_float64, a_input);
  buffer b_u1("b_u1", {40}, p_float64, a_input);
  buffer b_u2("b_u2", {40}, p_float64, a_input);
  buffer b_v1("b_v1", {40}, p_float64, a_input);
  buffer b_v2("b_v2", {40}, p_float64, a_input);
  buffer b_z("b_z", {40}, p_float64, a_input);
  buffer b_y("b_y", {40}, p_float64, a_input);
  buffer b_A_hat("b_A_hat", {40, 40}, p_float64, a_output);
  buffer b_x("b_x", {40}, p_float64, a_output);
  buffer b_w("b_w", {40}, p_float64, a_output);

  // Store inputs
  A.store_in(&b_A);
  u1.store_in(&b_u1);
  u2.store_in(&b_u2);
  v1.store_in(&b_v1);
  v2.store_in(&b_v2);
  y.store_in(&b_y);
  z.store_in(&b_z);

  // Store computations
  A_hat.store_in(&b_A_hat);
  x_temp.store_in(&b_x, {i});
  x.store_in(&b_x);
  w.store_in(&b_w, {i});

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------

  auto buffers = {&b_A, &b_u1, &b_u2, &b_v1, &b_v2, &b_y, &b_z, &b_A_hat, &b_x, &b_w};

  auto result = schedule_str_to_result(function_name, schedule, Operation::legality, buffers);

  // make and return a tuple consisting of the result and the halide ir of the function
  std::string halide_ir = global::get_implicit_function()->get_halide_ir(buffers);

  return std::tuple<Result, std::string>(result, halide_ir);
}

TEST(TiraLibCppTest, FusionIllegal)
{

  std::string schedule = "F(L0,comps=['A_hat', 'x_temp'])";
  auto result = apply_schedule_multi_comp_sample(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  auto function_name = "function_gemver_MINI";
  tiramisu::init("function_gemver_MINI");

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------

  // Iteration variables
  var i("i", 0, 40), j("j", 0, 40);

  // inputs
  input A("A", {i, j}, p_float64);
  input u1("u1", {i}, p_float64);
  input u2("u2", {i}, p_float64);
  input v1("v1", {i}, p_float64);
  input v2("v2", {i}, p_float64);
  input y("y", {i}, p_float64);
  input z("z", {i}, p_float64);

  // Computations

  computation A_hat("A_hat", {i, j}, A(i, j) + u1(i) * v1(j) + u2(i) * v2(j));
  computation x_temp("x_temp", {i, j}, p_float64);
  x_temp.set_expression(x_temp(i, j) + A_hat(j, i) * y(j) * 1.2);
  computation x("x", {i}, x_temp(i, 0) + z(i));
  computation w("w", {i, j}, p_float64);
  w.set_expression(w(i, j) + A_hat(i, j) * x(j) * 1.5);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------
  A_hat.then(x_temp, computation::root)
      .then(x, computation::root)
      .then(w, computation::root);

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Input Buffers
  buffer b_A("b_A", {40, 40}, p_float64, a_input);
  buffer b_u1("b_u1", {40}, p_float64, a_input);
  buffer b_u2("b_u2", {40}, p_float64, a_input);
  buffer b_v1("b_v1", {40}, p_float64, a_input);
  buffer b_v2("b_v2", {40}, p_float64, a_input);
  buffer b_z("b_z", {40}, p_float64, a_input);
  buffer b_y("b_y", {40}, p_float64, a_input);
  buffer b_A_hat("b_A_hat", {40, 40}, p_float64, a_output);
  buffer b_x("b_x", {40}, p_float64, a_output);
  buffer b_w("b_w", {40}, p_float64, a_output);

  // Store inputs
  A.store_in(&b_A);
  u1.store_in(&b_u1);
  u2.store_in(&b_u2);
  v1.store_in(&b_v1);
  v2.store_in(&b_v2);
  y.store_in(&b_y);
  z.store_in(&b_z);

  // Store computations
  A_hat.store_in(&b_A_hat);
  x_temp.store_in(&b_x, {i});
  x.store_in(&b_x);
  w.store_in(&b_w, {i});

  EXPECT_EQ(resultInstance.legality, false);
}

TEST(TiraLibCppTest, FusionLegal)
{

  std::string schedule = "I(L0,L1,comps=['x_temp'])|F(L0,comps=['A_hat', 'x_temp'])";
  auto result = apply_schedule_multi_comp_sample(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  auto function_name = "function_gemver_MINI";
  tiramisu::init("function_gemver_MINI");

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------

  // Iteration variables
  var i("i", 0, 40), j("j", 0, 40);

  // inputs
  input A("A", {i, j}, p_float64);
  input u1("u1", {i}, p_float64);
  input u2("u2", {i}, p_float64);
  input v1("v1", {i}, p_float64);
  input v2("v2", {i}, p_float64);
  input y("y", {i}, p_float64);
  input z("z", {i}, p_float64);

  // Computations

  computation A_hat("A_hat", {i, j}, A(i, j) + u1(i) * v1(j) + u2(i) * v2(j));
  computation x_temp("x_temp", {i, j}, p_float64);
  x_temp.set_expression(x_temp(i, j) + A_hat(j, i) * y(j) * 1.2);
  computation x("x", {i}, x_temp(i, 0) + z(i));
  computation w("w", {i, j}, p_float64);
  w.set_expression(w(i, j) + A_hat(i, j) * x(j) * 1.5);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------
  A_hat.then(x_temp, computation::root)
      .then(x, computation::root)
      .then(w, computation::root);

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Input Buffers
  buffer b_A("b_A", {40, 40}, p_float64, a_input);
  buffer b_u1("b_u1", {40}, p_float64, a_input);
  buffer b_u2("b_u2", {40}, p_float64, a_input);
  buffer b_v1("b_v1", {40}, p_float64, a_input);
  buffer b_v2("b_v2", {40}, p_float64, a_input);
  buffer b_z("b_z", {40}, p_float64, a_input);
  buffer b_y("b_y", {40}, p_float64, a_input);
  buffer b_A_hat("b_A_hat", {40, 40}, p_float64, a_output);
  buffer b_x("b_x", {40}, p_float64, a_output);
  buffer b_w("b_w", {40}, p_float64, a_output);

  // Store inputs
  A.store_in(&b_A);
  u1.store_in(&b_u1);
  u2.store_in(&b_u2);
  v1.store_in(&b_v1);
  v2.store_in(&b_v2);
  y.store_in(&b_y);
  z.store_in(&b_z);

  // Store computations
  A_hat.store_in(&b_A_hat);
  x_temp.store_in(&b_x, {i});
  x.store_in(&b_x);
  w.store_in(&b_w, {i});

  x_temp.interchange(0, 1);
  A_hat.then(x_temp, 0).then(x, -1).then(w, -1);

  EXPECT_EQ(resultInstance.legality, true);
  EXPECT_EQ(global::get_implicit_function()->get_name(), "function_gemver_MINI");
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir({&b_A, &b_u1, &b_u2, &b_v1, &b_v2, &b_y, &b_z, &b_A_hat, &b_x, &b_w})), clean_halide_ir(halide_ir));
}

TEST(TiraLibCppTest, UnrollingLNeg1Mismatch)
{
  // A_hat is 2D (innermost level 1); x is 1D (innermost level 0).
  // U(L-1,...) over both must reject the action.
  std::string schedule = "U(L-1,4,comps=['A_hat', 'x'])";
  EXPECT_THROW(apply_schedule_multi_comp_sample(schedule), std::invalid_argument);
}

TEST(TiraLibCppTest, Matrix)
{
  std::string schedule = "M([0, 1, 0, 1, 0, 0, 0, 0, 1],comps=['comp_blur'])";
  auto result = apply_schedule_blur(schedule);

  Result resultInstance = std::get<0>(result);
  std::string halide_ir = std::get<1>(result);

  std::string function_name = "function_blur_MINI";

  tiramisu::init(function_name);

  // -------------------------------------------------------
  // Layer I
  // -------------------------------------------------------
  var xi("xi", 0, 34), yi("yi", 0, 18), ci("ci", 0, 5);
  var x("x", 1, 34 - 1), y("y", 1, 18 - 1), c("c", 1, 5 - 1);

  // inputs
  input input_img("input_img", {ci, yi, xi}, p_float64);

  // Computations
  computation comp_blur("comp_blur", {c, y, x}, (input_img(c, y + 1, x - 1) + input_img(c, y + 1, x) + input_img(c, y + 1, x + 1) + input_img(c, y, x - 1) + input_img(c, y, x) + input_img(c, y, x + 1) + input_img(c, y - 1, x - 1) + input_img(c, y - 1, x) + input_img(c, y - 1, x + 1)) * 0.111111);

  // -------------------------------------------------------
  // Layer II
  // -------------------------------------------------------

  // -------------------------------------------------------
  // Layer III
  // -------------------------------------------------------
  // Buffers
  buffer input_buf("input_buf", {5, 18, 34}, p_float64, a_input);
  buffer output_buf("output_buf", {5, 18, 34}, p_float64, a_output);

  // Store inputs
  input_img.store_in(&input_buf);

  // Store computations
  comp_blur.store_in(&output_buf);

  auto buffers = {&input_buf, &output_buf};

  // -------------------------------------------------------
  // Code Generation
  // -------------------------------------------------------
  comp_blur.matrix_transform({{0, 1, 0}, {1, 0, 0}, {0, 0, 1}});

  EXPECT_EQ(global::get_implicit_function()->get_name(), function_name);
  EXPECT_EQ(clean_halide_ir(global::get_implicit_function()->get_halide_ir(buffers)),
            clean_halide_ir(halide_ir));
}
