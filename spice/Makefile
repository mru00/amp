
NGSPICE=/usr/bin/ngspice
GNUCAP=/usr/bin/gnucap

all: netlist-1.gnucap.tr netlist-1.ngspice.tr


%.ngspice.tr: %.net
	$(NGSPICE) -n -b -r $@ $< | tee $(@:.tr=.log) 
	@ ! grep 'Error' $(<:.net=.log)

%.gnucap.tr: %.net
	$(GNUCAP) -b $< | tee $(@:.tr=.log) | sed -n -e "1,10p; 1,10! w $@"
	@ ! grep 'failure' $(@)
	wc -l $@


.PHONY: view
view: all
	gwave netlist-1.gnucap.tr netlist-1.ngspice.tr

.DELETE_ON_ERROR:

.PHONY: clean
clean:
	rm netlist-1.gnucap.tr netlist-1.ngspice.tr netlist-1.gnucap.log netlist-1.ngspice.log