#include "test.h"

#include <algorithm>
#include <utility>

#include "redirectors.h"

namespace gcheck {

void FormatterAddTest(const std::string &suite, const std::string &test, const TestData &data);
void FormatterStartTest(const std::string &suite, const std::string &test);
void FormatterFinishTest(const std::string &suite, const std::string &test);
void FormatterFinish();

double TestInfo::default_points = 1;

TestInfo::TestInfo(std::string suite, std::string test, double points, Prerequisite prerequisite) :
	suite(suite), test(test), max_points(points), prerequisite(prerequisite) {}

TestInfo::TestInfo(std::string suite, std::string test, Prerequisite prerequisite) :
	TestInfo(suite, test, default_points, prerequisite) {}

TestInfo::TestInfo(std::string suite, std::string test, double points, std::string prerequisite) :
	TestInfo(suite, test, points, Prerequisite(suite, prerequisite)) {}

TestInfo::TestInfo(std::string suite, std::string test, std::string prerequisite) :
	TestInfo(suite, test, default_points, prerequisite) {}

Test::Test(const TestInfo &info) : data_(info.max_points, info.prerequisite), suite_(info.suite), test_(info.test) {
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

TestReport &Test::AddReport(TestReport &report) {
	auto increment_correct = [this](bool b) { b ? data_.correct++ : data_.incorrect++; };

	if (const auto d = std::get_if<EqualsData>(&report.data)) {
		increment_correct(d->result);
	} else if (const auto d = std::get_if<TrueData>(&report.data)) {
		increment_correct(d->result);
	} else if (const auto d = std::get_if<FalseData>(&report.data)) {
		increment_correct(d->result);
	} else if (const auto cases = std::get_if<CaseData>(&report.data)) {
		for (auto it = cases->begin(); it != cases->end(); it++) {
			increment_correct(it->result);
		}
	} else if (const auto cases = std::get_if<FunctionData>(&report.data)) {
		for (auto it = cases->begin(); it != cases->end(); it++) {
			increment_correct(it->result);
		}
	} else {
		// this should never be run
		throw std::exception();
	}
	data_.reports.push_back(report);

	return data_.reports[data_.reports.size() - 1];
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

Prerequisite::Prerequisite(std::string default_suite, std::string prereqs) {
	size_t pos = 0, epos = 0;
	do {
		epos = prereqs.find(' ', pos);
		std::string s = prereqs.substr(pos, epos - pos);
		if (s.length() != 0) {
			std::string suitename = default_suite, testname;
			size_t ppos = s.find('.');
			if (ppos != std::string::npos) {
				suitename = s.substr(0, ppos);
				testname = s.substr(ppos + 1);
			} else {
				testname = s;
			}
			names_.emplace_back(suitename, testname);
		}
		pos = epos + 1;
	} while (epos != std::string::npos);
}

std::vector<std::tuple<std::string, std::string, bool>> Prerequisite::GetFulfillmentData() const {
	std::vector<std::tuple<std::string, std::string, bool>> ret(names_.size());

	std::transform(names_.begin(), names_.end(), ret.begin(), [](const std::pair<std::string, std::string> &t) {
		Test *test = Test::FindTest(t.first, t.second);
		if (test)
			return std::tuple(t.first, t.second, test->IsPassed());
		else
			return std::tuple(t.first, t.second, false);
	});

	return ret;
}

bool Prerequisite::IsFulfilled() const {
	if (tests_.size() != names_.size())
		return false;

	for (auto t: tests_)
		if (!t->IsPassed())
			return false;

	return true;
}

bool Prerequisite::IsFulfilled() {
	FetchTests();

	return std::as_const(*this).IsFulfilled();
}

void Prerequisite::FetchTests() {
	if (tests_.size() == names_.size())
		return;

	tests_.clear();
	for (auto &p: names_) {
		Test *t = Test::FindTest(p.first, p.second);
		if (t)
			tests_.push_back(t);
	}
}

bool Test::do_safe_run_ = false;

bool Test::RunTests() {

	const auto &test_list = test_list_();

	for (auto it = test_list.begin(); it != test_list.end(); it++) {
		FormatterAddTest((*it)->suite_, (*it)->test_, (*it)->data_);
	}

	unsigned int counter, finished = 0;
	do {
		counter = 0;
		for (auto it = test_list.begin(); it != test_list.end(); it++) {
			if ((*it)->data_.status == NotStarted && (*it)->data_.prerequisite.IsFulfilled()) {
				(*it)->data_.status = Started;
				FormatterStartTest((*it)->suite_, (*it)->test_);
				(*it)->RunTest();
				FormatterFinishTest((*it)->suite_, (*it)->test_);
				counter++;
				finished++;
			}
		}
	} while (counter != 0);

	FormatterFinish();

	return test_list.size() == finished;
}

Test *Test::FindTest(std::string suite, std::string test) {
	std::vector<Test *> &tests = test_list_();
	for (Test *t: tests)
		if (t->suite_ == suite && t->test_ == test)
			return t;

	return nullptr;
}

} // namespace gcheck
