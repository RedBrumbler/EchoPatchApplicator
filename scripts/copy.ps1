param (
    [Parameter(Mandatory=$false)]
    [Switch]$log
)

$root = "${PSScriptRoot}/.."
$build = "${root}/build"
$lib = "${build}/libecho-patch-applicator.so"

& ${PSScriptRoot}/build.ps1

& adb push ${lib} /sdcard/ModData/com.readyatdawn.r15/Modloader/early_mods/libecho-patch-applicator.so

& adb shell am start com.readyatdawn.r15/com.oculus.gles3jni.MainActivity

if ($log.IsPresent) {
  & $PSScriptRoot/log.ps1
}
