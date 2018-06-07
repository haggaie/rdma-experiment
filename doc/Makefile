all: RDMA-Experiment.html INSTALL.html

%.html: %.md markdown.css
	pandoc $< -s --css markdown.css -o $@
