Param(
    [Parameter(Mandatory=$false)]
    [Switch]$clean
)
$root = "${PSScriptRoot}/.."
$build = "${root}/build"

if ($clean.IsPresent)
{
    if (Test-Path -Path ${build})
    {
        remove-item ${build} -R
    }
}


& cmake -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" ${root} -B ${build}
& cmake --build ${build}
