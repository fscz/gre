all: src

clean:
	@for lib in $$(ls -d */); do cd $${lib}; make clean; cd -; done

src:
	@for lib in $$(ls -d */); do cd $${lib}; make; cd -; done

