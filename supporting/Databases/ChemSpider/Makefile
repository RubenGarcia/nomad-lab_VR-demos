#copy the rapidjson library in this directory before compiling

all: ChemSpider2NOMADVR ChemSpiderSmiles2NOMADVR

ChemSpider2NOMADVR: ChemSpider2NOMADVR.cpp Makefile
	g++ $< -o $@

ChemSpiderSmiles2NOMADVR: ChemSpider2NOMADVR.cpp Makefile
	g++ -g -DSMILES $< -o $@

clean:
	rm -f ChemSpider2NOMADVR ChemSpiderSmiles2NOMADVR *.o
