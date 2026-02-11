#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <gcheck/argument.h>
#include <iostream>
#include <string>
#include <vector>

#include "geomvector.hpp"
#include <gcheck/tests.h>

gcheck::Generator rnd_vector2(
    [](size_t index) -> std::vector<std::vector<double>>
    {
        std::vector<std::vector<double>> args(2);
        gcheck::Random<double> rnd_1(0.1, 10000.0);
        gcheck::Random<double> rnd_2(100.0, 900000.0);
        gcheck::Random<double> rnd_3(1000.0, 10000.0);

        double x_1 = rnd_1.Next();
        double y_1 = rnd_2.Next();
        double z_1 = rnd_3.Next();

        double x_2 = rnd_1.Next();
        double y_2 = rnd_2.Next();
        double z_2 = rnd_3.Next();

        if (index % 2 == 0)
        {
            x_2 = x_1;
            y_2 = y_1;
            z_2 = z_1;
        }

        args[0].push_back(x_1);
        args[0].push_back(y_1);
        args[0].push_back(z_1);
        args[1].push_back(x_2);
        args[1].push_back(y_2);
        args[1].push_back(z_2);

        return args;
    });

gcheck::Generator rnd_vector(
    [](size_t index) -> std::vector<double>
    {
        (void)index;
        std::vector<double> args;
        gcheck::Random<double> rnd_1(0.1, 10000.0);
        gcheck::Random<double> rnd_2(100.0, 900000.0);
        gcheck::Random<double> rnd_3(1000.0, 10000.0);

        double x_1 = rnd_1.Next();
        double y_1 = rnd_2.Next();
        double z_1 = rnd_3.Next();

        args.push_back(x_1);
        args.push_back(y_1);
        args.push_back(z_1);

        return args;
    });

struct vector
{
    double x, y, z;
};

double length(vector a)
{
    return sqrt(pow(a.x, 2) + pow(a.y, 2) + pow(a.z, 2));
}

// BEGIN SERVER INTERNAL
bool operator==(const vector &l, const vector &r)
{
    return l.x == r.x && l.y == r.y && l.z == r.z;
}

bool operator>(const vector &l, const vector &r)
{
    return length(l) > length(r);
}

bool operator!=(const vector &l, const vector &r)
{
    return l.x != r.x || l.y != r.y || l.z != r.z;
}

bool operator<(const vector &l, const vector &r)
{
    return length(l) < length(r);
}

double Division(const std::vector<double> &arg, double div)
{
    return length(vector{arg[0] / div, arg[1] / div, arg[2] / div});
}
// END SERVER INTERNAL

bool VectorEquals(const std::vector<std::vector<double>> &args)
{
    return vector{args[0][0], args[0][1], args[0][2]} == vector{args[1][0], args[1][1], args[1][2]};
}

bool GeomVectorEquals(const std::vector<std::vector<double>> &args)
{
    return GeomVector(args[0][0], args[0][1], args[0][2]) == GeomVector(args[1][0], args[1][1], args[1][2]);
}

bool VectorNotEquals(const std::vector<std::vector<double>> &args)
{
    return vector{args[0][0], args[0][1], args[0][2]} != vector{args[1][0], args[1][1], args[1][2]};
}

bool GeomVectorNotEquals(const std::vector<std::vector<double>> &args)
{
    return GeomVector(args[0][0], args[0][1], args[0][2]) != GeomVector(args[1][0], args[1][1], args[1][2]);
}

bool VectorGreater(const std::vector<std::vector<double>> &args)
{
    return vector{args[0][0], args[0][1], args[0][2]} > vector{args[1][0], args[1][1], args[1][2]};
}

bool GeomVectorGreater(const std::vector<std::vector<double>> &args)
{
    return GeomVector(args[0][0], args[0][1], args[0][2]) > GeomVector(args[1][0], args[1][1], args[1][2]);
}

bool VectorLess(const std::vector<std::vector<double>> &args)
{
    return vector{args[0][0], args[0][1], args[0][2]} < vector{args[1][0], args[1][1], args[1][2]};
}

bool GeomVectorLess(const std::vector<std::vector<double>> &args)
{
    return GeomVector(args[0][0], args[0][1], args[0][2]) < GeomVector(args[1][0], args[1][1], args[1][2]);
}

double GeomVectorDivision(const std::vector<double> &arg, double div)
{
    return (GeomVector(arg[0], arg[1], arg[2]) / div).Length();
}

TEST(GeomVector, equals)
{
    GeomVector gv1 = GeomVector(1, 10923, 1);
    GeomVector gv2 = GeomVector(1, 10923, 1);

    ExpectEqual(true, gv1 == gv2, "Length([1, 10923, 1]) == Length([1, 10923, 1])");
}

TEST(GeomVector, equals_random)
{

    // gcheck::Random<double> rnd_div(-100.0, 100.0);

    CompareWithCallable(5, VectorEquals, GeomVectorEquals, rnd_vector2);
}

TEST(GeomVector, not_equals)
{
    GeomVector gv1 = GeomVector(1, 1, 1);
    GeomVector gv2 = GeomVector(1, 2, 1);

    ExpectEqual(true, gv1 != gv2, "Length([1, 1, 1]) != Length([1, 2, 1])");
}

TEST(GeomVector, not_equals_random)
{
    CompareWithCallable(5, VectorNotEquals, GeomVectorNotEquals, rnd_vector2);
}

TEST(GeomVector, greater)
{
    GeomVector gv1 = GeomVector(300, 1, 1);
    GeomVector gv2 = GeomVector(1, 100, 1);

    ExpectEqual(true, gv1 > gv2, "Length([300, 1, 1]) > Length([1,100,1])");
}

TEST(GeomVector, greater_random)
{
    CompareWithCallable(5, VectorGreater, GeomVectorGreater, rnd_vector2);
}

TEST(GeomVector, less)
{
    GeomVector gv1 = GeomVector(1, 93129, 1231);
    GeomVector gv2 = GeomVector(1, 93129, 1200);

    ExpectEqual(false, gv1 < gv2, "!(Length([1, 93129, 1231]) < Length([1, 93129, 1200]))");
}

TEST(GeomVector, less_random)
{
    CompareWithCallable(5, VectorLess, GeomVectorLess, rnd_vector2);
}

TEST(GeomVector, division_simple)
{
    EXPECT_EQ(true, GeomVector(0.6666666666666666, 0.6666666666666666, 0.6666666666666666) == GeomVector(1, 1, 1) / 1.5);
}

TEST(GeomVector, division_random)
{
    gcheck::Random<double> rnd_div(-10.0, 10.0);

    CompareWithCallable(5, Division, GeomVectorDivision, rnd_vector, rnd_div);
}