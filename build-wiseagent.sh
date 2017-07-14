#!/bin/bash

src_home=`pwd`
tmp_build_dir=${src_home}/build
release_dir=${src_home}/Release

compile_corelib()
{
	cd ${src_home}
	if [ ! -d "${src_home}/m4" ]; then
		mkdir "${src_home}/m4"
	fi

	autoreconf -i-f
	chmod 755 configure
	./configure
	make clean
	make
	make install DESTDIR=${tmp_build_dir}
}

generate_release()
{
	if [ ! -d "${release_dir}" ]; then
		mkdir "${release_dir}"
	fi

	if [ ! -d "${release_dir}/module" ]; then
		mkdir "${release_dir}/module"
	fi

	cp -f ${tmp_build_dir}/usr/local/lib/lib*.so* ${release_dir}/
	cp -f ${tmp_build_dir}/usr/local/bin/MosquittoSample ${release_dir}/
	cp -f ${tmp_build_dir}/usr/local/bin/WISECoreSample ${release_dir}/
}

compile_corelib
generate_release
exit 1 

