#!/bin/bash
cmd=push
branch=$2

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

if [ "$2" == "" ]; then
	branch=master
fi

echo "git subtree ${cmd}--prefix=Lib_EI http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Lib.git ${branch}"
git subtree ${cmd} --prefix=Lib_EI http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Lib.git ${branch}

echo "git subtree ${cmd} --prefix=Platform http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Platform.git ${branch}"
git subtree ${cmd} --prefix=Platform http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Platform.git ${branch}

echo "git subtree ${cmd} --prefix=Include/wise http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Predefine.git ${branch}"
git subtree ${cmd} --prefix=Include/wise http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Predefine.git ${branch}
