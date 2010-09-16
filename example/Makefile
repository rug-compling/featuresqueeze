all: fluency.best_features

%.est: %.zest
	gzcat $< > $@ 

%.features: %.features.gz
	gzcat $< > $@ 

%.best: %.est
	../squeeze -f -l 50 -n 100 $< > $@

%.best_features: %.best %.features ../libfsqueeze.dylib ../squeeze
	python ../util/numbers2names/numbers2names\
		$(<:.best=.features) < $< > $@
