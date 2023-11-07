$logfile = "${PSScriptRoot}/../log.log"

if (Test-Path ${logfile}) {
    & del ${logfile}
}

& adb logcat -c
& adb logcat > ${logfile}
