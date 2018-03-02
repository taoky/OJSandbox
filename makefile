.PHONY: all clean run

CACHES = __pycache__

PYTHON3 ?= python3

all: Backend

.PHONY: Backend

Backend:
	$(MAKE) -C Backend

run:
	$(PYTHON3) main.py

clean:
	$(MAKE) clean -C Backend
	rm -rf $(CACHES)
