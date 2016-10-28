
SUBDIRS = master segmentserver control api1 api2

all:
	@for a in $(SUBDIRS); do cd $$a; $(MAKE) $@; if [ "$$?" != "0" ]; then exit 1; fi; cd ..; done; exit 0

clean:
	@for a in $(SUBDIRS); do cd $$a; $(MAKE) $@; cd ..; done; exit 0

