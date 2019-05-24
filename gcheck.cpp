
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
double Test::default_points_ = 1;

void Formatter::WriteReport(std::string filename) {

    // Restores the cout and cerr buffers (check Formatter::Init())
    if(swapped_) {
        std::cout.rdbuf(cout_buf_);
        std::cerr.rdbuf(cerr_buf_);
    }

    std::ofstream file(filename);

    JSON wrapper;
    for(auto it = tests_.begin(); it != tests_.end(); it++) {
        wrapper.Set(it->first, it->second);
    }
    file << wrapper.AsString();

    file.close();

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

void Formatter::SetTestData(std::string suite, std::string test, JSON& data) {

    auto it = std::find_if(tests_.begin(), tests_.end(), [suite](std::pair<std::string, JSON> item){ return suite == item.first; });

    if(it == tests_.end()) {
        tests_.push_back({ suite, JSON() });
        it = tests_.end()-1;
    }
    
    it->second.Set(test, data);
}

void Formatter::Init() {

    if(swapped_) return;
    swapped_ = true;

    // Swaps the cout and cerr buffers to redirect them to own readable streams
    cout_buf_ = std::cout.rdbuf();
    cerr_buf_ = std::cerr.rdbuf();
    std::cout.rdbuf(cout_.rdbuf());
    std::cerr.rdbuf(cerr_.rdbuf());
}

Test::Test(std::string suite, std::string test, double points) : points_(points), suite_(suite), test_(test) {
    data_.Set("results", std::vector<JSON>());
    data_.Set("max_points", points);
    test_list_.push_back(this);
}

void Test::RunTest() {
    ActualTest();
    
    data_.Set("grading_method", grading_method_);
    data_.Set("format", output_format_);
    
    double points = 0;
    if(grading_method_ == "partial")
        points = correct_/double(correct_+incorrect_)*points_;
    else if(grading_method_ == "binary")
        points = incorrect_ == 0 ? points_ : 0;
    else if(grading_method_ == "most")
        points = incorrect_ <= correct_ ? points_ : 0;
    else if(grading_method_ == "strict_most")
        points = incorrect_ < correct_ ? points_ : 0;
    else
        points = 0;
    data_.Set("points", points);

    Formatter::SetTestData(suite_, test_, data_);
}

void Test::AddReport(JSON data) {
    
    auto increment_correct = [this](JSON& json) { 
        if(json.Get<bool>("result"))
            correct_++;
        else
            incorrect_++;
    };

    if(data.Get<std::string>("type") == "TC") {
        std::vector<JSON>& cases = data.Get<std::vector<JSON>>("cases");
        for(auto it = cases.begin(); it != cases.end(); it++) {
            it->Set("result", it->AsString("correct") == it->AsString("output"));
            increment_correct(*it);
        }
    } else
        increment_correct(data);

    std::vector<JSON>& results =  data_.Get<std::vector<JSON>>("results");
    results.push_back(data);
}

void Test::GradingMethod(std::string method) {
    grading_method_ = method; 
}

void Test::OutputFormat(std::string format) { 
    output_format_ = format; 
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