.PHONY: all release_sgp30 release_sgpc3

all: sgp30 sgpc3

sgp30/sgp30.o: sgp30/sgp30.h sgp30/sgp30.c sgp30/sgp30_featureset.c
	cd sgp30 && make

sgpc3/sgpc3.o: sgpc3/sgpc3.h sgpc3/sgpc3.c sgpc3/sgpc3_featureset.c
	cd sgpc3 && make

sgp30: sgp30/sgp30.o
sgpc3: sgpc3/sgpc3.o


release/sgp30:
	export driver=sgp30 && \
	export tag="$$(git describe --always --dirty)" && \
	export pkgname="$${driver}-$${tag}" && \
	export pkgdir="release/$${pkgname}" && \
	rm -rf "$${pkgdir}" && mkdir -p "$${pkgdir}" && \
	cp -rl embedded-common/* "$${pkgdir}" && \
	cp -rl sgp-common/* "$${pkgdir}" && \
	cp -rl $${driver}/* "$${pkgdir}" && \
	sed -i 's/^sensirion_common_dir :=.*$$/sensirion_common_dir := ./' "$${pkgdir}/Makefile" && \
	sed -i 's/^sgp_common_dir :=.*$$/sgp_common_dir := ./' "$${pkgdir}/Makefile" && \
	sed -i "s/^\(#define\s\+SGP_DRV_VERSION_STR\s*\)\".*\"$$/\1\"$${tag}\"/" "$${pkgdir}/sgp30.c" && \
	cd "$${pkgdir}" && make && make clean && cd - && \
	cd release && zip -r "$${pkgname}.zip" "$${pkgname}" && cd - && \
	ln -sf $${pkgname} $@

release/sgpc3:
	export driver=sgpc3 && \
	export tag="$$(git describe --always --dirty)" && \
	export pkgname="sgpc3-$${tag}" && \
	export pkgdir="release/$${pkgname}" && \
	rm -rf "$${pkgdir}" && mkdir -p "$${pkgdir}" && \
	cp -rl embedded-common/* "$${pkgdir}" && \
	cp -rl sgp-common/* "$${pkgdir}" && \
	cp -rl $${driver}/* "$${pkgdir}" && \
	sed -i 's/^sensirion_common_dir :=.*$$/sensirion_common_dir := ./' "$${pkgdir}/Makefile" && \
	sed -i 's/^sgp_common_dir :=.*$$/sgp_common_dir := ./' "$${pkgdir}/Makefile" && \
	sed -i "s/^\(#define\s\+SGP_DRV_VERSION_STR\s*\)\".*\"$$/\1\"$${tag}\"/" "$${pkgdir}/sgpc3.c" && \
	cd "$${pkgdir}" && make && make clean && cd - && \
	cd release && zip -r "$${pkgname}.zip" "$${pkgname}" && cd - && \
	ln -sf $${pkgname} $@

release: clean release/sgp30 release/sgpc3

clean:
	cd sgp30 && make clean && cd - && \
	cd sgpc3 && make clean && cd - && \
	rm -rf release
