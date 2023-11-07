
$root = "${PSScriptRoot}/.."
$build = "${root}/build"

& cmake -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" ${root} -B ${build}
& cmake --build ${build}
