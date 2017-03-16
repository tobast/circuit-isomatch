SRC_DIR = src/

all:
	make -C $(SRC_DIR)

docs:
	make -C $(SRC_DIR) docs

clean:
	make -C $(SRC_DIR) clean
