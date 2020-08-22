#include "json.h"

#include <cstdio>
#include <stack>
#include <cstring>
#include <algorithm>

#include "user_object.h"
#include "gcheck.h"
#include "stringify.h"

namespace gcheck {

std::string Escapees() {
    std::string escapees = "\\\"";
    for(char c = 0; c < 0x20; c++) {
        escapees.push_back(c);
    }
    return escapees;
}

std::string Replacee(char val) {
    static const char* digits = "0123456789ABCDEF";

    std::string ret = "\\u0000";
    //TODO: test
    for (size_t i = 0; i < 2; ++i)
        ret[i+4] = digits[(val >> (1-i)) & 0xf];
    return ret;
}
std::vector<std::string> Replacees() {
    std::vector<std::string> replacees{"\\\\", "\\\""};
    for(char c = 0; c < 0x20; c++) {
        replacees.push_back(Replacee(c));
    }
    return replacees;
}

JSON JSON::Escape(std::string str) {

    static const std::string escapees = Escapees();
    static const std::vector<std::string> replacees = Replacees();

    size_t pos = 0;
    int index = 0;
    size_t min = std::string::npos;
    for(unsigned int i = 0; i < escapees.length(); i++) {
        size_t p = str.find(escapees[i], pos);
        if(p < min) {
            min = p;
            index = i;
        }
    }
    pos = min;
    while(pos != std::string::npos) {
        str.replace(pos, 1, replacees[index]);
        pos += replacees[index].length();

        index = 0;
        min = std::string::npos;
        for(unsigned int i = 0; i < escapees.length(); i++) {
            size_t p = str.find(escapees[i], pos);
            if(p < min) {
                min = p;
                index = i;
            }
        }
        pos = min;
    }

    // encode non-utf-8 characters
    std::stack<size_t> positions;
    for(size_t pos = 0; pos < str.length(); pos++) {
        int num = 0;
        while((str[pos] << num) & 0b10000000) num++;
        if(num == 0)
            continue;
        else if(num != 1) {
            int count = 0;
            while(count < num-1 && (str[pos+count+1] & 0b11000000) == 0b10000000) count++;
            if(count == num-1) {
                pos += count;
                continue;
            }
        }
        positions.push(pos);
    }

    str.resize(str.length()+positions.size()*5); // add space for encoding
    //TODO: test
    size_t epos = str.length()-1;
    size_t offset = positions.size()*5;
    char* cstr = str.data();
    while(!positions.empty()) {
        size_t pos = positions.top();
        auto repl = Replacee(str[pos]);

        std::memmove(cstr+offset+pos+1, cstr+pos+1, epos-offset-pos);
        offset -= 5;
        std::copy(repl.data(), repl.data()+6, cstr+offset+pos);
        epos = offset+pos-1;

        positions.pop();
    }

    JSON json;
    return json.Set(str);
}

JSON::JSON(const FunctionEntry& e) {
    std::vector<JSON> data;
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
    data.emplace_back("result", e.result);

    Set(Stringify(data, [](const JSON& a) -> std::string { return a; }, "{", ",", "}"));
}

JSON::JSON(const UserObject& o)
        : JSON(std::vector{
            std::pair("json", o.json()),
            std::pair("construct", JSON(o.construct())),
            std::pair("string", JSON(o.string())),
        }) {}

JSON::JSON(const CaseEntry& e) {
    std::string out = "{";
    out += JSON("output", e.output) + ',';
    out += JSON("output_expected", e.output_expected) + ',';
    out += JSON("arguments", e.arguments) + ',';
    out += JSON("result", e.result) + ',';
    out += JSON("input", e.input) + ',';
    out += "}";

    Set(out);
}

JSON::JSON(const TestReport& r) {

    std::string out = "{";
    if(const auto d = std::get_if<EqualsData>(&r.data)) {
        out += JSON("type", "EE") + ',';

        out += JSON("output_expected", d->output_expected.string()) + ',';
        out += JSON("output", d->output.string()) + ',';
        out += JSON("result", d->result) + ',';
        out += JSON("descriptor", d->descriptor) + ',';
    } else if(const auto d = std::get_if<TrueData>(&r.data)) {
        out += JSON("type", "ET") + ',';

        out += JSON("value", d->value) + ',';
        out += JSON("result", d->result) + ',';
        out += JSON("descriptor", d->descriptor) + ',';
    } else if(const auto d = std::get_if<FalseData>(&r.data)) {
        out += JSON("type", "EF") + ',';

        out += JSON("value", d->value) + ',';
        out += JSON("result", d->result) + ',';
        out += JSON("descriptor", d->descriptor) + ',';
    } else if(const auto d = std::get_if<CaseData>(&r.data)) {
        out += JSON("type", "TC") + ',';

        out += JSON("cases", *d) + ',';
    } else if(const auto d = std::get_if<FunctionData>(&r.data)) {
        out += JSON("type", "FC") + ',';

        out += JSON("cases", *d) + ',';
    }
    out += JSON("info", r.info_stream->str());
    out += "}";

    Set(out);
}

JSON::JSON(const TestStatus& status) {
    switch (status) {
    case TestStatus::NotStarted:
        *this = JSON("NotStarted");
        break;
    case TestStatus::Started:
        *this = JSON("Started");
        break;
    case TestStatus::Finished:
        *this = JSON("Finished");
        break;
    default:
        *this = JSON("ERROR");
        break;
    }
}

JSON::JSON(const TestData& data) {

    std::string out = "{";
    out += JSON("results", data.reports) + ',';
    out += JSON("grading_method", data.grading_method) + ',';
    out += JSON("prerequisite", data.prerequisite) + ',';
    out += JSON("format", data.output_format) + ',';
    out += JSON("points", data.points) + ',';
    out += JSON("max_points", data.max_points) + ',';
    out += JSON("stdout", data.sout) + ',';
    out += JSON("stderr", data.serr) + ',';
    out += JSON("correct", data.correct) + ',';
    out += JSON("incorrect", data.incorrect) + ',';
    out += JSON("status", data.status);
    out += "}";

    Set(out);
}

JSON::JSON(const Prerequisite& pre) {
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
    out += JSON("isfullfilled", pre.IsFulfilled());
    out += JSON("details", v2);
    out += "}";
    Set(out);
}

template JSON::JSON(const std::string& key, const int& value);
template JSON::JSON(const int& v);
template JSON::JSON(const std::string& key, const unsigned int& value);
template JSON::JSON(const unsigned int& v);
template JSON::JSON(const std::string& key, const long& value);
template JSON::JSON(const long& v);
template JSON::JSON(const std::string& key, const unsigned long& value);
template JSON::JSON(const unsigned long& v);
template JSON::JSON(const std::string& key, const double& value);
template JSON::JSON(const double& v);
template JSON::JSON(const std::string& key, const float& value);
template JSON::JSON(const float& v);
template JSON::JSON(const std::string& key, const std::string& value);
template JSON::JSON(const std::string& v);

}