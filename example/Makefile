all: fluency.best_features

%.est: %.zest
	gzcat $< > $@ 

%.features: %.features.gz
	gzcat $< > $@ 

%.best: %.est
	../squeeze -f -n 100 $< > $@

%.best_features: %.best %.features
	python ../util/numbers2names/numbers2names\
		$(<:.best=.features) < $< > $@
