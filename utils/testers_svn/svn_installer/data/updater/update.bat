@echo off
cd ..
set INS_LC_MESSAGES_BAK=%LC_MESSAGES%
set LC_MESSAGES=en_US
echo Updating Insurgency...
updater\bin\svn up --config-dir updater\cfg
echo Done updating Insurgency
echo Updating update scripts...
updater_svn\bin\delete_updater
updater_svn\bin\svn export --config-dir updater\cfg updater_svn updater
echo Done updating update scripts
set LC_MESSAGES=%INS_LC_MESSAGES_BAK%
set INS_LC_MESSAGES_BAK=
if NOT EXIST updater\postupdate.bat GOTO END
echo Running post-update script
cd updater
postupdate.bat
:END
pause