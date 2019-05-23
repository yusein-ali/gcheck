
#include "gcheck.h"
#include <iostream>
#include <fstream>
#include <algorithm>

bool Formatter::show_input_ = false;
bool Formatter::highlight_difference_ = true;
bool Formatter::html_output_ = false;
bool Formatter::swapped_ = false;
std::string Formatter::default_format_ = "horizontal";
std::streambuf* Formatter::cout_buf_ = nullptr;
std::streambuf* Formatter::cerr_buf_ = nullptr;
std::stringstream Formatter::cout_(std::ios_base::in | std::ios_base::out);
std::stringstream Formatter::cerr_(std::ios_base::in | std::ios_base::out);
std::vector<std::pair<std::string, JSON>> Formatter::tests_ = std::vector<std::pair<std::string, JSON>>();

std::vector<Test*> Test::test_list_ = std::vector<Test*>();

std::pair<std::string, std::string> GetSuiteAndTest(std::string str) {
    const char separator[] = "GCHECK_TEST";
    size_t pos = str.find(separator);

    str = str.substr(pos+sizeof(separator)/sizeof(char));
    pos = str.find(separator);
    std::pair<std::string, std::string> out;
    out.first = str.substr(0, pos-1);
    out.second = str.substr(pos+sizeof(separator)/sizeof(char));

    return out;
}

JSON& Formatter::GetTest(std::string test_class) {

    auto ids = GetSuiteAndTest(test_class);

    auto it = std::find_if(tests_.begin(), tests_.end(), [ids](std::pair<std::string, JSON> item){ return ids.first == item.first; });
    return it->second.Get<JSON>(ids.second);
}

void Formatter::WriteReport(std::string filename) {

    if(swapped_) {
        std::cout.rdbuf(cout_buf_);
        std::cerr.rdbuf(cerr_buf_);
    }

    std::ofstream file(filename);

            std::cout << "asd" << std::endl;
    JSON wrapper;
    for(auto it = tests_.begin(); it != tests_.end(); it++) {
        for(auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            double points = 0;
            JSON* data = std::any_cast<JSON>(&it2->second);
            std::string grading_method = data->Get<std::string>("grading_method");
            double max_points = data->Get<double>("max_points");
            int correct = data->Get<int>("correct");
            int incorrect = data->Get<int>("incorrect");
            if(grading_method == "partial")
                points = correct/double(correct+incorrect)*max_points;
            else if(grading_method == "binary")
                points = incorrect == 0 ? max_points : 0;
            else if(grading_method == "most")
                points = incorrect <= correct ? max_points : 0;
            else if(grading_method == "strict_most")
                points = incorrect < correct ? max_points : 0;
            else
                points = 0; //TODO error
            data->Set("points", points);
        }
        wrapper.Set(it->first, it->second);
    }
    file << wrapper.AsString();

    file.close();


    std::cout << wrapper.AsString() << std::endl;
    for(auto it = tests_.begin(); it != tests_.end(); it++) {
        std::cout << "suite: " << it->first << std::endl;
        for(auto it2 = it->second.begin(); it2 != it->second.end(); ++it2) {
            std::cout << '\t' << "test: " << it2->first << std::endl;

            JSON* test_data = std::any_cast<JSON>(&it2->second);
            if(test_data == NULL) 
                std::cout << "error" << std::endl; //TODO actual error processing

            std::cout << '\t' << '\t' << "max_points: " << test_data->Get<double>("max_points") << std::endl;
            std::cout << '\t' << '\t' << "format: " << test_data->Get<std::string>("format") << std::endl;
            std::cout << '\t' << '\t' << "results: " << test_data->AsString("results") << std::endl;
        }
    }
}

void Formatter::AddReport(JSON& data) {
    
    std::string test_class = data.Get<std::string>("test_class");
    data.Remove("test_class");
    data.Set("cout", cout_.str());
    data.Set("cerr", cerr_.str());
    cout_.str("");
    cerr_.str("");

    auto increment_correct = [test_class](JSON& json) { 
        if(json.Get<bool>("result"))
            GetTest(test_class).Get<int>("correct")++;
        else
            GetTest(test_class).Get<int>("incorrect")++;
    };

    if(data.Get<std::string>("type") == "TC") {
        std::vector<JSON>& cases = data.Get<std::vector<JSON>>("cases");
        for(auto it = cases.begin(); it != cases.end(); it++) {
            it->Set("result", it->AsString("correct") == it->AsString("output"));
            increment_correct(*it);
        }
    } else
        increment_correct(data);

    std::vector<JSON>& results =  GetTest(test_class).Get<std::vector<JSON>>("results");
    results.push_back(data);
}

void Formatter::AddTest(std::string suite, std::string test, double points) {
    auto it = std::find_if(tests_.begin(), tests_.end(),
        [suite](std::pair<std::string, JSON> entry) {
             return entry.first == suite;
        });

    if(it == tests_.end()) {
        tests_.push_back({ suite, JSON() });
        it = tests_.end()-1;
    }

    JSON innertest;
    innertest.Set("max_points", points).Set("results", std::vector<JSON>()).Set("format", default_format_)
        .Set("correct", 0).Set("incorrect", 0);
    it->second.Set(test, innertest);
}

void Formatter::Init() {

    if(swapped_) return;
    swapped_ = true;

    cout_buf_ = std::cout.rdbuf();
    cerr_buf_ = std::cerr.rdbuf();
    std::cout.rdbuf(cout_.rdbuf());
    std::cerr.rdbuf(cerr_.rdbuf());
}

void Formatter::SetGradingMethod(std::string test_class, std::string method) {
    GetTest(test_class).Set("grading_method", method);
}

void Formatter::SetFormat(std::string test_class, std::string format) {
    GetTest(test_class).Set("format", format);
}

Test::Test() {
    test_list_.push_back(this);
}

void Test::RunTests() {
    for(auto it = test_list_.begin(); it != test_list_.end(); it++) {
        (*it)->RunTest();
    }
}

int main(int argc, char** argv) {

    if(argc > 1 && std::string(argv[1]) == "html")
        Formatter::html_output_ = true;

    //Formatter::Init();

    Test::RunTests();



    if(argc > 1) {
        Formatter::WriteReport(argv[1]);
    } else {
        Formatter::WriteReport(DEFAULT_REPORT_NAME);
    }

    return 0;
}