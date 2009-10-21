# implict pattern rule

define IDENT_HELPER
	DIR_PREFIX=$$(subst . /,./,$$(foreach IT,$$(subst /, ,$$(subst $$(DIR_ROOT),,$$(DIR_BUILD))),/..))
	HEADER=$$(addsuffix .hxx,$$(basename $(1)))
	ID="\"$(1) $$(shell $$(GITID) ./$$(DIR_PREFIX)/$$(subst $$(DIR_ROOT)/,,$(1)))\""
	TID="\"$$(HEADER) $$(if $$(wildcard $$(HEADER)),$$(shell $$(GITID) ./$$(DIR_PREFIX)/$$(subst $$(DIR_ROOT)/,,$$(HEADER))),)\""
endef

%.$(DS): %.$(SS)
	@$(call msg,echo -n "$($(CURR_DEP_PROGRESS_INDICATOR))$(DEP_PROGRESS_INDICATOR_SUFFIX)$(eval $(call PROGRESS_INDICATOR))" && ) \
	/bin/rm -f "$(@)" && \
	$(DXX) $(CXXFLAGS) -MM $(<) -MT $(@:.$(DS)=.$(OS)) -MT $(@) -MF $(@) \
	$(call msg,&& echo -n "$(DEP_CL)")

%.$(OS): %.$(SS)
	@$(eval $(call IDENT_HELPER,$(<))) $(call msg,echo -n "$($(CURR_PROGRESS_INDICATOR))$(eval $(call PROGRESS_INDICATOR))Compiling \`$(subst $(DIR_ROOT)/,,$(<))' ... " && ) \
	/bin/rm -f "$(@)" && \
	$(call invoke,$(CXX) $(CXXFLAGS) -D__ID__=$(ID) -D__TID__=$(TID) $(<) -c -o $(@) 2>&1 | tee -a make.log) && \
	test -f $(@) \
	$(call msg,&& echo $(NONL) "done.$(CL)")

