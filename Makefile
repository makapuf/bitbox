BASE_DEMOS = demos/crappy demos/kernel demos/sdio demos/usb 
LIB_DEMOS = lib/framebuffer  lib/textmode lib/blitter/balls lib/blitter/video lib/chiptune lib/sampler lib/serial 
PROJECTS = 1st_boot 2nd_boot $(BASE_DEMOS) $(LIB_DEMOS)

ALLCLEAN = $(patsubst %,%-clean,$(PROJECTS))
ALLTEST = $(patsubst %,%-test,$(TESTABLE))

.PHONY: $(PROJECTS) 
.PHONY: $(ALLCLEAN)

$(info making $(PROJECTS))

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
