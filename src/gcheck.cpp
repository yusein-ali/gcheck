#include "gcheck.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>

#include "argument.h"
#include "console_writer.h"
#include "redirectors.h"
#include "shared_allocator.h"
#include "test.h"

namespace gcheck {
// TODO: For some reason linker gives undefined reference errors without this.
shared_manager asdnsadinasidnasikufbiusdbfg;

namespace {
/*
        Static class for keeping track of and logging test results.
    */
class Formatter {
	typedef std::map<std::string, const TestData *> TestMap;
	typedef std::map<std::string, JSON> TestMapJSON;

	static std::map<std::string, TestMap> suites_;
	static std::map<std::string, TestMapJSON> suites_json_;

	static double total_points_;
	static double total_max_points_;

	static std::string default_format_;

	Formatter() {}; //Disallows instantiation of this class

	static void UpdateTestJSON(const std::string &suite, const std::string &test);
	static void SaveJSON();

public:
	static bool pretty_;
	static bool json_;
	static bool do_confirm_;
	static std::string filename_;

	static void AddTest(const std::string &suite, const std::string &test, const TestData &data);
	static void StartTest(const std::string &suite, const std::string &test);
	static void FinishTest(const std::string &suite, const std::string &test);
	static void Finish();
};

double Formatter::total_points_ = 0;
double Formatter::total_max_points_ = 0;
bool Formatter::pretty_ = true;
bool Formatter::json_ = false;
bool Formatter::do_confirm_ = true;
std::string Formatter::filename_ = "report.json";
std::string Formatter::default_format_ = "horizontal";
std::map<std::string, Formatter::TestMap> Formatter::suites_;
std::map<std::string, Formatter::TestMapJSON> Formatter::suites_json_;

void Formatter::UpdateTestJSON(const std::string &suite, const std::string &test) {
	suites_json_[suite][test] = JSON(*suites_[suite][test]);
}

void Formatter::AddTest(const std::string &suite, const std::string &test, const TestData &data) {
	suites_[suite][test] = &data;
	UpdateTestJSON(suite, test);

	total_max_points_ += data.max_points;
}

void Formatter::SaveJSON() {
	std::vector<std::pair<std::string, JSON>> output;
	output.push_back({"test_results", suites_json_});
	output.push_back({"points", JSON(total_points_)});
	output.push_back({"max_points", JSON(total_max_points_)});

	std::fstream file(filename_, std::ios_base::out);

	file << JSON(output) << std::endl << std::endl;

	file.close();
}

void Formatter::Finish() {
	if (pretty_) {
		ConsoleWriter writer;
		writer.WriteSeparator();
		std::cout << "Total: ";
		writer.SetColor(total_points_ == total_max_points_ ? ConsoleWriter::Green : ConsoleWriter::Red);
		std::cout << total_points_ << " / " << total_max_points_;
		writer.SetColor(ConsoleWriter::Black);
		std::cout << std::endl;

		if (do_confirm_) {
			// Wait for user confirmation
			std::cout << std::endl << "Press enter to exit." << std::endl;
			std::cin.get();
		}
	}
}

void Formatter::StartTest(const std::string &suite, const std::string &test) {
	auto data_ptr = suites_[suite][test];
	if (!data_ptr) {
		std::cerr << "error" << std::endl; //TODO actual error processing
		return;
	}

	total_points_ += data_ptr->points;

	if (json_) {
		UpdateTestJSON(suite, test);
		SaveJSON();
	}

	if (pretty_) {
		ConsoleWriter writer;
		writer.WriteSeparator();
	}
}

void Formatter::FinishTest(const std::string &suite, const std::string &test) {
	auto data_ptr = suites_[suite][test];
	if (!data_ptr) {
		std::cerr << "error" << std::endl; //TODO actual error processing
		return;
	}

	total_points_ += data_ptr->points;

	if (json_) {
		UpdateTestJSON(suite, test);
		SaveJSON();
	}

	if (pretty_) {
		const TestData &test_data = *data_ptr;

		ConsoleWriter writer;

		writer.SetColor(test_data.points == test_data.max_points ? ConsoleWriter::Green : ConsoleWriter::Red);
		std::cout << test_data.points << " / " << test_data.max_points << "  suite: " << suite << ", test: " << test
				  << std::endl;
		writer.SetColor(ConsoleWriter::Black);

		for (auto it = test_data.reports.begin(); it != test_data.reports.end(); it++) {
			std::vector<std::vector<std::string>> cells;
			if (const auto d = std::get_if<EqualsData>(&it->data)) {
				cells.push_back({});
				auto &row = cells[cells.size() - 1];
				row.push_back(d->result ? "correct" : "incorrect");
				row.push_back(d->descriptor);
				row.push_back(d->output_expected.string());
				row.push_back(d->output.string());
				row.push_back(it->info_stream.str());

				writer.SetHeaders({"Result", "Condition", "Correct", "Output", "Info"});
			} else if (const auto d = std::get_if<TrueData>(&it->data)) {

				cells.push_back({});
				auto &row = cells[cells.size() - 1];
				row.push_back(d->result ? "correct" : "incorrect");
				row.push_back(d->descriptor);
				row.push_back(d->result ? "true" : "false");
				row.push_back(it->info_stream.str());

				writer.SetHeaders({"Result", "Condition", "Value", "Info"});
			} else if (const auto d = std::get_if<FalseData>(&it->data)) {

				cells.push_back({});
				auto &row = cells[cells.size() - 1];
				row.push_back(d->result ? "correct" : "incorrect");
				row.push_back(d->descriptor);
				row.push_back(d->result ? "true" : "false");
				row.push_back(it->info_stream.str());

				writer.SetHeaders({"Result", "Condition", "Value", "Info"});
			} else if (const auto d = std::get_if<CaseData>(&it->data)) {

				for (auto it2 = d->begin(); it2 != d->end(); it2++) {
					cells.push_back({});
					auto &row = cells[cells.size() - 1];
					auto add_if = [&row](const std::optional<UserObject> &i) {
						if (i)
							row.push_back(i->string());
						else
							row.push_back("");
					};

					add_if(it2->result ? "correct" : "incorrect");
					add_if(it2->input);
					add_if(it2->output_expected);
					add_if(it2->output);
				}
				writer.SetHeaders({"Result", "Input", "Correct", "Output"});
			} else if (const auto d = std::get_if<FunctionData>(&it->data)) {

				std::vector<std::string> headers = {"Result"};
				bool headers_filled = false;
				for (auto it2 = d->begin(); it2 != d->end(); it2++) {
					cells.push_back({});
					auto &row = cells[cells.size() - 1];
					if (it2->status == TIMEDOUT) {
						row.push_back("Timed out");
						continue;
					} else if (it2->status == ERROR) {
						row.push_back("Crashed");
						continue;
					}
					row.push_back(it2->result ? "correct" : "incorrect");
					auto add = [&headers, &row, headers_filled](const std::string &str, const std::string &header) {
						row.push_back(str);
						if (!headers_filled)
							headers.push_back(header);
					};
					auto add_if = [&add](const std::optional<UserObject> &i, const std::string &header) {
						if (i)
							add(i->string(), header);
					};
					if (it2->max_run_time) {
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
				auto &row = cells[cells.size() - 1];
				row.push_back("Error: report type None");
				break;
			}

			writer.WriteRows(cells);
		}
	}
}
} // namespace

void FormatterAddTest(const std::string &suite, const std::string &test, const TestData &data) {
	Formatter::AddTest(suite, test, data);
}

void FormatterStartTest(const std::string &suite, const std::string &test) {
	Formatter::StartTest(suite, test);
}

void FormatterFinishTest(const std::string &suite, const std::string &test) {
	Formatter::FinishTest(suite, test);
}

void FormatterFinish() {
	Formatter::Finish();
}

} // namespace gcheck

#ifndef GCHECK_NOMAIN
int main(int argc, char **argv) {
	using namespace gcheck;

	int i = 1;
	auto next_param = [&i, argv]() { return argv[i++]; };

	Formatter::pretty_ = false;
	while (i < argc) {
		auto param = next_param();
		if (param == std::string("--json"))
			Formatter::json_ = true;
		else if (param == std::string("--pretty"))
			Formatter::pretty_ = true;
		else if (param == std::string("--no-confirm"))
			Formatter::do_confirm_ = false;
		else if (param == std::string("--safe"))
			Test::do_safe_run_ = true;
		else if (param == std::string("--width"))
			ConsoleWriter::width_ = std::stoi(next_param());
		else if (strncmp(param, "--", 2) == 0)
			throw std::runtime_error(std::string("Argument not recognized: ") + param);
		else
			Formatter::filename_ = param;
	}
	if (!Formatter::pretty_ && !Formatter::json_)
		Formatter::pretty_ = true;
	if (Formatter::json_ && Formatter::filename_ == "")
		Formatter::filename_ = "report.json";

	Test::RunTests();

	return 0;
}
#endif
