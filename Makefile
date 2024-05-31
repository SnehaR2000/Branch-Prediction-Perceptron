# Author: Jared Stark;   Created: Mon Aug 16 11:28:20 PDT 2004
# Description: Makefile for building a cbp submission.

ifeq ($(TYPE),TOURNAMENT)
	CFLAGS = -g -Wall -DTOURNAMENT
	CXXFLAGS = -g -Wall -DTOURNAMENT
endif

ifeq ($(TYPE),PERCEPTRON)
	CFLAGS = -g -Wall -DPERCEPTRON
	CXXFLAGS = -g -Wall -DPERCEPTRON
endif

objects = cbp_inst.o main.o op_state.o predictor.o tread.o

predictor : $(objects)
	$(CXX) -o $@ $(objects)

cbp_inst.o : cbp_inst.h cbp_assert.h cbp_fatal.h cond_pred.h finite_stack.h indirect_pred.h stride_pred.h value_cache.h
main.o : tread.h cbp_inst.h predictor.h op_state.h
op_state.o : op_state.h
predictor.o : predictor.h op_state.h tread.h cbp_inst.h
tread.o : tread.h cbp_inst.h op_state.h

run_traces : 
	for file in PredictorTraces/* .bz2; do \
	echo "FILE $${file%.bz2}... "; \
	./predictor $${file%.bz2}; \
	done;\
	exit 0

.PHONY : clean
clean :
	rm -f predictor $(objects)
