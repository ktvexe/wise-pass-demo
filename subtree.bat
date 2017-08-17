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
IF "%2" == "" (
	set branch=master
)

echo git subtree %cmd% --prefix=Platform https://github.com/ADVANTECH-Corp/EI-Platform.git %branch%
git subtree %cmd% --prefix=Platform https://github.com/ADVANTECH-Corp/EI-Platform.git %branch%

echo git subtree %cmd% --prefix=Include/wise https://github.com/ADVANTECH-Corp/WISEPredefine.git %branch%
git subtree %cmd% --prefix=Include/wise https://github.com/ADVANTECH-Corp/WISEPredefine.git %branch%