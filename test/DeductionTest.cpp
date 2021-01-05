#include "Deduction.h"
#include "SketchTree.h"
#include "Component.h"
#include "Util.h"
#include "z3++.h"
#include "gtest/gtest.h" // Google testing framework.
#include <list>
#include <string>
#include <sstream> //stringstream
#include <map>
#include <glob.h>
#include <iterator>

using namespace z3;

static const std::string toString(expr e) {
	std::ostringstream sstream;
    sstream << e;
	std::string str = sstream.str();
	return str;
}

static const std::string toString(expr* e) {
	std::ostringstream sstream;
    sstream << *e;
	std::string str = sstream.str();
	return str;
}

// convert string to z3 expr.
TEST(DeductionTest, strToExpr) {
    context c;
    std::string cst = "(declare-const CO Int)(declare-const CI Int)(declare-const RO Int)(declare-const RI Int)(assert (>= CO CI))(assert (<= RO RI))";
    expr* e = Util::convertStrToExpr(c, cst);
	EXPECT_EQ("(and (>= CO CI) (<= RO RI))", toString(e));

    std::string cst_true = "(declare-const CO Int)(declare-const CI Int)(declare-const RO Int)(declare-const RI Int)(assert true)";
    expr* e_true = Util::convertStrToExpr(c, cst_true);
	EXPECT_EQ("true", toString(e_true));
}

// convert string to z3 expr.
TEST(DeductionTest, worklist) {
    std::list<int> worklist;
    worklist.push_back(1);
    worklist.push_back(2);
    worklist.push_back(3);
    int i1 = worklist.front();
    worklist.pop_front();
    EXPECT_EQ(1, i1);
    int i2 = worklist.front();
    worklist.pop_front();
    EXPECT_EQ(2, i2);
}
// Replace variables of existing z3 expr.
TEST(DeductionTest, replaceVar) {
    context c;
    std::string cst = "(declare-const CO Int)(declare-const CI Int)(declare-const RO Int)(declare-const RI Int)(assert (>= CO CI))(assert (<= RO RI))";
    Util::replaceAll(cst, "CO", "CO1");
    Util::replaceAll(cst, "CI", "CI1");
    expr* e = Util::convertStrToExpr(c, cst);
    EXPECT_EQ("(and (>= CO1 CI1) (<= RO RI))", toString(e));
}


// Deduction with only one component in the tree
TEST(DeductionTest, deduction1) {
    std::map<int, Component*> spec_map;
    std::string spread_loc = "../specs/L3/spread.json";
    std::string gather_loc = "../specs/L3/gather.json";

    Component* comp1 = new Component(spread_loc);
    spec_map.insert(std::make_pair(comp1->getId(), comp1));

    Component* comp2 = new Component(gather_loc);
    spec_map.insert(std::make_pair(comp2->getId(), comp2));


    //Deduction Tree from Ruben.
    std::vector<int> spread_vec;
    //SketchTree tree();
    //spread
    spread_vec.push_back(1);
    spread_vec.push_back(0);
    spread_vec.push_back(-1);
    //tree.initByArray(sketch_vec);
    //gather
    std::vector<int> gather_vec;
    gather_vec.push_back(2);
    gather_vec.push_back(0);
    gather_vec.push_back(-1);

    //Read spec

    //Input/output constraints
    context c;
    expr col_input = c.int_const("COL_IN_99_0");
    expr row_input = c.int_const("ROW_IN_99_0");
    //input: 2X2
    expr col_input_cst = (col_input == 2);
    expr row_input_cst = (row_input == 2);
    expr input_cst = (col_input_cst && row_input_cst);
    //std::cout << "input_cst: " <<  input_cst << "\n";
    std::vector<expr> vec_inputs;
    vec_inputs.push_back(input_cst);
    //output: 1X2
    expr col_output = c.int_const("COL_OUT_99_0");
    expr row_output = c.int_const("ROW_OUT_99_0");
    expr col_output_cst = (col_output == 2);
    expr row_output_cst = (row_output == 1);
    expr output_cst = (col_output_cst && row_output_cst);
    //std::cout << "output_cst: " <<  output_cst << "\n";

    //do deduction
    Deduction deduct(spec_map);
    deduct.buildSystem(spread_vec, output_cst, vec_inputs);
    bool spread_res = deduct.solve();
	EXPECT_EQ(true, spread_res);

    deduct.buildSystem(gather_vec, output_cst, vec_inputs);
    bool gather_res = deduct.solve();
	EXPECT_EQ(false, gather_res);

    //pointer operation.
    std::map<int, z3::expr*> node_expr_map;
    node_expr_map.insert(std::make_pair(1, &output_cst));
	EXPECT_EQ(*node_expr_map[1], output_cst);

}

// Deduction with two components
TEST(DeductionTest, deduction2) {
    std::map<int, Component*> spec_map;
    std::string spread_loc = "../specs/L3/filter.json";
    std::string gather_loc = "../specs/L3/select.json";

    Component* comp1 = new Component(spread_loc);
    spec_map.insert(std::make_pair(comp1->getId(), comp1));

    Component* comp2 = new Component(gather_loc);
    spec_map.insert(std::make_pair(comp2->getId(), comp2));


    //Deduction Tree from Ruben.
    std::vector<int> spread_vec;
    //SketchTree tree();
    //spread
    spread_vec.push_back(3);
    spread_vec.push_back(10);
    spread_vec.push_back(-1);
    spread_vec.push_back(0);
    spread_vec.push_back(-1);
    spread_vec.push_back(-1);
    spread_vec.push_back(-1);
    //Read spec

    //Input/output constraints
    context c;
    expr col_input = c.int_const("COL_IN_99_0");
    expr row_input = c.int_const("ROW_IN_99_0");
    //input: 2X2
    expr col_input_cst = (col_input == 2);
    expr row_input_cst = (row_input == 2);
    expr input_cst = (col_input_cst && row_input_cst);
    //std::cout << "input_cst: " <<  input_cst << "\n";
    std::vector<expr> vec_inputs;
    vec_inputs.push_back(input_cst);
    //output: 1X2
    expr col_output = c.int_const("COL_OUT_99_0");
    expr row_output = c.int_const("ROW_OUT_99_0");
    expr col_output_cst = (col_output == 2);
    expr row_output_cst = (row_output == 1);
    expr output_cst = (col_output_cst && row_output_cst);
    //std::cout << "output_cst: " <<  output_cst << "\n";

    //do deduction
    Deduction deduct(spec_map);
    deduct.buildSystem(spread_vec, output_cst, vec_inputs);
    bool spread_res = deduct.solve();
	EXPECT_EQ(false, spread_res);
}


// Deduction with two inputs 
TEST(DeductionTest, deduction3) {
    std::map<int, Component*> spec_map;
    std::string spread_loc = "../specs/L2/cbind.json";
    std::string gather_loc = "../specs/L2/rbind.json";

    Component* comp1 = new Component(spread_loc);
    spec_map.insert(std::make_pair(comp1->getId(), comp1));

    Component* comp2 = new Component(gather_loc);
    spec_map.insert(std::make_pair(comp2->getId(), comp2));


    //Deduction Tree from Ruben.
    std::vector<int> cbind_vec;
    std::vector<int> rbind_vec;
    //SketchTree tree();
    //cbind
    cbind_vec.push_back(8);
    cbind_vec.push_back(0);
    cbind_vec.push_back(0);

    rbind_vec.push_back(9);
    rbind_vec.push_back(0);
    rbind_vec.push_back(0);
    //Read spec

    //Input/output constraints
    context c;
    expr col_input = c.int_const("COL_IN_99_0");
    expr row_input = c.int_const("ROW_IN_99_0");
    //input1: 1X2
    expr row_input_cst = (row_input == 1);
    expr col_input_cst = (col_input == 2);
    expr input_cst = (col_input_cst && row_input_cst);

    expr col_input2 = c.int_const("COL_IN_99_1");
    expr row_input2 = c.int_const("ROW_IN_99_1");
    //input2: 1X2
    expr row_input_cst2 = (row_input2 == 1);
    expr col_input_cst2 = (col_input2 == 2);
    expr input_cst2 = (col_input_cst2 && row_input_cst2);


    //std::cout << "input_cst1: " <<  input_cst << "\n";
    //std::cout << "input_cst2: " <<  input_cst2 << "\n";
    std::vector<expr> vec_inputs;
    vec_inputs.push_back(input_cst);
    vec_inputs.push_back(input_cst2);
    //output: 1X2
    expr col_output = c.int_const("COL_OUT_99_0");
    expr row_output = c.int_const("ROW_OUT_99_0");
    expr col_output_cst = (col_output == 2);
    expr row_output_cst = (row_output == 2);
    expr output_cst = (col_output_cst && row_output_cst);
    //std::cout << "output_cst: " <<  output_cst << "\n";

    //do deduction
    Deduction deduct(spec_map);
    deduct.buildSystem(cbind_vec, output_cst, vec_inputs);
    bool cbind_res = deduct.solve();
    deduct.buildSystem(rbind_vec, output_cst, vec_inputs);
    bool rbind_res = deduct.solve();
	EXPECT_EQ(false, cbind_res);
	EXPECT_EQ(true, rbind_res);
}

//################Set encoding.
TEST(DeductionTest, c_api) {
    Z3_config  cfg;
    cfg = Z3_mk_config();
    Z3_context c = Z3_mk_context(cfg);
    Z3_solver s = Z3_mk_solver(c);

    Z3_sort str_ty = Z3_mk_string_sort(c);
    Z3_sort set_sort = Z3_mk_set_sort(c, str_ty);
    Z3_symbol  x_s  = Z3_mk_string_symbol(c, "x");
    Z3_ast set_x = Z3_mk_const(c, x_s, set_sort);
    Z3_symbol  y_s  = Z3_mk_string_symbol(c, "y");
    Z3_ast set_y = Z3_mk_const(c, y_s, set_sort);
    //Z3_ast subset_cst = Z3_mk_set_subset(c, set_x, set_y);
    Z3_ast emp_set = Z3_mk_empty_set(c, str_ty);
    Z3_ast intersect_array[2];
    intersect_array[0] = set_x;
    intersect_array[1] = set_y;
    Z3_ast s3 = Z3_mk_set_intersect(c, 2, intersect_array);
    Z3_ast assert1 = Z3_mk_eq(c, s3, emp_set);


    //printf("subset cst:\n%s\n", Z3_ast_to_string(c, assert1));

    /*Z3_ast emp = Z3_mk_empty_set(c, ty);
    Z3_ast s1 = Z3_mk_empty_set(c, ty);
    Z3_ast s2 = Z3_mk_empty_set(c, ty);
    Z3_ast one = Z3_mk_int(c, 1, ty);
    Z3_ast two = Z3_mk_int(c, 2, ty);
    Z3_ast three = Z3_mk_int(c, 3, ty);

    s1 = Z3_mk_set_add(c, s1, one);
    s1 = Z3_mk_set_add(c, s1, two);
    s2 = Z3_mk_set_add(c, s2, three);

    Z3_ast intersect_array[2];
    intersect_array[0] = s1;
    intersect_array[1] = s2;
    Z3_ast s3 = Z3_mk_set_intersect(c, 2, intersect_array);
    Z3_ast assert1 = Z3_mk_eq(c, s3, emp);
    Z3_solver_assert(c, s,assert1);
    check(c, s, Z3_L_TRUE);*/
}

TEST(DeductionTest, set_theory) {
    context ctx;
    solver solver(ctx);
    sort elt_type = ctx.string_sort();
    expr s1 = Util::empty_set(elt_type);
    expr s2 = Util::empty_set(elt_type);
    expr s4 = Util::empty_set(elt_type);
    expr s5 = Util::empty_set(elt_type);
    expr emp = Util::empty_set(elt_type);

    //Init sets.
    s1 = Util::set_add(s1, ctx.string_val("1"));
    s1 = Util::set_add(s1, ctx.string_val("2"));
    s2 = Util::set_add(s2, ctx.string_val("3"));
    s4 = Util::set_add(s4, ctx.string_val("2"));
    s5 = Util::set_add(s5, ctx.string_val("1"));

    //intersect
    expr s3 = Util::set_intersect(s1, s2);
    expr assert1 = (s3 == emp);
    solver.push();
    solver.add(assert1);
	EXPECT_EQ(true, solver.check() == sat);
    solver.pop();

    //subset
    expr assert_subset = Util::set_subset(s2, s1);
    solver.push();
    solver.add(assert_subset);
	EXPECT_EQ(true, solver.check() == unsat);
    solver.pop();

    expr assert_subset2 = Util::set_subset(s4, s1);
    //std::cout << "subset string: " << toString(assert_subset2) << std::endl;
    solver.push();
    solver.add(assert_subset2);
	EXPECT_EQ(true, solver.check() == sat);
    solver.pop();

    //union
    expr union_4_5 = Util::set_union(s4, s5);
    expr assert_union = (union_4_5 == s1);
    solver.push();
    solver.add(assert_union);
	EXPECT_EQ(true, solver.check() == sat);
    solver.pop();

    //diff
    expr diff_1_4 = Util::set_difference(s1, s4);
    expr assert_diff = (diff_1_4 == s5);
    solver.push();
    solver.add(assert_diff);
	EXPECT_EQ(true, solver.check() == sat);
    solver.pop();

    solver.push();
    expr assert_diff2 = (diff_1_4 == s4);
    solver.add(assert_diff2);
	EXPECT_EQ(true, solver.check() == unsat);
    solver.pop();

    //set constraints from string
    //1. strict subset constraint
    std::string subset_str = "(define-sort Set () (Array String Bool))(declare-const x (Set))(declare-const y (Set))(assert (subset x y))(assert (not (= x y)))";
    expr* subset_cst = Util::convertStrToExpr(ctx, subset_str);
	EXPECT_EQ("(and (subset x y) (not (= x y)))", toString(subset_cst));

    sort set_sort = ctx.array_sort(ctx.string_sort(), ctx.bool_sort());
    expr set_x = ctx.constant("x", set_sort);
    expr set_y = ctx.constant("y", set_sort);
    expr set_z = ctx.constant("z", set_sort);

    solver.push();
    solver.add(*subset_cst);
    solver.add(set_x == s2);
    solver.add(set_y == s1);
	EXPECT_EQ(true, solver.check() == unsat);
    solver.pop();

    solver.push();
    solver.add(*subset_cst);
    solver.add(set_x == s4);
    solver.add(set_y == s1);
	EXPECT_EQ(true, solver.check() == sat);
    solver.pop();

    solver.push();
    solver.add(*subset_cst);
    solver.add(set_x == s1);
    solver.add(set_y == s1);
	EXPECT_EQ(true, solver.check() == unsat);
    solver.pop();


    //2. equality constraint
    std::string eq_str = "(define-sort Set () (Array String Bool))(declare-const x (Set))(declare-const y (Set))(assert (= x y))";
    expr* eq_cst = Util::convertStrToExpr(ctx, eq_str);
	EXPECT_EQ("(= x y)", toString(eq_cst));
    solver.push();
    solver.add(*eq_cst);
    solver.add(set_x == diff_1_4);
    solver.add(set_y == s5);
	EXPECT_EQ(true, solver.check() == sat);
    solver.pop();

    //3. union constraint
    std::string union_str = "(define-sort Set () (Array String Bool))(declare-const x (Set))(declare-const y (Set))(declare-const z (Set))(assert (= (union x y) z))";
    expr* union_cst = Util::convertStrToExpr(ctx, union_str);
	EXPECT_EQ("(= ((as union (Array String Bool)) x y) z)", toString(union_cst));

    solver.push();
    solver.add(*union_cst);
    solver.add(set_x == s4);
    solver.add(set_y == s5);
    solver.add(set_z == s1);
	EXPECT_EQ(true, solver.check() == sat);
    solver.pop();

    solver.push();
    solver.add(*union_cst);
    solver.add(set_x == s3);
    solver.add(set_y == s5);
    solver.add(set_z == s1);
	EXPECT_EQ(true, solver.check() == unsat);
    solver.pop();

    //4. intersect constraint
    std::string intersect_str = "(define-sort Set () (Array String Bool))(declare-const x (Set))(declare-const y (Set))(assert (not (= (intersect x y) ((as const Set) false))))(assert (not (subset x y)))(assert (not (subset y x)))";
    expr* intersect_cst = Util::convertStrToExpr(ctx, intersect_str);
	EXPECT_EQ("(and (not (= (intersect x y) ((as const (Array String Bool)) false)))\n     (not (subset x y))\n     (not (subset y x)))", toString(intersect_cst));

    solver.push();
    solver.add(*intersect_cst);
    solver.add(set_x == s1);
    solver.add(set_y == Util::set_union(s4,s5));
	EXPECT_EQ(true, solver.check() == unsat);
    solver.pop();

    solver.push();
    solver.add(*intersect_cst);
    solver.add(set_x == s1);
    solver.add(set_y == Util::set_union(s4,s2));
	EXPECT_EQ(true, solver.check() == sat);
    solver.pop();


}

//###############Soundness checking: sketch of correct solution should not be rejected.
TEST(DeductionTest, benchmark1) {
    std::map<int, Component*> spec_map;
    std::string spread_loc = "../specs/L3/spread.json";

    Component* comp1 = new Component(spread_loc);
    spec_map.insert(std::make_pair(comp1->getId(), comp1));
    //Deduction Tree from Ruben.
    std::vector<int> spread_vec;
    //SketchTree tree();
    //spread
    spread_vec.push_back(1);
    spread_vec.push_back(0);
    spread_vec.push_back(-1);

    //Input/output constraints
    context c;
    expr col_input = c.int_const("COL_IN_99_0");
    expr row_input = c.int_const("ROW_IN_99_0");
    //input: 12X4
    expr row_input_cst = (row_input == 12);
    expr col_input_cst = (col_input == 4);
    expr input_cst = (col_input_cst && row_input_cst);
    //std::cout << "input_cst: " <<  input_cst << "\n";
    std::vector<expr> vec_inputs;
    vec_inputs.push_back(input_cst);
    //output: 6X4
    expr col_output = c.int_const("COL_OUT_99_0");
    expr row_output = c.int_const("ROW_OUT_99_0");
    expr row_output_cst = (row_output == 6);
    expr col_output_cst = (col_output == 4);
    expr output_cst = (col_output_cst && row_output_cst);
    //std::cout << "output_cst: " <<  output_cst << "\n";

    //do deduction
    Deduction deduct(spec_map);
    deduct.buildSystem(spread_vec, output_cst, vec_inputs);
    bool spread_res = deduct.solve();
	EXPECT_EQ(true, spread_res);
    deduct.blockSolution();
    bool block_spread_res = deduct.solve();
	EXPECT_EQ(false, block_spread_res);
}

TEST(DeductionTest, benchmark7) {
    std::map<int, Component*> spec_map;
    std::string spread_loc = "../specs/L3/spread.json";
    std::string gather_loc = "../specs/L3/gather.json";
    std::string unite_loc = "../specs/L3/unite.json";

    Component* comp1 = new Component(spread_loc);
    spec_map.insert(std::make_pair(comp1->getId(), comp1));

    Component* comp2 = new Component(gather_loc);
    spec_map.insert(std::make_pair(comp2->getId(), comp2));

    Component* comp3 = new Component(unite_loc);
    spec_map.insert(std::make_pair(comp3->getId(), comp3));

    //Deduction Tree from Ruben.
    int x[15] = {1, 5, -1, 2, -1, -1, -1, 0, -1, -1, -1, -1, -1, -1, -1};
    std::vector<int> sol_vec(std::begin(x), std::end(x));
    //Input/output constraints
    context c;
    expr col_input = c.int_const("COL_IN_99_0");
    expr row_input = c.int_const("ROW_IN_99_0");
    //input: 12X4
    expr row_input_cst = (row_input == 4);
    expr col_input_cst = (col_input == 5);
    expr input_cst = (col_input_cst && row_input_cst);
    //std::cout << "input_cst: " <<  input_cst << "\n";
    std::vector<expr> vec_inputs;
    vec_inputs.push_back(input_cst);
    //output: 6X4
    expr col_output = c.int_const("COL_OUT_99_0");
    expr row_output = c.int_const("ROW_OUT_99_0");
    expr row_output_cst = (row_output == 2);
    expr col_output_cst = (col_output == 7);
    expr output_cst = (col_output_cst && row_output_cst);
    //std::cout << "output_cst: " <<  output_cst << "\n";

    //do deduction
    Deduction deduct(spec_map);
    deduct.buildSystem(sol_vec, output_cst, vec_inputs);
    bool sol = deduct.solve();
	EXPECT_EQ(true, sol);

}

TEST(DeductionTest, benchmark1_set) {
    std::map<int, Component*> spec_map;
    std::string filter_loc = "../specs/L3/filter.json";

    Component* comp1 = new Component(filter_loc);
    spec_map.insert(std::make_pair(comp1->getId(), comp1));
    //Deduction Tree from Ruben.
    std::vector<int> spread_vec;
    //SketchTree tree();
    //Filter
    spread_vec.push_back(10);
    spread_vec.push_back(0);
    spread_vec.push_back(-1);

    context c;
    sort elt_type = c.int_sort();
    expr s1 = Util::empty_set(elt_type);
    s1 = Util::set_add(s1, c.int_val(1));
    s1 = Util::set_add(s1, c.int_val(2));
    //Input/output constraints
    expr col_input = c.int_const("COL_IN_99_0");
    expr row_input = c.int_const("ROW_IN_99_0");
    expr set_input = Util::set_expr(c, "SET_IN_99_0");
    //input: 12X4
    expr row_input_cst = (row_input == 12);
    expr col_input_cst = (col_input == 4);
    expr set_input_cst = (set_input == s1);
    expr input_cst = (col_input_cst && row_input_cst && set_input_cst);
    //std::cout << "input_cst: " <<  input_cst << "\n";
    std::vector<expr> vec_inputs;
    vec_inputs.push_back(input_cst);
    //output: 6X4
    expr col_output = c.int_const("COL_OUT_99_0");
    expr row_output = c.int_const("ROW_OUT_99_0");
    expr set_output = Util::set_expr(c, "SET_OUT_99_0");
    expr set_output_cst = (set_output == s1);
    //input: 12X4
    expr row_output_cst = (row_output == 6);
    expr col_output_cst = (col_output == 4);
    expr output_cst = (col_output_cst && row_output_cst && set_output_cst);
    //std::cout << "output_cst: " <<  output_cst << "\n";


    //do deduction
    Deduction deduct(spec_map);
    deduct.buildSystem(spread_vec, output_cst, vec_inputs);
    bool spread_res = deduct.solve();
	EXPECT_EQ(true, spread_res);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
