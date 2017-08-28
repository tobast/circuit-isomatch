SRC_DIR = src/
TESTS_DIR = util/lang/

all:
	make -C $(SRC_DIR)

debug:
	make -C $(SRC_DIR) debug

release:
	make -C $(SRC_DIR) release

docs:
	make -C $(SRC_DIR) docs

clean:
	make -C $(SRC_DIR) clean

test: all
	make -C $(TESTS_DIR) test

tmp-velocity:
	make -C $(SRC_DIR) release
	OPTFLAGS=-O3 make -C $(TESTS_DIR) find.bin
	time for i in $$(seq 100); do $(TESTS_DIR)/find.bin \
		$(TESTS_DIR)/circ/processor.circ \
		$(TESTS_DIR)/circ/mux.circ > /dev/null ; done
