//
// Created by ghost-him on 8/12/24.
//

#include "gtest/gtest.h"
#include "../Core/Data_Stream.h"


TEST(DataStreamTest, testInit) {
    Data_Stream ds;

    int a1{1}, a2;
    bool b1{true}, b2;
    double c1{1.235}, c2;
    std::string d1 { "hello"}, d2;

    ds << a1 << b1 << c1 << d1;
    ds >> a2 >> b2 >> c2 >> d2;
    ASSERT_EQ(a1, a2);
    ASSERT_EQ(b1, b2);
    ASSERT_EQ(c1, c2);
    ASSERT_EQ(d1, d2);
}

TEST(DataStreamTest, testPlainText) {
    Data_Stream ds;
    const char* str = "hello";
    ds << str;
    std::string b;
    ds >> b;
    ASSERT_EQ(std::string(str), b);

}

TEST(DataStreamTest, testVec) {
    Data_Stream ds;
    std::vector<std::string> a1, a2;
    a1 = {"hello", "world"};
    ds << a1;
    ds >> a2;
    ASSERT_EQ(a1, a2);
}

TEST(DataStreamTest, testSet) {
    Data_Stream ds;
    std::set<std::string> a1, a2;
    a1 = {"hello", "world"};
    ds << a1;
    ds >> a2;
    ASSERT_EQ(a1, a2);
}

TEST(DataStreamTest, testList) {
    Data_Stream ds;
    std::list<std::string> a1, a2;
    a1 = {"hello", "world"};
    ds << a1;
    ds >> a2;
    ASSERT_EQ(a1, a2);
}

struct myclass : public enable_serializable {
public:
    int a;
    double b;
    bool c;
    std::string d;
    std::map<std::string, double> e;
    bool operator==(const myclass& other) const {
        if (a != other.a) return false;
        if (b != other.b) return false;
        if (c != other.c) return false;
        if (d != other.d) return false;
        if (e != other.e) return false;
        return true;
    }

    SERIALIZE(a, b, c, d, e)
};

TEST(DataStreamTest, testCustom) {
    Data_Stream ds;
    myclass a;

    a.a = 1;
    a.b = 5.3262;
    a.c = false;
    a.d = "test string";
    a.e = {{"qwe", 1.56}};

    myclass b;
    ASSERT_NE(a, b);
    ds << a;
    ds >> b;
    ASSERT_EQ(a, b);
}

TEST(DataStreamTest, testUnorderedContainer1) {
    Data_Stream ds;
    std::unordered_set<std::string> a, b;
    a.insert("hello");
    a.insert("world");
    ASSERT_NE(a, b);
    ds << a;
    ds >> b;
    ASSERT_EQ(a, b);
}

TEST(DataStreamTest, testUnorderedContainer2) {
    Data_Stream ds;
    std::unordered_map<std::string, bool> a, b;
    a["hello"] = true;
    a["world"] = false;
    ASSERT_NE(a, b);
    ds << a;
    ds >> b;
    ASSERT_EQ(a, b);
}

template<typename ...Args>
Data_Stream test(Args ... args) {
    Data_Stream ds;
    ds.write_args(args...);

    return ds;
}

TEST(DataStreamTest, testSelf) {
    Data_Stream ds = test("hello", true, false, 'c', 5);
    auto& c = ds.data();
    std::cout << c.size()<< std::endl;
    for (int i = 0; i < c.size(); i ++) {
        std::cout << c[i];
    }
}