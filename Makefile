PROJECTS = 1st_boot 2nd_boot crappy hwchip test_engine test_kernel test_sampler test_sdio test_simplegraph test_simpletxt test_simpletxt_color test_usb test_video

ALLCLEAN = $(patsubst %,%-clean,$(PROJECTS))

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
	
$(ALLCLEAN): 
	$(MAKE) -C $(patsubst %-clean,%,$@) clean

test_allemu: $(PROJECTS) 
	for prg in $(PROJECTS);do echo "\n\n **** Testing $$prg\n\n"; cd $$prg && ./*_emu --quiet; cd .. ; done
