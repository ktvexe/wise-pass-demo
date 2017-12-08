#!/bin/bash
cmd=push
account=$2

if [ "$1" == "push" ] || [ "$1" == "add" ] || [ "$1" == "pull" ]; then
	cmd=$1
elif [ "$1" == "checkout" ]; then
	cmd=add
	rm -r Lib_EI
	rm -r Platform
	rm -r Include/wise
else
	echo "not support $1 $2"
	exit -1
fi

if [ -z "${account}" ]; then
	url="http://advgitlab.eastasia.cloudapp.azure.com"
else
	url="http://${account}@advgitlab.eastasia.cloudapp.azure.com"
fi
echo "git subtree ${cmd} --prefix=Lib_EI ${url}/EI-Connect/EI-Lib.git master"
git subtree ${cmd} --prefix=Lib_EI ${url}/EI-Connect/EI-Lib.git master

echo "git subtree ${cmd} --prefix=Platform ${url}/EI-Connect/EI-Platform.git master"
git subtree ${cmd} --prefix=Platform ${url}/EI-Connect/EI-Platform.git master

echo "git subtree ${cmd} --prefix=Include/wise ${url}/EI-Connect/EI-Predefine.git master"
git subtree ${cmd} --prefix=Include/wise ${url}/EI-Connect/EI-Predefine.git master
