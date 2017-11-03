EXAMPLES = crappy kernel sdio usb \
	framebuffer textmode blitter_balls blitter_video \
	chiptune mod sampler \
	serial events
PROJECTS = 1st_boot 2nd_boot $(EXAMPLES:%=examples/%) 

ALLCLEAN = $(patsubst %,%-clean,$(PROJECTS))
ALLTEST = $(patsubst %,%-test,$(TESTABLE))

export BITBOX=$(PWD)/sdk

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

clean: $(ALLCLEAN)
test: $(ALLTEST)

$(ALLCLEAN): 
	$(MAKE) -C $(patsubst %-clean,%,$@) clean

$(ALLTEST):
	echo "\n\n **** Testing $(patsubst %-test,%,$@)\n";
	$(MAKE) -C $(patsubst %-test,%,$@) test
