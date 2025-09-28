CC = gcc
CFLAGS = -Wall -O2
LDFLAGS = -pthread

SEQ_TARGET = log_analyzer_seq
PAR_TARGET = log_analyzer_par

SEQ_SOURCE = log_analyzer_seq.c
PAR_SOURCE = log_analyzer_par.c

all: $(SEQ_TARGET) $(PAR_TARGET)

$(SEQ_TARGET): $(SEQ_SOURCE)
	$(CC) $(CFLAGS) -o $(SEQ_TARGET) $(SEQ_SOURCE)

$(PAR_TARGET): $(PAR_SOURCE)
	$(CC) $(CFLAGS) -o $(PAR_TARGET) $(PAR_SOURCE) $(LDFLAGS)

clean:
	rm -f $(SEQ_TARGET) $(PAR_TARGET)

.PHONY: all clean