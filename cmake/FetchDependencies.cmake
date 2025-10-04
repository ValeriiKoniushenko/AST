include_guard()

include(FetchContent)

function(installBoostModule module_name boost_version)
    string(PREPEND full_module_name sw_boost_)
    set(full_module_name "${full_module_name}${module_name}")

    FetchContent_Declare(${full_module_name}
        GIT_REPOSITORY "https://github.com/boostorg/${module_name}.git"
        GIT_TAG ${boost_version}
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(${full_module_name})
endfunction()


set(BOOST_VERSION boost-1.88.0)
set(BOOST_MODULES
    assert
    static_assert
    cmake
    config
    core
    headers
    throw_exception
    smart_ptr
)

foreach (module IN LISTS BOOST_MODULES)
    installBoostModule(${module} ${BOOST_VERSION})
endforeach ()

FetchContent_Declare(Utils
    GIT_REPOSITORY https://github.com/ValeriiKoniushenko/Utils.git
    GIT_TAG e162d718fe984f3dd3a66a3666ab83632e418669
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)

FetchContent_MakeAvailable(Utils)


FetchContent_Declare(SpdLog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.15.3
    GIT_SHALLOW TRUE
    GIT_PROGRESS TRUE
)
set(SPDLOG_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(SPDLOG_BUILD_BENCH OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(SpdLog)
