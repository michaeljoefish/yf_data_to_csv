CC=gcc
LINK=gcc
CFLAGS=-c -Wall

OBJS=strat_analyzer.o

strat_analyzer : strat_analyzer.o
	$(CC) $^ -o $@

strat_analyzer.o : yf_csv_to_strat.c
	$(CC) $(CFLAGS) $^ -o $@

clean :
	rm -f $(OBJS) strat_analyzer
