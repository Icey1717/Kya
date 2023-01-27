echo off
robocopy %1 %2 /XO /s > Robocopy.txt
EXIT /B 0