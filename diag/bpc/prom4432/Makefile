NPROM =		/og3/roaque/iris/gl2/proms/nprom
FAMILY =	7951
SOURCE =	bpcd4432
SPLIT =		/og3/kurt/bin/psplit

split:
	$(SPLIT) 0 10000 $(SOURCE)

0:
	$(NPROM) -S2 -v -c$(FAMILY) < $(SOURCE).0

1:
	$(NPROM) -S2 -v -c$(FAMILY) < $(SOURCE).1

2:
	$(NPROM) -S2 -v -c$(FAMILY) < $(SOURCE).2

