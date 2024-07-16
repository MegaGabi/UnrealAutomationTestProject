@echo off 

call "%~dp0\..\config.bat"

goto:skipbuild
call "%RunUATPath%" BuildCookRun ^
-project="%ProjectPath%" ^
-platform="%Platform%" ^
-clientconfig="%Configuration%" ^
-archivedirectory="%ArchivePath%" ^
-build -cook 


rem run tests
"%EditorPath%" "%ProjectPath%" -ExecCmds="Automation RunTests %TestName%;Quit" ^
-log -abslog="%TestOutputLogPath%" -nosplash -ReportOutputPath="%ReportOutputPath%"
:skipbuild

rem copy test artifacts
set TestsDir=%~dp0
set TestsDataDir=%~dp0data
robocopy "%TestsDataDir%" "%ReportOutputPath%" /E

rem start local server and show report
set Port=8081

pushd "%ReportOutputPath%"
call http-server -p="%Port%"
popd