@echo off
set cmd=push
set branch=%2
IF "%1" == "push" (
	set cmd=%1
)
IF "%1" == "add" (
	set cmd=%1
)
IF "%1" == "pull" (
	set cmd=%1
)

IF "%1" == "checkout" (
	set cmd=add
	rmdir Lib_EI /s
	rmdir Platform /s
	rmdir Include/wise /s
)

IF "%2" == "" (
	set branch=master
)

echo git subtree %cmd%--prefix=Lib_EI http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Lib.git %branch%
git subtree %cmd% --prefix=Lib_EI http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Lib.git %branch%

echo git subtree %cmd% --prefix=Platform http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Platform.git %branch%
git subtree %cmd% --prefix=Platform http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Platform.git %branch%

echo git subtree %cmd% --prefix=Include/wise http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Predefine.git %branch%
git subtree %cmd% --prefix=Include/wise http://advgitlab.eastasia.cloudapp.azure.com/EI-Connect/EI-Predefine.git %branch%