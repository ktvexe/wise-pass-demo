MAIN_VERSION=1
SUB_VERSION=1
BUILD_VERSION=2
SVN_REVISION=0

RED='\033[0;31m'
LIGT_GREEN='\033[0;32m'
NO_COLOR='\033[0m' # No Color

function print_title()
{
	printf "${LIGT_GREEN}$1${NO_COLOR}\n"
}

function print_error()
{
	printf "${RED}$1${NO_COLOR}\n"
}

function initialize()
{
	git reset --hard || exit 1
	git checkout develop || exit 1
}

function subtree_pull_push()
{
	while : ; do
		ret=$(./subtree.sh pull 2>&1)
		echo -e "$ret"
		if [[ "$ret" != *"Failed"* ]]; then
			break
		fi
		sleep 1
		print_error "\nretry..."
	done
	
	while : ; do
		ret=$(./subtree.sh push 2>&1)
		echo -e "$ret"
		if [[ "$ret" != *"Failed"* ]]; then
			break
		fi
		sleep 1
		print_error "\nretry..."
	done
	while : ; do
		ret=$(./subtree.sh pull 2>&1)
		echo -e "$ret"
		if [[ "$ret" != *"Failed"* ]]; then
			break
		fi
		sleep 1
		print_error "\nretry..."
	done
}

function change_version()
{
	echo "${MAIN_VERSION}.${SUB_VERSION}.${BUILD_VERSION}" > Lib_EI/Include/VERSION || exit 1
	echo -e "#ifndef __SVN_REVISION_H__\n#define __SVN_REVISION_H__" > Lib_EI/Include/svnversion.h || exit 1
	echo "#define MAIN_VERSION ${MAIN_VERSION}" >> Lib_EI/Include/svnversion.h || exit 1
	echo "#define SUB_VERSION ${SUB_VERSION}" >> Lib_EI/Include/svnversion.h || exit 1
	echo "#define BUILD_VERSION ${BUILD_VERSION}" >> Lib_EI/Include/svnversion.h || exit 1
	echo "#define SVN_REVISION ${SVN_REVISION}" >> Lib_EI/Include/svnversion.h || exit 1
	echo "#endif /* __SVN_REVISION_H__ */" >> Lib_EI/Include/svnversion.h || exit 1
}

function commit_version()
{
	git add release-helper.sh || exit 1
	git add Lib_EI/Include/VERSION || exit 1
	git add Lib_EI/Include/svnversion.h || exit 1
	git commit -m "Update version to ${MAIN_VERSION}.${SUB_VERSION}.${BUILD_VERSION}" || exit 1
	git push origin develop || exit 1
}

print_title "initialize"
initialize

print_title "git pull"
git pull || exit 1

print_title "subtree push/pull/push"
subtree_pull_push

print_title "push version changes"
git push origin develop || exit 1

print_title "change version"
change_version

print_title "commit version"
commit_version

print_title "subtree push/pull/push"
subtree_pull_push

print_title "push subtree changes"
git push origin develop || exit 1

print_title "subtree push/pull/push"
subtree_pull_push
