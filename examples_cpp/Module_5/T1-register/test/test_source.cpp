#include <iostream>
#include <string>
#include <fstream>

#include "aircraft.hpp"
#include "boat.hpp"
#include "register.hpp"
#include "vehicle.hpp"
#include <gcheck/redirectors.h>
#include <gcheck/tests.h>

std::string empty_content = "";
std::string three_items_content =
R"(A;G_BOAC;British Airways;Concorde SST;80.8;2000
A;RU-123;Russian Federation;MiG-21;80.8;1600
B;FI123;Pekka Purjehtija;Mariella;1.8;0)";
std::string two_with_malformed_lines_content =
R"(B;SINKED;Francesco Schettino;Costa Concordia;8.2;103000
K;MAG 1C;Paul Daniels;Bentley;650;0
D;OH-LVI;Finnair;Airbus 350;67.2;900
A;OH-LVI;Finnair;Airbus 350;67.2;900
Z;asdsad;sadsad;sadsad;0;0)";

gcheck::Random<std::string> rnd_reg({"G_BOAC", "YUU-123", "RU-123", "EES-321", "FI123"});
gcheck::Random<std::string> rnd_name({"Mariella", "USS Sinking", "Boat"});
gcheck::Random<std::string> rnd_model({"MiG-21", "707", "737", "F-22"});
gcheck::Random<std::string> rnd_owner({"British Airways", "Teemu Teekkari", "Russian Federation", "Nikola", "Pekka Purjehtija"});
gcheck::Random rnd_range(0u, 1000u);
gcheck::Random<bool> rnd_inspection(std::vector<bool>{false,true});
gcheck::Random<double> rnd_double(0,1000);
gcheck::Random<unsigned int> rnd_uint(0,1000);
gcheck::Random<std::string> rnd_line({"A;G_BOAC;British Airways;Concorde SST;80.8;2000\n",
"A;RU-123;Russian Federation;MiG-21;80.8;1600\n",
"B;FI123;Pekka Purjehtija;Mariella;1.8;0\n"});

std::string UniqueName() {
    gcheck::Container<char> rnd_filename = gcheck::RandomSizeContainer(0, 20, 'a', 'z');
    std::string filename;
    do {
        std::vector v = rnd_filename.Next();
        filename = std::string(v.begin(), v.end()) + ".txt";
    } while(std::ifstream(filename).is_open());
    return filename;
}

std::string WriteFile(std::string content) {
    std::string filename = UniqueName();

    std::ofstream file(filename);
    if(!file.is_open())
        return "";

    file << content;
    file.close();

    return filename;
}
// BEGIN SERVER INTERNAL
template<char V, typename T, typename S>
std::string COut(std::string register_number, std::string owner, std::string third, T fourth, S fifth) {
    std::stringstream ss;
    ss << V << ";" << register_number << ";" << owner << ";" << third << ";" << fourth << ";" << fifth << "\n";
    return ss.str();
}
template<char type>
std::string CRead(std::string str) {
    if(str[0] != type)
        return "nullptr";
    return str;
}
// END SERVER INTERNAL
template<typename V, typename T, typename S>
std::string TPrint(std::string register_number, std::string owner, std::string third, T fourth, S fifth) {
    V vehicle(register_number, owner, third, fourth, fifth);
    gcheck::StdoutCapturer tout;
    vehicle.Print();
    return tout.str();
}
template<typename V, typename T, typename S>
std::string TWrite(std::string register_number, std::string owner, std::string third, T fourth, S fifth) {
    V vehicle(register_number, owner, third, fourth, fifth);
    std::stringstream ss;
    vehicle.Write(ss);
    return ss.str();
}
template<typename V>
std::string TRead(std::string str) {
    std::stringstream ss(str);
    V* vehicle = V::Read(ss);
    if(vehicle == nullptr)
        return "nullptr";

    ss.str("");
    ss.clear();
    vehicle->Write(ss);
    std::string asd = ss.str();
    delete(vehicle);
    return asd;
}

TEST(Boat, Print, 1) {
    CompareWithCallable(5, COut<'B', double, double>, TPrint<Boat, double, double>, rnd_reg, rnd_owner, rnd_name, rnd_double, rnd_double);
}
TEST(Boat, Write, 1) {
    CompareWithCallable(5, COut<'B', double, double>, TWrite<Boat, double, double>, rnd_reg, rnd_owner, rnd_name, rnd_double, rnd_double);
}
TEST(Boat, Read, 2, "Write") {
    CompareWithCallable(10, CRead<'B'>, TRead<Boat>, rnd_line);
}

TEST(Aircraft, Print, 1) {
    CompareWithCallable(5, COut<'A', double, unsigned int>, TPrint<Aircraft, double, unsigned int>, rnd_reg, rnd_owner, rnd_model, rnd_double, rnd_uint);
}
TEST(Aircraft, Write, 1) {
    CompareWithCallable(5, COut<'A', double, unsigned int>, TWrite<Aircraft, double, unsigned int>, rnd_reg, rnd_owner, rnd_model, rnd_double, rnd_uint);
}
TEST(Aircraft, Read, 2, "Write") {
    CompareWithCallable(10, CRead<'A'>, TRead<Aircraft>, rnd_line);
}

TEST(Register, ReadLine) {
    SetGradingMethod(gcheck::AllOrNothing);
    Register reg;

    EXPECT_EQ(0u, reg.Size()) << "None read yet";
    std::stringstream ss(two_with_malformed_lines_content);
    reg.ReadLine(ss);
    EXPECT_EQ(1u, reg.Size()) << "Read one line";
    reg.ReadLine(ss);
    EXPECT_EQ(1u, reg.Size()) << "Read a line with unknown vehicle";

    std::string next_line_in_stream;
    std::getline(ss, next_line_in_stream);
    EXPECT_EQ("D;OH-LVI;Finnair;Airbus 350;67.2;900", next_line_in_stream);

    reg.ReadLine(ss);
    EXPECT_EQ(2u, reg.Size()) << "Read one line";

    std::getline(ss, next_line_in_stream);
    EXPECT_EQ("Z;asdsad;sadsad;sadsad;0;0", next_line_in_stream);
}

TEST(Register, Load_1) {
    SetGradingMethod(gcheck::AllOrNothing);
    Register reg;

    std::string nonexistent = UniqueName();
    EXPECT_EQ(-1, reg.Load(UniqueName())) << "Loading from a nonexistent file";

    EXPECT_EQ(0u, reg.Size());

    gcheck::StdoutCapturer tout;
    reg.Print();

    std::string Print_out = tout.str();

    std::string correct = "";

    EXPECT_EQ(correct, Print_out);
}

TEST(Register, Load_2) {
    SetGradingMethod(gcheck::AllOrNothing);
    Register reg;

    std::string empty = WriteFile(empty_content);
    if(empty == "") { FAIL() << "Couldn't write test file"; }
    EXPECT_EQ(0, reg.Load(empty)) << "Loading from an empty file";
    std::remove(empty.c_str());

    EXPECT_EQ(0u, reg.Size());

    gcheck::StdoutCapturer tout;
    reg.Print();

    std::string Print_out = tout.str();

    std::string correct = "";

    EXPECT_EQ(correct, Print_out);
}

TEST(Register, Load_3) {
    SetGradingMethod(gcheck::AllOrNothing);
    Register reg;

    std::string three_items = WriteFile(three_items_content);
    if(three_items == "") { FAIL() << "Couldn't write test file"; }
    EXPECT_EQ(3, reg.Load(three_items)) << "Loading from a file with 3 items";
    std::remove(three_items.c_str());

    EXPECT_EQ(3u, reg.Size());

    gcheck::StdoutCapturer tout;
    reg.Print();

    std::string Print_out = tout.str();

    std::string correct =
        "A;G_BOAC;British Airways;Concorde SST;80.8;2000\n"
        "A;RU-123;Russian Federation;MiG-21;80.8;1600\n"
        "B;FI123;Pekka Purjehtija;Mariella;1.8;0\n";

    EXPECT_EQ(correct, Print_out);
}

TEST(Register, Load_4) {
    SetGradingMethod(gcheck::AllOrNothing);
    Register reg;

    std::string two_with_malformed_lines = WriteFile(two_with_malformed_lines_content);
    if(two_with_malformed_lines == "") { FAIL() << "Couldn't write test file"; }
    EXPECT_EQ(2, reg.Load(two_with_malformed_lines)) << "Loading from a file with 2 items and unknown vehicle types";
    std::remove(two_with_malformed_lines.c_str());

    EXPECT_EQ(2u, reg.Size());

    gcheck::StdoutCapturer tout;
    reg.Print();

    std::string Print_out = tout.str();

    std::string correct =
        "B;SINKED;Francesco Schettino;Costa Concordia;8.2;103000\n"
        "A;OH-LVI;Finnair;Airbus 350;67.2;900\n";

    EXPECT_EQ(correct, Print_out);
}

TEST(Register, Add_Save_1) {
    SetGradingMethod(gcheck::AllOrNothing);
    Register reg;

    reg.Add(new Aircraft("G_BOAC", "British Airways", "Concorde SST", 80.8, 2000));
    EXPECT_EQ(1u, reg.Size());
    reg.Add(new Aircraft("RU-123", "Russian Federation", "MiG-21", 80.8, 1600));
    reg.Add(new Boat("FI123", "Pekka Purjehtija", "Mariella", 1.8, 0));
    EXPECT_EQ(3u, reg.Size());

    std::string filename = UniqueName();
    reg.Save(filename);

    std::ifstream test(filename);
    std::stringstream ss;
    ss << test.rdbuf();
    std::string Save_out = ss.str();
    std::remove(filename.c_str());

    std::string correct =
        "A;G_BOAC;British Airways;Concorde SST;80.8;2000\n"
        "A;RU-123;Russian Federation;MiG-21;80.8;1600\n"
        "B;FI123;Pekka Purjehtija;Mariella;1.8;0\n";

    EXPECT_EQ(correct, Save_out);
}

TEST(Register, Add_Save_2) {
    SetGradingMethod(gcheck::AllOrNothing);
    Register reg;

    EXPECT_EQ(0u, reg.Size());
    reg.Add(new Boat("SINKED", "Francesco Schettino", "Costa Concordia", 8.2, 103000));
    reg.Add(new Aircraft("OH-LVI", "Finnair", "Airbus 350", 67.2, 900));
    EXPECT_EQ(2u, reg.Size());

    std::string filename = UniqueName();
    reg.Save(filename);

    std::ifstream test(filename);
    std::stringstream ss;
    ss << test.rdbuf();
    std::string Save_out = ss.str();
    std::remove(filename.c_str());

    std::string correct =
        "B;SINKED;Francesco Schettino;Costa Concordia;8.2;103000\n"
        "A;OH-LVI;Finnair;Airbus 350;67.2;900\n";

    EXPECT_EQ(correct, Save_out);
}
