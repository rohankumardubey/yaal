ifeq ($(TERMINAL),TERM)

CL						:= $(shell tput cr;tput dl1)
BOLD					:= $(shell tput bold || tput md)
RED						:= $(shell tput setaf 1 || tput AF 1)
RS						:= $(shell tput sgr0 || tput me)
NONL					:= "-n"

PROGRESS_INDICATOR_0=\\ 
PROGRESS_INDICATOR_1=| 
PROGRESS_INDICATOR_2=/ 
PROGRESS_INDICATOR_3=- 
NEXT_PROGRESS_INDICATOR_ONCE=PROGRESS_INDICATOR_0
NEXT_PROGRESS_INDICATOR_0=PROGRESS_INDICATOR_1
NEXT_PROGRESS_INDICATOR_1=PROGRESS_INDICATOR_2
NEXT_PROGRESS_INDICATOR_2=PROGRESS_INDICATOR_3
NEXT_PROGRESS_INDICATOR_3=PROGRESS_INDICATOR_0
CURR_PROGRESS_INDICATOR=PROGRESS_INDICATOR_0
CURR_DEP_PROGRESS_INDICATOR=PROGRESS_INDICATOR_0
DEP_PROGRESS_INDICATOR_SUFFIX= Generating dependencies ...
DEP_CL=$(CL)

define PROGRESS_INDICATOR
CURR_PROGRESS_INDICATOR:=$$(NEXT_$$(CURR_PROGRESS_INDICATOR))
CURR_DEP_PROGRESS_INDICATOR:=$$(NEXT_$$(CURR_DEP_PROGRESS_INDICATOR))
endef

else

NEWLINE=\n
PROGRESS_INDICATOR_0=.
PROGRESS_INDICATOR_ONCE=Generating dependencies 
NEXT_PROGRESS_INDICATOR_ONCE=PROGRESS_INDICATOR_0
NEXT_PROGRESS_INDICATOR_0=PROGRESS_INDICATOR_0
CURR_PROGRESS_INDICATOR=NEWLINE
CURR_DEP_PROGRESS_INDICATOR=PROGRESS_INDICATOR_ONCE

define PROGRESS_INDICATOR
CURR_DEP_PROGRESS_INDICATOR:=$$(NEXT_$$(CURR_DEP_PROGRESS_INDICATOR))
CURR_PROGRESS_INDICATOR=
endef

endif
