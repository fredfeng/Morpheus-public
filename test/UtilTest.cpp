#include "gtest/gtest.h" // Google testing framework.
#include <string>
#include <vector>
#include <set>
#include <sstream> //stringstream
#include "Util.h"

static int countSubStr(std::string org, std::string sub) {
    int count = 0;
    size_t nPos = org.find(sub, 0); // fist occurrence
    while(nPos != std::string::npos)
    {
        count++;
        nPos = org.find(sub, nPos+1);
    }
    return count;
}

static std::string vec2string(std::vector<std::string> vec) {
    std::string str = "";
    for(unsigned i = 0; i < vec.size() - 1; i++) {
        str += vec[i]; 
        str += ",";
    } 
    str += vec.back();
    return str;
}

TEST(UtilTest, countStr) {
    std::string s = " <- gather(#table,#STR0,#STR1,#Unknown)";
    size_t n = Util::countSubStr(s, "#STR");
	EXPECT_EQ(2, n);
}

TEST(UtilTest, vec2Str) {
    std::vector<std::string> vec_;
    vec_.push_back("a");
    vec_.push_back("b");
    vec_.push_back("c");
    std::string s = Util::vec2string(vec_);
	EXPECT_EQ("`a`,`b`,`c`", s);
}

TEST(UtilTest, vecEq) {
    std::vector<int> vec1;
    std::vector<int> vec2;
    std::vector<int> vec3;
    vec1.push_back(1);
    vec1.push_back(0);
    vec1.push_back(-1);
    vec2.push_back(4);
    vec2.push_back(0);
    vec2.push_back(-1);
    vec3.push_back(1);
    vec3.push_back(0);
    vec3.push_back(-1);

	EXPECT_EQ(false, Util::vecEqual(vec1,vec2));
	EXPECT_EQ(true, Util::vecEqual(vec1,vec3));
}


TEST(UtilTest, freshStr) {
    std::string str1 = Util::getColName();
    std::string str2 = Util::getColName();

	EXPECT_EQ("MORPH1", str1);
	EXPECT_EQ("MORPH2", str2);
}

TEST(UtilTest, setDiff) {
    std::set<int> s1;
    std::set<int> s2;
    std::set<int> s3;
    s1.insert(1);
    s1.insert(2);
    s1.insert(3);
    s2.insert(3);
    s2.insert(4);
    s2.insert(5);
    s3.insert(4);
    s3.insert(5);
    std::set<int> diff1 = Util::setDiff(s2,s1);
    std::set<int> diff2 = Util::setDiff(s3,s1);
	EXPECT_EQ(diff1, s3);
	EXPECT_EQ(s3, diff2);
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
