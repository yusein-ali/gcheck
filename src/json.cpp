#include "json.h"

#include <cstdio>
#include <algorithm>

#include "user_object.h"
#include "gcheck.h"
#include "stringify.h"

namespace gcheck {

_JSON<std::allocator> _JSON<std::allocator>::Escape(std::string str) {
    _JSON json;
    return json.Set(UTF8Encode(str));
}

_JSON<std::allocator>::_JSON(const _FunctionEntry<std::allocator>& e) {
    std::vector<_JSON> data;
    auto add_if = [&data](const std::string& str, auto a) {
        if(a) data.emplace_back(str, *a);
    };
    add_if("input", e.input);
    add_if("output", e.output);
    add_if("output_expected", e.output_expected);
    add_if("error", e.error);
    add_if("error_expected", e.error_expected);
    add_if("arguments", e.arguments);
    add_if("arguments_after", e.arguments_after);
    add_if("arguments_after_expected", e.arguments_after_expected);
    add_if("return_value", e.return_value);
    add_if("return_value_expected", e.return_value_expected);
    add_if("object", e.object);
    add_if("object_after", e.object_after);
    add_if("object_after_expected", e.object_after_expected);
    data.emplace_back("run_time", e.run_time.count());
    data.emplace_back("timeout", e.timeout.count());
    data.emplace_back("timed_out", e.timed_out);
    data.emplace_back("result", e.result);

    Set(Stringify(data, [](const _JSON& a) -> std::string { return a; }, "{", ",", "}"));
}

_JSON<std::allocator>::_JSON(const _UserObject<std::allocator>& o)
        : _JSON(std::vector{
            std::pair("json", o.json()),
            std::pair("construct", _JSON(o.construct())),
            std::pair("string", _JSON(o.string())),
        }) {}

_JSON<std::allocator>::_JSON(const _CaseEntry<std::allocator>& e) {
    std::vector<_JSON> data;
    auto add_if = [&data](const std::string& str, auto a) {
        if(a) data.emplace_back(str, *a);
    };
    add_if("input", e.input);
    add_if("output", e.output);
    add_if("output_expected", e.output_expected);
    add_if("arguments", e.arguments);
    data.emplace_back("result", e.result);

    Set(Stringify(data, [](const _JSON& a) -> std::string { return a; }, "{", ",", "}"));
}

_JSON<std::allocator>::_JSON(const _TestReport<std::allocator>& r) {

    std::string out = "{";
    if(const auto d = std::get_if<EqualsData>(&r.data)) {
        out += _JSON("type", "EE") + ',';

        out += _JSON("output_expected", d->output_expected.string()) + ',';
        out += _JSON("output", d->output.string()) + ',';
        out += _JSON("result", d->result) + ',';
        out += _JSON("descriptor", d->descriptor) + ',';
    } else if(const auto d = std::get_if<TrueData>(&r.data)) {
        out += _JSON("type", "ET") + ',';

        out += _JSON("value", d->value) + ',';
        out += _JSON("result", d->result) + ',';
        out += _JSON("descriptor", d->descriptor) + ',';
    } else if(const auto d = std::get_if<FalseData>(&r.data)) {
        out += _JSON("type", "EF") + ',';

        out += _JSON("value", d->value) + ',';
        out += _JSON("result", d->result) + ',';
        out += _JSON("descriptor", d->descriptor) + ',';
    } else if(const auto d = std::get_if<CaseData>(&r.data)) {
        out += _JSON("type", "TC") + ',';

        out += _JSON("cases", *d) + ',';
    } else if(const auto d = std::get_if<FunctionData>(&r.data)) {
        out += _JSON("type", "FC") + ',';

        out += _JSON("cases", *d) + ',';
    }
    out += _JSON("info", r.info_stream.str());
    out += "}";

    Set(out);
}

_JSON<std::allocator>::_JSON(const TestStatus& status) {
    switch (status) {
    case TestStatus::NotStarted:
        *this = _JSON("NotStarted");
        break;
    case TestStatus::Started:
        *this = _JSON("Started");
        break;
    case TestStatus::Finished:
        *this = _JSON("Finished");
        break;
    default:
        *this = _JSON("ERROR");
        break;
    }
}

_JSON<std::allocator>::_JSON(const _TestData<std::allocator>& data) {

    std::string out = "{";
    out += _JSON("results", data.reports) + ',';
    out += _JSON("grading_method", data.grading_method) + ',';
    out += _JSON("prerequisite", data.prerequisite) + ',';
    out += _JSON("format", data.output_format) + ',';
    out += _JSON("points", data.points) + ',';
    out += _JSON("max_points", data.max_points) + ',';
    out += _JSON("stdout", data.sout) + ',';
    out += _JSON("stderr", data.serr) + ',';
    out += _JSON("correct", data.correct) + ',';
    out += _JSON("incorrect", data.incorrect) + ',';
    out += _JSON("status", data.status);
    out += "}";

    Set(out);
}

_JSON<std::allocator>::_JSON(const Prerequisite& pre) {
    auto v = pre.GetFullfillmentData();
    std::vector<std::tuple<std::pair<std::string, std::string>,std::pair<std::string, std::string>, std::pair<std::string, bool>>> v2(v.size());
    std::transform(v.begin(), v.end(), v2.begin(),
        [](const std::tuple<std::string, std::string, bool>& t) {
            return std::tuple(
                std::pair("suite", std::get<0>(t)),
                std::pair("test", std::get<1>(t)),
                std::pair("ispassed", std::get<2>(t))
            );
        });

    std::string out = "{";
    out += _JSON("isfullfilled", pre.IsFulfilled()) + ',';
    out += _JSON("details", v2);
    out += "}";
    Set(out);
}

template _JSON<std::allocator>::_JSON(const std::string& key, const int& value);
template _JSON<std::allocator>::_JSON(const int& v);
template _JSON<std::allocator>::_JSON(const std::string& key, const unsigned int& value);
template _JSON<std::allocator>::_JSON(const unsigned int& v);
template _JSON<std::allocator>::_JSON(const std::string& key, const long& value);
template _JSON<std::allocator>::_JSON(const long& v);
template _JSON<std::allocator>::_JSON(const std::string& key, const unsigned long& value);
template _JSON<std::allocator>::_JSON(const unsigned long& v);
template _JSON<std::allocator>::_JSON(const std::string& key, const double& value);
template _JSON<std::allocator>::_JSON(const double& v);
template _JSON<std::allocator>::_JSON(const std::string& key, const float& value);
template _JSON<std::allocator>::_JSON(const float& v);
template _JSON<std::allocator>::_JSON(const std::string& key, const std::string& value);

}