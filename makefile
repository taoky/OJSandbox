.PHONY: all clean run config

CACHES = __pycache__

PYTHON3 ?= python3

all: Backend

.PHONY: Backend

Backend:
	$(MAKE) -C Backend

run:
	$(PYTHON3) main.py $(DEBUG)
	$(PYTHON3) main.py cleanup

config:
	$(PYTHON3) setup.py

clean:
	$(MAKE) clean -C Backend
	rm -rf $(CACHES)
