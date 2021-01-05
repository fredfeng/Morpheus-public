#include "SketchTree.h"
#include "gtest/gtest.h" // Google testing framework.
#include <list>
#include <string>
#include <sstream> //stringstream

static const std::string toString(std::list<int> list) {
	std::ostringstream sstream;
	std::copy(list.begin(), list.end(),
			std::ostream_iterator<int>(sstream, " "));
	std::string str = sstream.str();
	str.resize(str.size() - 1);

	return str;
}

TEST(SketchTreeTest, toList) {
	SketchTree t1;
	t1.add(3);
	t1.add(4);
	t1.add(5);
	EXPECT_EQ("0 1 2", toString(t1.toList()));
}

TEST(SketchTreeTest, parent) {
	SketchTree t1;
	t1.add(3);
	t1.add(4);
	t1.add(5);
    EXPECT_EQ(3, t1.getRoot()->val);
	EXPECT_EQ(4, t1.getRoot()->right->val);
}

TEST(SketchTreeTest, FromVector) {
	std::vector<int> init;
	init.push_back(1);
	init.push_back(2);
	init.push_back(3);

	SketchTree t1;
	t1.initByArray(init);
	EXPECT_EQ("0 1 2", toString(t1.toList()));
	EXPECT_EQ(1, t1.getRoot()->right->parent->val);
	EXPECT_EQ(0, t1.getRoot()->id);
	EXPECT_EQ(3, t1.getRoot()->right->val);
	EXPECT_EQ(2, t1.getRoot()->right->id);
}

TEST(SketchTreeTest, BFS) {
	std::vector<int> init;
	init.push_back(1);
	init.push_back(2);
	init.push_back(3);

	SketchTree t1;
	t1.initByArray(init);
	EXPECT_EQ("1 2 3", t1.traversal(TraversalType::BREADTHFIRST));
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
