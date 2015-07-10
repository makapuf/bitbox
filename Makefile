TESTABLE = crappy test_chip test_engine test_kernel test_sampler test_sdio test_simplegraph test_simpletxt test_simpletxt_color test_usb 
PROJECTS = 1st_boot 2nd_boot test_video $(TESTABLE)

ALLCLEAN = $(patsubst %,%-clean,$(PROJECTS))
ALLTEST = $(patsubst %,%-test,$(TESTABLE))

.PHONY: $(PROJECTS) 
.PHONY: $(ALLCLEAN)

$(info makin $(PROJECTS))

all: $(PROJECTS) 

$(PROJECTS):
	$(info "-------------------------------------------------------")
	$(info $@)
	$(info "-------------------------------------------------------")
	$(info "")
	$(MAKE) -C $@ # can fail

allclean: $(ALLCLEAN)
alltest: $(ALLTEST)

$(ALLCLEAN): 
	$(MAKE) -C $(patsubst %-clean,%,$@) clean

$(ALLTEST):
	echo "\n\n **** Testing $(patsubst %-test,%,$@)\n";
	$(MAKE) -C $(patsubst %-test,%,$@) test
