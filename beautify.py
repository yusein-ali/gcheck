import json
import os
import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-o", dest='out', type=str, default="stdout")
parser.add_argument("-i", dest='input', type=str, default="report.json")
parser.add_argument("-p", dest='max_points', type=float, default=-1)
args = parser.parse_args()

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
max_points = args.max_points
templates = {}

def differences(correct, answer):
    """Returns a tuple with two lists of tuples with the difference locations and lengths"""

    n = 0
    while n < len(correct) and n < len(answer) and correct[n] == answer[n]:
        n+=1
    start = correct[:n]
    
    n = 0
    while n < len(correct) and n < len(answer) and correct[-n-1] == answer[-n-1]:
        n+=1
    if n != 0:
        end = correct[-n:]
        correct = correct[len(start):-len(end)]
        answer = answer[len(start):-len(end)]
    else:
        end = ""
        correct = correct[len(start):]
        answer = answer[len(start):]
    
    mem = [[None] * len(answer)] * len(correct)
    
    def LCS(n, m):
        nonlocal correct, answer
        if n < 0 or m < 0:
            return ""
        elif mem[n][m] is not None:
            pass
        elif correct[n] == answer[m]:
            mem[n][m] = LCS(n-1, m-1) + correct[n]
        else:
            mem[n][m] = max(LCS(n-1, m), LCS(n, m-1), key=len)
        return mem[n][m]
    
    lcs = LCS(len(correct)-1, len(answer)-1)

    correct = start + correct + end
    answer = start + answer + end
    lcs = start + lcs + end

    def diffs(string):
        nonlocal lcs
        i = 0
        i2 = 0
        diff = []
        while len(lcs) > i and len(string) > i2:
            length = 0
            while string[i2+length] != lcs[i] and len(string) > i2+length:
                length += 1
            if length != 0:
                diff.append((i2, length))
            i2 += length + 1
            i+=1
        if i2 != len(string):
            diff.append((i2, len(string)-i2))
            
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
    if len(input) > 0 and input[-1] == '\n': # html pre tags need two newlines at the end to place a newline
        input += '\n'
    if len(output) > 0 and output[-1] == '\n':
        output += '\n'
    if len(correct) > 0 and correct[-1] == '\n':
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
        if end == -1:
            break

        content = ""
        for result in results:
            content += replace_func(entries[index], result)

        hor = hor[:pos] + content + hor[end+3:]

        pos = end
        index += 1
    
    return hor

def replace(templ, results, typ, func, input_text, output_text, correct_text):
    templ = templates[format_name].replace("{{{input_header}}}", input_text)
    templ = templ.replace("{{{output_header}}}", output_text)
    templ = templ.replace("{{{correct_header}}}", correct_text)
    return get_table(templ, [res for res in results if res["type"] == typ], func)

for key, file_name in template_filenames.items():
    with open(template_location + file_name, 'r') as f:
        templates[key] = f.read()

with open(report_filename, 'r') as f:
    report_data = json.load(f)
    test_results = report_data["test_results"]
    total_max_points = report_data["max_points"]
    
if max_points == -1:
    point_multiplier = 1
else:
    point_multiplier = max_points/total_max_points

#TODO: sanitize html
stdio = ""
tests = ""
for suite_name, suite_data in test_results.items():
    for test_name, test_data in suite_data.items():
        
        if ("stdout" in test_data and test_data["stdout"] != "") or ("stderr" in test_data and test_data["stderr"] != ""):
            stdio += "Test: " + test_name + "\n"
            if ("stdout" in test_data and test_data["stdout"] != ""):
                stdio += "Stdout: " + test_data["stdout"] + "\n"
            if ("stderr" in test_data and test_data["stderr"] != ""):
                stdio += "Stderr: " + test_data["stderr"] + "\n"
            stdio += "--------------------------------------------------------------\n"
        
        format_name = default_format if 'format' not in test_data else test_data['format']
        
        def ET_func(template, result):
            """Function for replacing condition case data"""
            return replace_entries(template, result["descriptor"], result["value"], result["result"])
            
        def EF_func(template, result):
            """Function for replacing condition case data"""
            return replace_entries(template, result["descriptor"], result["value"], result["result"])
        
        def EE_func(template, result):
            """Function for replacing condition case data"""
            return replace_entries(template, result["descriptor"], result["right"], result["left"])
            
        def TC_func(template, result):
            """Function for replacing test case data"""
            content = ""
            for case in result.get("cases", []):
                content += replace_entries(template, case["input"], case["correct"], case["output"])
            return content
            
        content = replace(templates[format_name], test_data['results'], "ET", ET_func, "Input", "Output", "Correct")
        content += replace(templates[format_name], test_data['results'], "EF", EF_func, "Input", "Left (Output)", "Right (Correct)")
        content += replace(templates[format_name], test_data['results'], "EE", EE_func, "Input", "Left (Output)", "Right (Correct)")
        content += replace(templates[format_name], test_data['results'], "TC", TC_func, "Input", "Left (Output)", "Right (Correct)")
        
        testbody = templates["testbody"].replace('{{{testname}}}',test_name)
        testbody = testbody.replace('{{{points}}}', str(test_data["points"]*point_multiplier))
        testbody = testbody.replace('{{{max_points}}}', str(test_data['max_points']*point_multiplier))
        testbody = testbody.replace('{{{point_state}}}', 
            "full-points" if test_data["points"] == test_data['max_points'] else 
            "zero-points" if test_data["points"] == 0 else 
            "partial-points")
        testbody = testbody.replace('{{{testid}}}', suite_name+test_name)
        testbody = testbody.replace('{{{testcontent}}}', content)

        tests += testbody

if "ERROR" in report_data:
    stdio += report_data["ERROR"]

main = templates["main"].replace('{{{stdio}}}', stdio)
main = main.replace('{{{tests}}}', tests)

if output_filename == "stdout":
    print(main)
else:
    with open(output_filename, 'w') as f:
        f.write(main)