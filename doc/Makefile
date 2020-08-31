# Makefile para generar la documentación en formato PDF.

OUT = proyecto.pdf
MAIN_FILE = proyecto.tex
CONTROL_FILE = proyecto.bcf
BIB_FILE = bibliografia.bib

SRCS := $(shell find . -name '*.tex')
IMAGES := $(shell find ./images -type f)

.PHONY: all
all: cleanall doc

.PHONY: doc
doc: $(OUT)

$(OUT): $(SRCS) $(BIB_FILE) $(IMAGES)
	pdflatex $(MAIN_FILE)
	biber $(CONTROL_FILE)
	pdflatex $(MAIN_FILE)

.PHONY: clean
clean:
	-find . \( -name "*.aux" -o -name "*.log" -o -name "*.toc" -o -name "*.bbl" -o -name "*.nlg" -o -name "*.out" -o -name "*.fls" -o -name "*.fdb_latexmk" -o -name "*.bcf" -o -name "*.xml" -o -name "*.lof" -o -name "*.lot" -o -name "*.synctex.gz" -o -name "*.blg" \) -delete

.PHONY: cleanall
cleanall: clean
	-find . -name "*.pdf" -delete


