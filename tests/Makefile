tests = function_test io_test prerequisite
tests_clean = $(tests:%=%-clean)

.PHONY: all clean $(tests) $(tests_clean)

all: $(tests)

$(tests):
	$(MAKE) -C $@

clean: $(tests_clean)

$(tests_clean):
	$(MAKE) -C $(subst -clean,,$@) clean