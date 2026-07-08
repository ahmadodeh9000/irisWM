all:
	$(MAKE) -C wm
	$(MAKE) -C bar

clean:
	$(MAKE) -C wm clean
	$(MAKE) -C bar clean

.PHONY: all clean