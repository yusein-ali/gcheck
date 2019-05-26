# Templates <!-- omit in toc -->

This folder contains templates to be used with the beautify script.

Currently in use:
- [main.html](#mainhtml)
- [testbody.html](#testbodyhtml)
- [horizontal.html and vertical.html](#horizontalhtml-and-verticalhtml)

## main.html
The main frame that will contain everything.  

Variables:  
- `{{{tests}}}` will be replaced by the contents of the tests appended after each other.    
## testbody.html
Frame to be used for each test i.e. each test will use this once and get appended
after each other.  

Variables:
- `{{{testname}}}` replaced with the name of the test, which is specified in the test source.
- `{{{testid}}}` replaced with the id of the test, which is equal to `<suite name><test name>`
- (without the <> characters).
- `{{{points}}}` replaced with the points gained from the test.
- `{{{testcontent}}}` replaced with the test results according to the format 
specified in the test source and templates corresponding to it.
## horizontal.html and vertical.html
Frames to be used for test results. horizontal.html is used when `horizontal` format is
specified in the test source. vertical.html is used when `vertical` is specified.  

Text marked by
```
<--ENTRY
<contents>
-->
```
will be removed, the contents copied, with the `<---ENTRY` and `-->` **lines** 
removed, and appended after each other for every test case.

Variable contents will depend on whether the test case was performed using 
`TEST_CASE(...)` or `EXPECT_X(...)`, with the options listed below in this same order.

Variables (Outside `<--ENTRY` and `-->` markers):
- `{{{input_header}}}` replaced with either `Input` or `Condition`
- `{{{correct_header}}}` replaced with either `Correct` or `Left (Correct)`
- `{{{output_header}}}` replaced with either `Output` or `Right (Output)`  

Variables (Inside `<--ENTRY` and `-->` markers):
- `{{{input}}}` replaced with either the input given to a test case or the condition used
- `{{{correct}}}` replaced with either the correct output for a test case or the value of 
the left operand in condition
- `{{{output}}}` replaced with either the output of the tested code for a test case or 
the value of the right operand in condition
