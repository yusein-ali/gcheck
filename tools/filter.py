#!/usr/bin/python3
import sys
import argparse
import os
import shutil
import subprocess
import regex
import json
from itertools import product
from typing import Tuple, List

from report_parser import *

parser = argparse.ArgumentParser()
parser.add_argument("-o", dest='output', type=str, default="")
parser.add_argument("-i", dest='input', type=str, default="")
parser.add_argument("-r", dest='recurse', action="store_true")
parser.add_argument("-c", dest='clear', action="store_true")
parser.add_argument("-v", dest='verbose', action="store_true")
parser.add_argument("-q", dest='quiet', action="store_true")
parser.add_argument("--keep-solution", dest='keep_solution', action="store_true")
parser.add_argument("--skip-tests", dest='skip_tests', action="store_true")
args = parser.parse_args()

cwd = os.getcwd()
input = os.path.normpath(os.path.join(cwd, args.input))
output = os.path.normpath(os.path.join(cwd, args.output))

is_file = os.path.isfile(input)
is_dir = os.path.isdir(input)

if not is_file and not is_dir:
    print("error: " + input + " is not a directory or a file.")
    sys.exit(1)

if os.path.isfile(output):
    print("error: " + output + " is a file, not a directory.")
    sys.exit(1)

if args.clear and os.path.isdir(output):
    shutil.rmtree(output)

test_source_checker = regex.compile(r"(TEST|IOTEST|FUNCTIONTEST|METHODIOTEST)\([^\n]*\)\s*{")
template_checker = regex.compile(r"<([^\";<>]|(?0))*>")
template_checker2 = regex.compile(r"<([^\";<>]|(?0))*>$")
nullfinder = regex.compile(r"null([^\w]|$)")
solution_finder = regex.compile(r"(//\s*BEGIN SOLUTION|\/\*\s*BEGIN SOLUTION\s*\*\/)((?!(//\s*BEGIN SOLUTION|\/\*\s*BEGIN SOLUTION\s*\*\/|//\s*END SOLUTION|\/\*\s*END SOLUTION\s*\*\/)).)*(//\s*END SOLUTION|\/\*\s*END SOLUTION\s*\*\/)",
    regex.DOTALL | regex.IGNORECASE)
server_internal_finder = regex.compile(r"(?://|/\*)\s*BEGIN\s+SERVER\s+INTERNAL(?:(?!(?://|/\*)\s*END\s+SERVER\s+INTERNAL).)*(?:(?://|/\*)\s*END\s+SERVER\s+INTERNAL\s*(?:\*/)?)?",
    regex.DOTALL | regex.IGNORECASE)
stub_finder = regex.compile(r"/\*\s*STUB:\s*((?:(?!\*/).)*)(?:\*/)?",
    regex.DOTALL | regex.IGNORECASE)
stub_finder2 = regex.compile(r"//\s*STUB:\s*((?:(?!\n).)*)",
    regex.DOTALL | regex.IGNORECASE)
student_finder = regex.compile(r"/\*\s*STUDENT:\s*((?:(?!\*/).)*)(?:\*/)?",
    regex.DOTALL | regex.IGNORECASE)
student_finder2 = regex.compile(r"//\s*STUDENT:\s*((?:(?!\n).)*)",
    regex.DOTALL | regex.IGNORECASE)
ignore_finder = regex.compile(r"//\s*IGNORE[^\S\r\n]*\n?")
template_remover = regex.compile(r"<.*>")
#comment_finder = regex.compile(r"(?://[^\n]*(?:\n)?)|(?:/\*(?!\*/)*(?:\*/)?)"),
#    regex.DOTALL)

class Logger:
    errors = []
    ignored = []
    num = 0

    @staticmethod
    def ignore(file):
        Logger.ignored.append(file)
        if args.verbose:
            print("\r", end="")
            print("Ignored " + file)

    @staticmethod
    def error(file, msg, was_copied):
        Logger.errors.append((file, msg, was_copied))
        if not args.quiet:
            if was_copied:
                print("ERROR", file, ":", msg, "File copied without parsing")
            else:
                print("ERROR", file, ":", msg, "File was not copied")

    @staticmethod
    def fatal(file, msg, code):
        print("filter.py encoutered a fatal error:", msg)
        print("Stopping.")
        Logger.exit(code)

    @staticmethod
    def exit(code):

        with open("filter.log", "w") as f:
            temp = ["   " + error[0] + ": " + error[1] for error in Logger.errors if not error[2]]
            f.write("Files with errors that weren't copied (" + str(len(temp)) + "):\n")
            f.write("\n".join(temp))

            temp = ["   " + error[0] + ": " + error[1] for error in Logger.errors if error[2]]
            f.write("\nFiles with errors that were copied (" + str(len(temp)) + "):\n")
            f.write("\n".join(temp))

            f.write("\nFiles that were ignored (" + str(len(Logger.ignored)) + "):\n")
            f.write("\n".join(["   " + ig for ig in Logger.ignored]))

        if not args.quiet:
            if code == 0:
                print("filter.py finished with", len(Logger.ignored), "ignored files and", len(Logger.errors),
                    "files with errors of which", len([err for err in Logger.errors if err[2]]), "was copied anyway.")
            else:
                print("filter.py exited with code", code, "with", len(Logger.ignored), "ignored files and", len(Logger.errors),
                    "files with errors of which", len([err for err in Logger.errors if err[2]]), "was copied anyway.")

        sys.exit(code)

    @staticmethod
    def nextfile(file):
        if not args.quiet:
            print(str(Logger.num), os.path.relpath(file, input), "                    ", end="\r", flush=True)
        Logger.num+=1

def is_test_source(content):

    lines = content.split("\n")

    lines2 = [line for line in lines if line.find("#include") != -1]
    lines2 = [line for line in lines2 if line.find("gcheck") != -1]

    if len(lines2) == 0:
        return False

    lines2 = [line for line in lines if test_source_checker.search(line) != None]

    return len(lines2) != 0

def CppProcessor(file, content):

    def type_arg_to_cpp(argtype, o_arg):
        arg = str(o_arg)
        def string():
            return 'std::string("' + arg + '")'
        def vector():
            return argtype+'({' + arg[1:-1] + '})'
        def listarg():
            return argtype+'({' + arg[1:-1] + '})'
        def char():
            return "'" + arg + "'"
        def long_double():
            return arg + "l"
        def floatvar():
            return arg + "f"
        def default():
            return arg
        convert = {
            r"^((std::|)string|(const\s+)?char\*)$": string,
            r"^(std::|)vector<.*>$": vector,
            r"^(std::|)list<.*>$": listarg,
            r"^char$": char,
            r"^long double$": long_double,
            r"^float$": floatvar,
            r"default": default
        }
        for pattern, recipe in convert.items():
            if regex.match(pattern, argtype) != None:
                return recipe()
        return convert["default"]()

    def get_input_output(file):
        dir = os.path.split(file)[0]
        subprocess.run(["make", "-s", "get-report"], cwd=dir)

        out = Report(os.path.join(dir, "report.json"))

        subprocess.run(["make", "-s", "clean"], cwd=dir)

        return out

    def find_space(string, start, end, is_space = True):
        for i, c in enumerate(string[start:end], start):
            if c.isspace() == is_space:
                return i
        return -1

    def rfind_space(string, start, end, is_space = True):
        for i, c in reversed(list(enumerate(string[start:end], start))):
            if c.isspace() == is_space:
                return i
        return -1

    def is_in_comment(string, position):

        pos1 = string.rfind("//", 0, position+1)
        pos2 = string.rfind("\n", 0, position)
        pos3 = string.rfind("/*", 0, position+1)
        pos4 = string.rfind("*/", 0, position+1)

        return pos1 > pos2 or pos3 > pos4

    def find_on_level(string, target, start, assume_template_error = False):
        stack = [None]
        escape = False
        for index, c in enumerate(string[start:], start):
            if escape:
                escape = False
            elif is_in_comment(string, index):
                pass
            elif c == "\\":
                escape = True
            elif stack[-1] == "'":
                if c == "'":
                    stack.pop()
            elif stack[-1] == '"':
                if c == '"':
                    stack.pop()
            elif c == "<": # tries to find out whether this is an operator or a template argument
                if template_checker.match(string, index) is not None:
                    stack.append(c)
            elif c in ["(", "{", '"', "'"]:
                stack.append(c)
            elif {"(":")", "{":"}", "<":">", None:None}[stack[-1]] == c:
                stack.pop()
            elif c in [")", "}"] and {")":"(", "}":"{"}[c] in stack:
                # a fall back in case of invalid template detection
                for i, c2 in reversed(list(enumerate(stack))):
                    if c2 == {")":"(", "}":"{"}[c]:
                        stack = stack[:i]
                        break
            elif len(stack) == 1 and c == target:
                return index
            elif assume_template_error and c == target and len([c for c in stack if c != "<"]) == 1:
                return index
            if len(stack) == 0:
                break
        if not assume_template_error:
            return find_on_level(string, target, start, True)
        return -1

    def rfind_on_level(string, target, end):
        def is_escaped(index):
            nindex = index
            while string[nindex-1] == "\\":
                nindex -= 1
            return (index-nindex) % 2 == 1

        stack = [None]
        escape = False
        for index, c in reversed(list(enumerate(string[:end]))):
            if escape:
                escape = False
            elif is_in_comment(string, index):
                pass
            elif is_escaped(index):
                escape = True
            elif stack[-1] == "'":
                if c == "'":
                    stack.pop()
            elif stack[-1] == '"':
                if c == '"':
                    stack.pop()
            elif c == ">": # tries to find out whether this is an operator or a template argument

                if template_checker2.search(string[:index+1]) is not None:
                    stack.append(c)
            elif c in [")", "}", '"', "'"]:
                stack.append(c)
            elif {")":"(", "}":"{", ">":"<", None:None}[stack[-1]] == c:
                stack.pop()
            elif len(stack) == 1 and c == target:
                return index
            if len(stack) == 0:
                break
        return -1

    def get_args(string, arg_start):
        args = []
        n_start = arg_start+1
        arg_end = find_on_level(string, ")", n_start)
        while True:
            index = find_on_level(string, ",", n_start)
            if index == -1 or index > arg_end:
                index = arg_end

            args.append((string[n_start:index].strip(), n_start, index))
            if index == arg_end:
                break
            n_start = index+1

        return (args, arg_start+1, arg_end)

    class CompareWithCallable:
        def __init__(self, case):
            self.args, self.arg_start, self.arg_end = get_args(case, case.find("("))
            self.arg_end += 1
            self.repeats = self.args[0][0]
            self.correct_func_w_template = self.args[1][0]
            self.correct_func = regex.sub(template_remover, "", self.correct_func_w_template)
            self.under_test_w_template = self.args[2][0]
            self.under_test = regex.sub(template_remover, "", self.under_test_w_template)
            self.inputs = [arg[0] for arg in self.args[3:]]
            self.content = "CompareWithCallable" + case[:self.arg_end]
    class FunctionTest:
        def __init__(self, results: Result):
            self.data = {}
            self.init("SetTimeout", len(results.cases))
            self.init("SetArguments", len(results.cases))
            self.init("SetArgumentsAfter", len(results.cases))
            self.init("SetReturn", len(results.cases))
            self.init("SetMaxRunTime", len(results.cases))
            for index, case in enumerate(results.cases):
                self.add_uo(index, "SetArguments", case.arguments)
                self.add_uo(index, "SetArgumentsAfter", case.arguments_after_expected)
                self.add_uo(index, "SetReturn", case.return_value_expected)
                self.add_uo(index, "SetMaxRunTime", case.max_run_time)
                self.add(index, "SetTimeout", case.timeout)

        def __getitem__(self, key):
            return self.data[key]

        def __setitem__(self, key, val):
            self.data[key] = val

        def init(self, type: str, num: int):
            self[type] = [None] * num

        def add_uo(self, index: int, type: str, uo: UserObject):
            self.add(index, type, uo.construct if uo is not None else None)

        def add(self, index: int, type: str, val):
            self[type][index] = str(val) if val is not None else None

        def get_content(self):
            content = "    auto index = GetRunIndex();\n"
            for key, values in self.data.items():
                noneless = [val for val in values if val is not None]
                if len(noneless) == 0:
                    continue
                mapping = []
                index = 0
                for val in values:
                    if val is not None:
                        mapping.append(str(index))
                        index = index + 1
                    else:
                        mapping.append("-1")

                content = content + "    static std::array " + key + "_mapping" + "{" + ",".join(mapping) + "};\n"
                content = content + "    static std::array " + key + "_vars" + "{" + ",".join(noneless) + "};\n"
                content = content + "    if(" + key + "_mapping[index] != -1)\n"
                content = content + "        " + key + "(" + key + "_vars[" + key + "_mapping[index]]);\n"

            return content

    class MethodTest(FunctionTest):
        def __init__(self, results: Result):
            super().__init__(results)
            self.init("SetObject", len(results.cases))
            self.init("SetObjectAfter", len(results.cases))
            self.init("SetStateComparer", len(results.cases))
            for index, case in enumerate(results.cases):
                self.add_uo(index, "SetObject", case.object)
                self.add_uo(index, "SetObjectAfter", case.object_after_expected)
                #self.add_uo(index, "SetStateComparer", case.state_comparer)

    class IOTest(FunctionTest):
        def __init__(self, results: Result):
            super().__init__(results)
            self.init("SetInput", len(results.cases))
            self.init("SetOutput", len(results.cases))
            self.init("SetError", len(results.cases))
            for index, case in enumerate(results.cases):
                self.add_uo(index, "SetInput", case.input)
                self.add_uo(index, "SetOutput", case.output_expected)
                self.add_uo(index, "SetError", case.error_expected)

    class MethodIOTest(MethodTest, IOTest):
        def __init__(self, results: Result):
            super().__init__(results)

    class Function:
        def check_for_ignore(self, string):
            string = string.rstrip()
            self.ignore = string.endswith('IGNORE')

        def __init__(self, scope_start, end):
            arg_start = rfind_on_level(content, "(", content.rfind(")", 0, scope_start))
            args, arg_start, arg_end = get_args(content, arg_start)

            self.arg_types = []
            for arg in args:
                type_end = rfind_on_level(arg[0], " ", len(arg[0]))
                if type_end == -1:
                    type_end = 0
                elif arg[0][:type_end].strip()[-1] == "=":
                    temp = arg[0][:type_end-1].strip()
                    type_end = rfind_on_level(temp, " ", len(temp))

                arg_type = arg[0][:type_end].replace("const", "").replace("&", "").strip()

                self.arg_types.append(arg_type)

            self.arg_string = content[arg_start:arg_end]
            name_end = len(content[:arg_start].rstrip())
            name_end = rfind_space(content, 0, arg_start, False) # find last nonspace
            name_start = rfind_space(content, 0, name_end, True)+1 # find last space
            self.name = content[name_start:name_end]
            ret_start = name_start
            if self.name in ["TEST","FUNCTIONTEST","IOTEST","METHODTEST","METHODIOTEST"]:
                self.test_type = self.name
                self.ret_str = ""
                args = self.arg_string.split(",")
                self.suite = args[0].strip()
                self.test = args[1].strip()

                self.check_for_ignore(content[:name_start])
            else:
                self.test_type = None
                ret_end = rfind_space(content, 0, name_start, False) # find last nonspace
                ret_start = rfind_space(content, 0, ret_end, True)+1 # find last space
                self.ret_str = content[ret_start:ret_end+1]

                self.check_for_ignore(content[:ret_start])
            self.start = ret_start
            self.scope_start = scope_start
            self.lscope_start = scope_start-ret_start
            self.end = end-1
            self.content = content[self.start:end]
            if self.istest:
                self.__populate_contents()

        @property
        def istest(self):
            return self.test_type is not None

        def __populate_contents(self):
            content = self.content
            if self.test_type == "TEST":
                parts = content.split("CompareWithCallable")
                part2 = [p[p.rfind("\n")+1:] for p in parts]
                self.parted_content = [parts[0]]
                self.test_cases = []
                for index, case in enumerate(parts[1:]):
                    if is_in_comment(parts[index], len(parts[index])-1):
                        self.parted_content.append("CompareWithCallable"+case)
                    else:
                        test_case = CompareWithCallable(case)
                        test_case.indent = part2[index] if part2[index].strip() == "" else ""
                        self.test_cases.append(test_case)
                        self.parted_content.append(test_case)
                        self.parted_content.append(case[test_case.arg_end:])

        def update_content(self, content = None):
            if content is None:
                self.content = ""
                for part in self.parted_content:
                    if type(part) is CompareWithCallable:
                        self.content += part.content
                    else:
                        self.content += part
            else:
                self.content = self.content[:self.lscope_start+1] + "\n" + content + "}"

    class Struct:
        def __init__(self, start, scope_start, end):
            self.content = content[start:end]
            string = content[start+6:scope_start]
            self.name = string.strip()
            self.start = start
            self.end = end

    class Class:
        def __init__(self, start, scope_start, end):
            self.content = content[start:end]
            string = content[start+5:scope_start]
            self.name = string.strip()
            self.start = start
            self.end = end

    def find_not_in_comment(string, substr, start):
        start = string.find(substr, start)
        while start != -1 and is_in_comment(content, start):
            start = string.find(substr, start+1)
        return start

    def get_next_scope(start):
        scope_start = find_not_in_comment(content, "{", start)
        if scope_start == -1:
            return (None, -1, -1)

        end = find_on_level(content, "}", scope_start+1)

        if end < scope_start:
            Logger.error(file, "Failed to parse.", False)
            return (None, None, None)

        s_start = content.rfind("struct", 0, scope_start)
        c_start = content.rfind("class", 0, scope_start)
        scope_s = content.rfind("{", 0, scope_start)
        scope_s = max(scope_s, content.rfind(";", 0, scope_start))

        if scope_s > s_start:
            s_start = -1
        if scope_s > c_start:
            c_start = -1

        stripped = content[:scope_start].rstrip()
        f_end = -1
        if len(stripped) == 0:
            pass
        elif stripped[-1] != ')':
            rdecl = stripped.rfind("->")
            f_arg_end = stripped.rfind(")")
            scope_s = stripped.rfind("{")
            if rdecl > f_arg_end and scope_s < f_arg_end:
                f_end = f_arg_end
        else:
            f_end = content.rfind(")", 0, scope_start)

        if c_start == -1 and s_start == -1 and f_end == -1:
            return get_next_scope(end)
        else:

            if f_end > s_start and f_end > c_start:
                obj = Function(scope_start, end+1)
            elif s_start > c_start:
                obj = Struct(s_start, scope_start, end+2)
            else:
                obj = Class(c_start, scope_start, end+2)

            return (obj, obj.start, obj.end)

    fillers = []
    top_scopes = []

    try:
        input_output = get_input_output(file)
    except Exception as e:
        Logger.error(file, "Failed to retrieve input and output data from test report. Does make rule 'get-report' exist? " + str(e), False)
        return None

    pos = 0
    while True:
        scope, start, end = get_next_scope(pos)
        if scope is None:
            if start is None:
                return None
            break
        fillers.append(content[pos:start])
        top_scopes.append(scope)
        pos = end+1
    fillers.append(content[pos:])

    remove_list = []
    test_funcs = [scope for scope in top_scopes if type(scope) is Function and scope.istest and not scope.ignore]
    for test_func in test_funcs:
        if test_func.test_type == "TEST":
            for test_case in test_func.test_cases:
                remove_list.append(test_case.correct_func)
    remove_list = [func_list[0].name for func_list in [[func for func in top_scopes if func.name == rem] for rem in remove_list] if len(func_list) != 0 and not func_list[0].ignore]
    for test_func in test_funcs:
        if test_func.test_type == "TEST":
            for test_case in test_func.test_cases:
                if test_case.under_test in remove_list:
                    Logger.error(file, "Correct function is used as under_test function. Cannot remove.", False)
                    return None

    test_pairs = [(x,y) for x, y in product(test_funcs, input_output.tests) if x.suite == y.suite and x.test == y.test]

    for func_obj, test in test_pairs:
        if func_obj.test_type == "FUNCTIONTEST":
            func_obj.update_content(FunctionTest(test.results[0]).get_content())
        elif func_obj.test_type == "IOTEST":
            func_obj.update_content(IOTest(test.results[0]).get_content())
        elif func_obj.test_type == "METHODTEST":
            func_obj.update_content(MethodTest(test.results[0]).get_content())
        elif func_obj.test_type == "METHODIOTEST":
            func_obj.update_content(MethodIOTest(test.results[0]).get_content())
        elif func_obj.test_type == "TEST":
            for index, test_case in enumerate(func_obj.test_cases):
                io = test.results[index]

                if nullfinder.search(" ".join([str(case.input.construct) for case in io.cases])) != None:
                    Logger.error(file, "Input field in gcheck report contains null fields. Cannot substitute", False)
                    return None

                trans = str.maketrans({"\\":  "\\\\",
                                    "\n":  "\\n",
                                    "\t": "\\t",
                                    "\r": "\\r",
                                    "\b": "\\b",
                                    "\f": "\\f",
                                    "\"": "\\\"",
                                    "\'": "\\\'"})

                indent = test_case.indent + "    "
                ntests = "{\n"
                ntests += indent + f"auto forwarder = [](decltype({io.cases[0].input.construct}) t) {{ return std::apply({test_case.under_test_w_template}, t); }};\n"
                if len(io.cases[0].output.construct) == 0:
                    jsons = [case.output.json for case in io.cases]
                    jsons = ['"' + json.translate(trans) + '"' if isinstance(json, str) else ("true" if json else "false") if isinstance(json, bool) else str(json) for json in jsons]
                    ntests += indent + "std::vector correct = {" + ",".join(jsons) + "};\n"
                else:
                    ntests += indent + "std::vector correct = {" + ",".join([case.output.construct for case in io.cases]) + "};\n"

                ntests += f"{indent}gcheck::SequenceArgument inputs(std::vector({{{','.join(case.input.construct for case in io.cases)}}}));\n"

                ntests += f"{indent}CompareWithAnswer({test_case.repeats},correct,forwarder,inputs);\n"
                ntests += test_case.indent + "}"

                test_case.content = ntests
            func_obj.update_content()
        else:
            raise NotImplementedError(f"Test type {func_obj.test_type} parsing not implemented")

    ncontent = "#include <array>\n#include <vector>\n"
    for filler, scope in zip(fillers, top_scopes):
        ncontent += filler
        if scope.name not in remove_list:
            ncontent += scope.content
    ncontent += fillers[-1]

    return ncontent

def replace_matching(content, regex, group = None):

    matches = regex.finditer(content)
    if group is None:
        substrings = [match.span() for match in matches]
        for subs in reversed(substrings):
            content = content[:subs[0]] + content[subs[1]:]
    else:
        substrings = [(match.span(), match.group(1)) for match in matches]
        for subs in reversed(substrings):
            content = content[:subs[0][0]] + subs[1] + content[subs[0][1]:]

    return content

def remove_solution(content):
    return replace_matching(content, solution_finder)

def remove_server_internal(content):
    return replace_matching(content, server_internal_finder)

def apply_stubs(content):
    content = replace_matching(content, stub_finder, 1)
    return replace_matching(content, stub_finder2, 1)

def apply_students(content):
    content = replace_matching(content, student_finder, 1)
    return replace_matching(content, student_finder2, 1)

def remove_ignores(content):
    return replace_matching(content, ignore_finder)

def remove(infile, outdir):

    if not os.path.isdir(outdir):
        os.makedirs(outdir)

    try:
        with open(infile) as f:
            content = f.read()
    except Exception as e:
        Logger.error(infile, str(e), True)
        shutil.copy2(infile, outdir)
        return

    if not args.keep_solution:
        content = remove_solution(content)
        content = apply_stubs(content)

    if is_test_source(content):
        if args.skip_tests:
            Logger.ignore(infile)
            return
        content = CppProcessor(infile, content)
        if content == None:
            return
        content = remove_server_internal(content)
        content = apply_students(content)
        content = remove_ignores(content)

    outfile = os.path.join(outdir, os.path.basename(infile))
    with open(outfile, "w") as f:
        f.write(content)

def add_ignore(dir, prev):
    ignore = []
    ignorepath = os.path.join(dir, ".filterignore")
    try:
        if os.path.isfile(ignorepath):
            with open(ignorepath) as f:
                ignore.extend(f.read().split("\n"))
            ignore = [ig.strip() for ig in ignore if ig.strip() != ""]
            ignore = [
                (ig[1:] if ig.startswith("\\!") or ig.startswith("\\#") or ig[0] == "!" else ig
                    , ig[0] == "!")
                    for ig in ignore if ig[0] != "#"]
            ignore = [(regex.compile(ig[0]),ig[1]) for ig in ignore]
    except Exception as e:
        Logger.fatal(ignorepath, "Failed to compile .filterignore. " + str(e), False)

    return prev + ignore

def rem_in_dir(dir, ignore):
    ignore = add_ignore(dir, ignore)

    files = os.listdir(dir)
    files = [os.path.join(dir, file) for file in files]

    for file in files:
        if os.path.isfile(file):
            Logger.nextfile(file)
            do_include = ([True] + [include for regex, include in ignore if regex.search(file) != None])[-1]
            if not do_include:
                Logger.ignore(file)
                continue
            outdir = os.path.normpath(os.path.join(output, os.path.relpath(dir, input)))
            remove(file, outdir)

    if args.recurse:
        for dir in files:
            if os.path.isdir(dir):
                rem_in_dir(dir, ignore)

if is_dir:
    ignore = [(regex.compile(ig), False) for ig in [".filterignore$", "filter.log$", "filter.py$"]]
    ignore = add_ignore(cwd, ignore)
    rem_in_dir(input, ignore)

else:
    remove(input, output)

Logger.exit(0)