import json
import os
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-o", dest='out', type=str, default="stdout")
parser.add_argument("-i", dest='input', type=str, default="report.json")
args = parser.parse_args()

default_grading_method = "partial"
default_format = "list"

template_location = sys.path[0] + "/templates/"
template_filenames = {
    "vertical": "vertical.html", 
    "horizontal": "horizontal.html",
    "main": "main.html",
    "testbody": "testbody.html"
    }
report_filename = args.input
output_filename = args.out
templates = {}

def differences(correct, answer):
    """Returns a tuple with two lists of tuples with the difference locations and lengths"""
    def LCS(n, m):
        nonlocal correct, answer
        if n < 0 or m < 0:
            return ""
        elif correct[n] == answer[m]:
            return LCS(n-1, m-1) + correct[n]
        else:
            return max(LCS(n-1, m), LCS(n, m-1), key=len)

    lcs = LCS(len(correct)-1, len(answer)-1)

    def diffs(string):
        nonlocal lcs
        i = 0
        diff = []
        for c in lcs:
            length = 0
            while c != string[i+length]:
                length += 1
            if(length != 0):
                diff.append((i, length))
                i += length
            i += 1
            
        return diff

    return diffs(correct), diffs(answer)

def mark_differences(correct, answer):
    """Returns a tuple the differences between the two strings highlighted with html and css"""
    def mark(string, positions, start_marker, end_marker):
        offset = 0
        marker_length = len(start_marker) + len(end_marker)
        for pos, length in positions:
            string = string[:pos+offset] + start_marker + string[pos+offset:pos+offset+length] + end_marker + string[pos+offset+length:]
            offset += marker_length
        return string

    cor_diff, ans_diff = differences(correct, answer)
    return mark(correct, cor_diff, "<span style=\"background-color: red\">", "</span>"), mark(answer, ans_diff, "<span style=\"background-color: red\">", "</span>")

def replace_entries(template, input_data, correct_data, output_data):
    """Returns the template with the input, output and correct tags replaced"""
    correct, output = mark_differences(str(correct_data), str(output_data))
    if len(input_data) == 1: # no brackets when only item
        input = str(input_data[0])
    else:
        input = str(input_data)
    if input[-1] == '\n': # html pre tags need two newlines at the end to place a newline
        input += '\n'
    if output[-1] == '\n':
        output += '\n'
    if correct[-1] == '\n':
        correct += '\n'
    templ = template.replace("{{{input}}}", input)
    templ = templ.replace("{{{output}}}", output)
    templ = templ.replace("{{{correct}}}", correct)
    return templ

def get_table(template, results, replace_func):
    """Fills the template with the data in results according to replace_func"""
    if len(results) == 0:
        return ""
    hor = template
    pos = 0
    entries = []
    while True: # find and copy the templates
        pos = hor.find("<!--ENTRY", pos)
        if pos == -1:
            break
        pos = hor.find("\n", pos)
        if pos == -1:
            break
        end = hor.find("-->", pos)
        if end == -1:
            break
        end = hor.rfind("\n", 0, end)
        if end == -1:
            break
        entries.append(hor[pos+1:end])
        pos = end

    pos = 0
    index = 0
    while True: # replace each template with a series of filled ones
        pos = hor.find("<!--ENTRY")
        if pos == -1:
            break
        end = hor.find("-->", pos)

        content = ""
        for result in results:
            content += replace_func(entries[index], result)

        hor = hor[:pos] + content + hor[end+3:]

        pos = end
        index += 1
    
    return hor

for key, file_name in template_filenames.items():
    with open(template_location + file_name, 'r') as f:
        templates[key] = f.read()

with open(report_filename, 'r') as f:
    report_data = json.load(f)

tests = ""
for suite_name, suite_data in report_data.items():
    for test_name, test_data in suite_data.items():
        format_name = default_format if 'format' not in test_data else test_data['format']

        def TC_func(template, result):
            """Function for replacing test case data"""
            content = ""
            for case in result.get("cases", []):
                content += replace_entries(template, case["input"], case["correct"], case["output"])
            return content

        templ = templates[format_name].replace("{{{input_header}}}", "Input")
        templ = templ.replace("{{{output_header}}}", "Output")
        templ = templ.replace("{{{correct_header}}}", "Correct")
        content = get_table(templ, [res for res in test_data['results'] if res["type"] == "TC"], TC_func)
            
        def nTC_func(template, result):
            """Function for replacing condition case data"""
            return replace_entries(template, result["condition"], result["right"], result["left"])

        templ = templates[format_name].replace("{{{input_header}}}", "Condition")
        templ = templ.replace("{{{output_header}}}", "Right (Correct)")
        templ = templ.replace("{{{correct_header}}}", "Left (Output)")
        content += get_table(templ, [res for res in test_data['results'] if res["type"] != "TC"], nTC_func)

        testbody = templates["testbody"].replace('{{{testname}}}',test_name)
        testbody = testbody.replace('{{{points}}}', str(test_data["points"]) + "/" + str(test_data['max_points']))
        testbody = testbody.replace('{{{testid}}}', suite_name+test_name)
        testbody = testbody.replace('{{{testcontent}}}', content)

        tests += testbody

main = templates["main"].replace('{{{tests}}}', tests)

if output_filename == "stdout":
    print(main)
else:
    with open(output_filename, 'w') as f:
        f.write(main)