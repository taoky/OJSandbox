.PHONY: all clean

CACHES = __pycache

all: Backend

.PHONY: Backend

Backend:
	$(MAKE) -C Backend

clean:
	$(MAKE) clean -C Backend
	rm -rf $(CACHES)
