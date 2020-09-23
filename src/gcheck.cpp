#include "gcheck.h"

#include <iostream>
#include <fstream>
#include <algorithm>

#include "argument.h"
#include "redirectors.h"
#include "console_writer.h"
#include "shared_allocator.h"

namespace gcheck {
// TODO: For some reason linker gives undefined reference errors without this.
shared_manager asdnsadinasidnasikufbiusdbfg;

namespace {
    /*
        Static class for keeping track of and logging test results.
    */
    class Formatter {
        typedef std::vector<std::pair<std::string, const TestData&>> TestVector;
        static std::vector<std::pair<std::string, TestVector>> suites_;

        static double total_points_;
        static double total_max_points_;

        static std::string default_format_;

        Formatter() {}; //Disallows instantiation of this class

        static TestVector* GetTestVector(const std::string& suite);
        static const TestData* GetTest(const std::string& suite, const std::string& test);
    public:
        static bool pretty_;
        static std::string filename_;

        static void AddTest(const std::string& suite, const std::string& test, const TestData& data);
        static void FinishTest(const std::string& suite, const std::string& test);
        static void Finish();
    };

    double Formatter::total_points_ = 0;
    double Formatter::total_max_points_ = 0;
    bool Formatter::pretty_ = true;
    std::string Formatter::filename_ = "";
    std::string Formatter::default_format_ = "horizontal";
    std::vector<std::pair<std::string, Formatter::TestVector>> Formatter::suites_;

    Formatter::TestVector* Formatter::GetTestVector(const std::string& suite) {
        auto it = std::find_if(suites_.begin(), suites_.end(), [&suite](const std::pair<std::string, TestVector>& item){ return suite == item.first; });
        if(it == suites_.end())
            return nullptr;

        return &it->second;
    }

    const TestData* Formatter::GetTest(const std::string& suite, const std::string& test) {
        auto tests = GetTestVector(suite);
        if(!tests)
            return nullptr;

        auto it = std::find_if(tests->begin(), tests->end(), [&test](const std::pair<std::string, TestData>& p) { return p.first == test; });
        if(it == tests->end())
            return nullptr;

        return &it->second;
    }

    void Formatter::AddTest(const std::string& suite, const std::string& test, const TestData& data) {
        auto tests = GetTestVector(suite);
        if(!tests) {
            suites_.push_back({ suite, TestVector() });
            tests = &suites_.back().second;
        }

        tests->push_back({test, data});

        total_max_points_ += data.max_points;
    }

    void Formatter::Finish() {
        std::fstream file;
        auto& out = filename_ == "" ? std::cout : (file = std::fstream(filename_, std::ios_base::out));

        if(!pretty_) { //if not pretty, print out the whole json
            std::vector<std::pair<std::string, JSON>> output;
            output.push_back({"test_results", JSON(suites_)});
            output.push_back({"points", JSON(total_points_)});
            output.push_back({"max_points", JSON(total_max_points_)});

            out << JSON(output) << std::endl << std::endl;
        } else {
            ConsoleWriter writer;
            writer.WriteSeparator();
            out << "Total: ";
            writer.SetColor(total_points_ == total_max_points_ ? ConsoleWriter::Green : ConsoleWriter::Red);
            out << total_points_ << " / " << total_max_points_;
            writer.SetColor(ConsoleWriter::Black);
            out << std::endl;

            // Wait for user confirmation
            out << std::endl << "Press enter to exit." << std::endl;
            std::cin.get();
        }
    }

    void Formatter::FinishTest(const std::string& suite, const std::string& test) {

        std::fstream file;
        auto& out = filename_ == "" ? std::cout : (file = std::fstream(filename_, std::ios_base::out));

        auto data_ptr = GetTest(suite, test);
        if(!data_ptr) {
            out << "error" << std::endl; //TODO actual error processing
            return;
        }

        total_points_ += data_ptr->points;

        if(!pretty_) { //if not pretty, print out the whole json
            std::vector<std::pair<std::string, JSON>> output;
            output.push_back({"test_results", suites_});
            output.push_back({"points", total_points_});
            output.push_back({"max_points", total_max_points_});

            out << JSON(output) << std::endl << std::endl;
        } else { //if pretty and not finished, print out only current test

            auto it = std::find_if(suites_.begin(), suites_.end(),
                    [suite](std::pair<std::string, TestVector> a){ return a.first == suite; }
                );

            auto it2 = std::find_if(it->second.begin(), it->second.end(),
                    [test](const std::pair<std::string, TestData>& a){
                        return a.first == test;
                    }
                );

            if(it2 == it->second.end()) {
                out << "error" << std::endl; //TODO actual error processing
                return;
            }

            ConsoleWriter writer;
            writer.WriteSeparator();

            const TestData& test_data = *data_ptr;

            writer.SetColor(test_data.points == test_data.max_points ? ConsoleWriter::Green : ConsoleWriter::Red);
            out << test_data.points << " / " << test_data.max_points << "  suite: " << suite << ", test: " << test << std::endl;
            writer.SetColor(ConsoleWriter::Black);

            for(auto it = test_data.reports.begin(); it != test_data.reports.end(); it++) {
                std::vector<std::vector<std::string>> cells;
                if(const auto d = std::get_if<EqualsData>(&it->data)) {
                    cells.push_back({});
                    auto& row = cells[cells.size()-1];
                    row.push_back(d->result ? "correct" : "incorrect");
                    row.push_back(d->descriptor);
                    row.push_back(d->output_expected.string());
                    row.push_back(d->output.string());
                    row.push_back(it->info_stream.str());

                    writer.SetHeaders({"Result", "Condition", "Correct", "Output", "Info"});
                } else if(const auto d = std::get_if<TrueData>(&it->data)) {

                    cells.push_back({});
                    auto& row = cells[cells.size()-1];
                    row.push_back(d->result ? "correct" : "incorrect");
                    row.push_back(d->descriptor);
                    row.push_back(d->result ? "true" : "false");
                    row.push_back(it->info_stream.str());

                    writer.SetHeaders({"Result", "Condition", "Value", "Info"});
                } else if(const auto d = std::get_if<FalseData>(&it->data)) {

                    cells.push_back({});
                    auto& row = cells[cells.size()-1];
                    row.push_back(d->result ? "correct" : "incorrect");
                    row.push_back(d->descriptor);
                    row.push_back(d->result ? "true" : "false");
                    row.push_back(it->info_stream.str());

                    writer.SetHeaders({"Result", "Condition", "Value", "Info"});
                } else if(const auto d = std::get_if<CaseData>(&it->data)) {

                    for(auto it2 = d->begin(); it2 != d->end(); it2++) {
                        cells.push_back({});
                        auto& row = cells[cells.size()-1];
                        auto add_if = [&row](const std::optional<UserObject>& i) {
                            if(i) row.push_back(i->string());
                            else row.push_back("");
                        };

                        add_if(it2->result ? "correct" : "incorrect");
                        add_if(it2->input);
                        add_if(it2->output_expected);
                        add_if(it2->output);
                    }
                    writer.SetHeaders({"Result", "Input", "Correct", "Output"});
                } else if(const auto d = std::get_if<FunctionData>(&it->data)) {

                    std::vector<std::string> headers = {"Result"};
                    bool headers_filled = false;
                    for(auto it2 = d->begin(); it2 != d->end(); it2++) {
                        cells.push_back({});
                        auto& row = cells[cells.size()-1];
                        if(it2->status == TIMEDOUT) {
                            row.push_back("Timed out");
                            continue;
                        } else if(it2->status == ERROR) {
                            row.push_back("Crashed");
                            continue;
                        }
                        row.push_back(it2->result ? "correct" : "incorrect");
                        auto add = [&headers, &row, headers_filled](const std::string& str, const std::string& header) {
                            row.push_back(str);
                            if(!headers_filled) headers.push_back(header);
                        };
                        auto add_if = [&add, &headers, &row, headers_filled](const std::optional<UserObject>& i, const std::string& header) {
                            if(i) add(i->string(), header);
                        };
                        if(it2->max_run_time) {
                            add(std::to_string(it2->max_run_time->count()), "Max Run Time");
                            add(std::to_string(it2->run_time.count()), "Run Time");
                        }
                        add_if(it2->object, "Object");
                        add_if(it2->object_after, "Object Afterwards");
                        add_if(it2->object_after_expected, "Correct Object Afterwards");
                        add_if(it2->arguments, "Arguments");
                        add_if(it2->return_value, "Return Value");
                        add_if(it2->return_value_expected, "Correct Return Value");
                        add_if(it2->input, "Standard Input");
                        add_if(it2->output, "Standard Output");
                        add_if(it2->output_expected, "Expected Output");
                        add_if(it2->error, "Standard Error");
                        add_if(it2->error_expected, "Expected Error");
                        add_if(it2->arguments_after, "Arguments Afterwards");
                        add_if(it2->arguments_after_expected, "Correct Arguments Afterwards");

                        headers_filled = true;
                    }
                    writer.SetHeaders(headers);
                } else {

                    cells.push_back({});
                    auto& row = cells[cells.size()-1];
                    row.push_back("Error: report type None");
                    break;
                }

                writer.WriteRows(cells);
            }
        }

        if(filename_ != "")
            file.close();
    }
}


Prerequisite::Prerequisite(std::string default_suite, std::string prereqs) {
    size_t pos = 0, epos = 0;
    do {
        epos = prereqs.find(' ', pos);
        std::string s = prereqs.substr(pos, epos - pos);
        if(s.length() != 0) {
            std::string suitename = default_suite, testname;
            size_t ppos = s.find('.');
            if(ppos != std::string::npos) {
                suitename = s.substr(0, ppos);
                testname = s.substr(ppos+1);
            } else {
                testname = s;
            }
            names_.emplace_back(suitename, testname);
        }
        pos = epos + 1;
    } while(epos != std::string::npos);
}

std::vector<std::tuple<std::string, std::string, bool>> Prerequisite::GetFullfillmentData() const {
    std::vector<std::tuple<std::string, std::string, bool>> ret(names_.size());

    std::transform(names_.begin(), names_.end(), ret.begin(),
        [](const std::pair<std::string, std::string>& t){
            Test* test = Test::FindTest(t.first, t.second);
            if(test)
                return std::tuple(t.first, t.second, test->IsPassed());
            else
                return std::tuple(t.first, t.second, false);
        });

    return ret;
}

bool Prerequisite::IsFulfilled() const {
    if(tests_.size() != names_.size())
        return false;

    for(auto t : tests_)
        if(!t->IsPassed())
            return false;

    return true;
}

bool Prerequisite::IsFulfilled() {
    FetchTests();

    return std::as_const(*this).IsFulfilled();
}

void Prerequisite::FetchTests() {
    if(tests_.size() == names_.size())
        return;

    tests_.clear();
    for(auto& p : names_) {
        Test* t = Test::FindTest(p.first, p.second);
        if(t) tests_.push_back(t);
    }
}


double TestInfo::default_points = 1;

bool Test::do_safe_run_ = false;

Test::Test(const TestInfo& info) : data_(info.max_points, info.prerequisite), suite_(info.suite), test_(info.test) {
    test_list_().push_back(this);
}

void Test::RunTest() {
    StdoutCapturer tout;
    StderrCapturer terr;

    ActualTest();

    tout.Restore();
    terr.Restore();
    data_.sout = tout.str();
    data_.serr = terr.str();

    data_.CalculatePoints();
}

TestReport& Test::AddReport(TestReport& report) {
    auto increment_correct = [this](bool b) {
        b ? data_.correct++ : data_.incorrect++;
    };

    if(const auto d = std::get_if<EqualsData>(&report.data)) {
        increment_correct(d->result);
    } else if(const auto d = std::get_if<TrueData>(&report.data)) {
        increment_correct(d->result);
    } else if(const auto d = std::get_if<FalseData>(&report.data)) {
        increment_correct(d->result);
    } else if(const auto cases = std::get_if<CaseData>(&report.data)) {
        for(auto it = cases->begin(); it != cases->end(); it++) {
            increment_correct(it->result);
        }
    } else if(const auto cases = std::get_if<FunctionData>(&report.data)) {
        for(auto it = cases->begin(); it != cases->end(); it++) {
            increment_correct(it->result);
        }
    } else {
        // this should never be run
        throw std::exception();
    }
    data_.reports.push_back(report);

    return data_.reports[data_.reports.size()-1];
}

void Test::SetGradingMethod(gcheck::GradingMethod method) {
    data_.grading_method = method;
}

void Test::OutputFormat(std::string format) {
    data_.output_format = format;
}

bool Test::IsPassed() const {
    return data_.status == Finished && data_.max_points == data_.points;
}

bool Test::RunTests() {

    const auto& test_list = test_list_();

    for(auto it = test_list.begin(); it != test_list.end(); it++) {
        Formatter::AddTest((*it)->suite_, (*it)->test_, (*it)->data_);
    }

    unsigned int counter, finished = 0;
    do {
        counter = 0;
        for(auto it = test_list.begin(); it != test_list.end(); it++) {
            if((*it)->data_.status != Finished && (*it)->data_.prerequisite.IsFulfilled()) {
                (*it)->data_.status = Started;
                (*it)->RunTest();
                Formatter::FinishTest((*it)->suite_, (*it)->test_);
                counter++;
                finished++;
            }
        }
    } while(counter != 0);

    Formatter::Finish();

    return test_list.size() == finished;
}

Test* Test::FindTest(std::string suite, std::string test) {
    std::vector<Test*>& tests = test_list_();
    for(Test* t : tests)
        if(t->suite_ == suite && t->test_ == test)
            return t;

    return nullptr;
}

} // gcheck

#ifndef GCHECK_NOMAIN
int main(int argc, char** argv) {
    using namespace gcheck;

    int i = 1;
    auto next_param = [&i, argv]() {
        return argv[i++];
    };

    while(i < argc) {
        auto param = next_param();
        if(param == std::string("--json")) Formatter::pretty_ = false;
        else if(param == std::string("--safe")) Test::do_safe_run_ = true;
        else if(strncmp(param, "--", 2) == 0) throw std::runtime_error(std::string("Argument not recognized: ") + param);
        else Formatter::filename_ = param;
    }
    if(!Formatter::pretty_ && Formatter::filename_ == "") Formatter::filename_ = "report.json";

    Test::RunTests();

    return 0;
}
#endif