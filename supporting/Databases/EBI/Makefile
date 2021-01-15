all: EBI2NOMADVR EBISearch2NOMADVR

EBI2NOMADVR: EBI2NOMADVR.cpp Makefile
	g++ -g -I.. -DACCESSION $< -o $@

EBISearch2NOMADVR: EBI2NOMADVR.cpp Makefile
	g++ -I.. -g $< -o $@

clean:
	rm -f EBI2NOMADVR EBISearch2NOMADVR *.o
