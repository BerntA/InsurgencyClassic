@echo off
cd ..
set INS_LC_MESSAGES_BAK=%LC_MESSAGES%
set LC_MESSAGES=en_US
echo Installing Insurgency...
updater\bin\svn co --config-dir updater\cfg https://svn.insmod.net/svn/game/Internal .
echo Done installing Insurgency
set LC_MESSAGES=%INS_LC_MESSAGES_BAK%
set INS_LC_MESSAGES_BAK=
cd updater
update.bat