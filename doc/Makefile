TEX=fml.tex csr.tex system.tex milkdrop.tex confslides.tex confslides_overview.tex paper_overview.tex thesis.tex

DVI=$(TEX:.tex=.dvi)
PS=$(TEX:.tex=.ps)
PDF=$(TEX:.tex=.pdf)
AUX=$(TEX:.tex=.aux)
LOG=$(TEX:.tex=.log)
SNM=$(TEX:.tex=.snm)
OUT=$(TEX:.tex=.out)
NAV=$(TEX:.tex=.nav)
TOC=$(TEX:.tex=.toc)
BBL=$(TEX:.tex=.bbl)
BLG=$(TEX:.tex=.blg)
LOF=$(TEX:.tex=.lof)

all: $(PDF)

paper_overview.dvi: paper_overview.tex paper_overview.bib
	latex paper_overview.tex
	bibtex paper_overview
	latex paper_overview.tex
	latex paper_overview.tex
	
thesis.dvi: thesis.tex thesis.bib
	latex thesis.tex
	bibtex thesis
	latex thesis.tex
	latex thesis.tex

confslides.dvi: confslides.tex
	latex confslides.tex
	latex confslides.tex

confslides_overview.dvi: confslides_overview.tex
	latex confslides_overview.tex
	latex confslides_overview.tex

milkdrop.dvi: milkdrop.tex
	latex milkdrop.tex
	latex milkdrop.tex

%.dvi: %.tex
	latex $<

%.ps: %.dvi
	dvips $<

%.pdf: %.ps
	ps2pdf $<

clean:
	rm -f $(DVI) $(PS) $(PDF) $(AUX) $(LOG) $(SNM) $(OUT) $(NAV) $(TOC) $(BBL) $(BLG) $(LOF) *~

.PHONY: clean
