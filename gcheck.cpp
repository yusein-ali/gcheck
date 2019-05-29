
#include "gcheck.h"
#include <iostream>
#include <fstream>
#include <algorithm>

namespace gcheck {

double Formatter::total_points_ = 0;
double Formatter::total_max_points_ = 0;
bool Formatter::show_input_ = false;
bool Formatter::highlight_difference_ = true;
bool Formatter::pretty_ = true;
bool Formatter::swapped_ = false;
std::string Formatter::filename_ = "";
std::string Formatter::default_format_ = "horizontal";
std::streambuf* Formatter::cout_buf_ = nullptr;
std::streambuf* Formatter::cerr_buf_ = nullptr;
std::stringstream Formatter::cout_(std::ios_base::in | std::ios_base::out);
std::stringstream Formatter::cerr_(std::ios_base::in | std::ios_base::out);
std::vector<std::pair<std::string, JSON>> Formatter::tests_ = std::vector<std::pair<std::string, JSON>>();

double Test::default_points_ = 1;

void Formatter::WriteReport(bool is_finished, std::string suite, std::string test) {

    // Restores the cout and cerr buffers (check Formatter::Init())
    bool was_swapped = swapped_;
    if(swapped_) {
        std::cout.rdbuf(cout_buf_);
        std::cerr.rdbuf(cerr_buf_);
        swapped_ = false;
    }

    std::fstream file;
    auto& out = filename_ == "" ? std::cout : (file = std::fstream(filename_, std::ios_base::out));

    if(!pretty_) { //if not pretty, print out the whole json
        JSON results;
        for(auto it = tests_.begin(); it != tests_.end(); it++) {
            results.Set(it->first, it->second);
        }
        JSON wrapper;
        wrapper.Set("points", total_points_)
            .Set("max_points", total_max_points_)
            .Set("test_results", results);
        
        if(!is_finished) {
            wrapper.Set("ERROR", "Crashed before finished all the tests.");
            out << wrapper.AsString() << std::endl << std::endl;
        } else {
            out << wrapper.AsString()<< std::endl << std::endl;
        }
    } else if(!is_finished) { //if pretty and not finished, print out only current test

        auto it = std::find_if(tests_.begin(), tests_.end(), [suite](std::pair<std::string, JSON> a){ return a.first == suite; });
        
        auto it2 = std::find_if(it->second.begin(), it->second.end(), 
            [test](const std::pair<std::string, std::any>& a){ 
                const JSON* data = std::any_cast<JSON>(&a.second);
                return data != NULL && a.first == test; 
            });
        if(it2 == it->second.end()) {
            out << "error" << std::endl; //TODO actual error processing
            return;
        }

        out << "suite: " << suite << ", test: " << test << std::endl;

        JSON* test_data = std::any_cast<JSON>(&it2->second);

        out << '\t' << "max_points: " << test_data->Get<double>("max_points") << std::endl;
        out << '\t' << "format: " << test_data->Get<std::string>("format") << std::endl;
        out << '\t' << "results: " << test_data->AsString("results") << std::endl;
    } else { //if pretty and finished, print totals
        out <<  "total points: " << total_points_ << " / " << total_max_points_ << std::endl;
    }

    if(filename_ != "")
        file.close();

    if(!is_finished && was_swapped) {
        // Swaps the cout and cerr buffers to redirect them to own readable streams
        cout_buf_ = std::cout.rdbuf();
        cerr_buf_ = std::cerr.rdbuf();
        std::cout.rdbuf(cout_.rdbuf());
        std::cerr.rdbuf(cerr_.rdbuf());
        swapped_ = true;
    }
}

void Formatter::SetTestData(std::string suite, std::string test, JSON& data) {

    auto it = std::find_if(tests_.begin(), tests_.end(), [suite](std::pair<std::string, JSON> item){ return suite == item.first; });

    if(it == tests_.end()) {
        tests_.push_back({ suite, JSON() });
        it = tests_.end()-1;
    }
    
    it->second.Set(test, data);

    WriteReport(false, suite, test);
}

void Formatter::SetTotal(double points, double max_points) {
    total_max_points_ = max_points;
    total_points_ = points;
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

Test::Test(std::string suite, std::string test, double points, int priority) : points_(points), suite_(suite), test_(test), priority_(priority) {
    data_.Set("results", std::vector<JSON>());
    data_.Set("max_points", points);
    test_list_().push_back(this);
}

double Test::RunTest() {
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

    return points;
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

    auto& test_list = test_list_();
    std::sort(test_list.begin(), test_list.end(), [](const Test* a, const Test* b){ return a->priority_ >= b->priority_; });

    double total_max = 0;
    for(auto it = test_list_().begin(); it != test_list_().end(); it++) {
        total_max += (*it)->points_;
    }

    double total = 0;
    double max_so_far = 0;
    int c_priority = -1;
    for(auto it = test_list_().begin(); it != test_list_().end(); it++) {
        if(c_priority != (*it)->priority_) {
            if(max_so_far != total)
                break; // stop if last prerequisite priority level didnt gain full points
            
            c_priority = (*it)->priority_;
        }
        total += (*it)->RunTest();
        max_so_far += (*it)->points_;
        Formatter::SetTotal(total, total_max);
    }
}

}

int main(int argc, char** argv) {

    using namespace gcheck;

    for(int i = 1; i < argc; ++i) {
        if(argv[i] == std::string("--json")) Formatter::pretty_ = false;
        else Formatter::filename_ = argv[i];
    }
    if(!Formatter::pretty_ && Formatter::filename_ == "") Formatter::filename_ = "report.json";

    //Formatter::Init();

    Test::RunTests();

    Formatter::WriteReport();

    return 0;
}