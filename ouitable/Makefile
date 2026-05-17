# oui.txtをダウンロードしてouitable.hを生成するMakefile
all: ouitable.h

ouitable.h: oui.txt ouitable2c.py
	python3 ouitable2c.py > ouitable.h

oui.txt:
	wget https://standards-oui.ieee.org/oui/oui.txt

.PHONY: clean
clean:
	rm -f oui.txt ouitable.h

.PHONY: test
test: ouitable.h test_oui.c
	gcc -o test_oui test_oui.c
	./test_oui
