# Detect Conan profile
detect:
    conan profile detect --force

# Initialize single-config build for a given build type
init build_type compiler='gcc' coverage='cov' modules='mod' sanitize='':
    conan install . --build=missing \
        -pr .pr/{{ compiler }} \
        -pr .pr/ninja \
        -pr .pr/{{ build_type }} \
        -o '&:use_modules={{ if modules =~ 'mod' { 'True' } else { 'False' } }}'
    cmake --preset conan-{{ build_type }} \
        -DSORTED_ARRAY_TOWER_ENABLE_COVERAGE={{ if coverage =~ 'cov' { 'ON' } else { 'OFF' } }} \
        {{ if sanitize != '' { '-DSORTED_ARRAY_TOWER_SANITIZE=' + sanitize } else { '' } }}

# Initialize single-config build for all build types
init-single compiler='gcc' coverage='cov' modules='mod' sanitize='':
    just init debug {{ compiler }} {{ coverage }} {{ modules }} {{ sanitize }}
    just init release {{ compiler }} {{ coverage }} {{ modules }} {{ sanitize }}
    just init relwithdebinfo {{ compiler }} {{ coverage }} {{ modules }} {{ sanitize }}
    just init minsizerel {{ compiler }} {{ coverage }} {{ modules }} {{ sanitize }}

# Initialize multi-config build for a given build type
initm build_type compiler='gcc' coverage='cov' modules='mod' sanitize='':
    conan install . --build=missing \
        -pr .pr/{{ compiler }} \
        -pr .pr/ninja-multi \
        -pr .pr/{{ build_type }} \
        -o '&:use_modules={{ if modules =~ 'mod' { 'True' } else { 'False' } }}'
    cmake --preset conan-default \
        -DSORTED_ARRAY_TOWER_ENABLE_COVERAGE={{ if coverage =~ 'cov' { 'ON' } else { 'OFF' } }} \
        {{ if sanitize != '' { '-DSORTED_ARRAY_TOWER_SANITIZE=' + sanitize } else { '' } }}

# Initialize multi-config build for all build types
init-multi compiler='gcc' coverage='cov' modules='mod' sanitize='':
    just initm debug {{ compiler }} {{ coverage }} {{ modules }} {{ sanitize }}
    just initm release {{ compiler }} {{ coverage }} {{ modules }} {{ sanitize }}
    just initm relwithdebinfo {{ compiler }} {{ coverage }} {{ modules }} {{ sanitize }}
    just initm minsizerel {{ compiler }} {{ coverage }} {{ modules }} {{ sanitize }}

# Run `init debug`
id: (init 'debug')

# Run `init release`
ir: (init 'release')

# Run `init debug` and `init release`
is: id ir

# Run `init-multi`
im: init-multi

# Run `init debug`, but with use_modules=False
ihd: (init 'debug' 'gcc' 'cov' '-')

# Run `init release`, but with use_modules=False
ihr: (init 'release' 'gcc' 'cov' '-')

# Run `init debug` and `init release`, but with use_modules=False
ihs: ihd ihr

# Run `init-multi`, but with use_modules=False
ihm: (init-multi 'gcc' 'cov' '-')

# Build code
build build_type *args:
    cmake --build --preset conan-{{ build_type }} {{ args }}

bd: (build 'debug')

br: (build 'release')

ba: bd br

alias b := bd

# Run tests
test build_type *args:
    ctest --preset conan-{{ build_type }} \
        --output-junit test-report.xml \
        -O build/{{ \
            if lowercase(build_type) =~ 'rel' { \
                if lowercase(build_type) =~ 'deb' { \
                    'RelWithDebInfo' \
                } else { \
                    if lowercase(build_type) =~ 'min' { \
                        'MinSizeRel' \
                    } else { \
                        'Release' \
                    } \
                }
            } else {
                'Debug'
            } \
        }}/test-report.txt \
        --output-on-failure \
        {{ args }}

td: (test 'debug')

tr: (test 'release')

ta: td tr

# Run benchmarks
benchmark build_type='release' *args:
    ./build/{{ \
            if lowercase(build_type) =~ 'rel' { \
                if lowercase(build_type) =~ 'deb' { \
                    'RelWithDebInfo' \
                } else { \
                    if lowercase(build_type) =~ 'min' { \
                        'MinSizeRel' \
                    } else { \
                        'Release' \
                    } \
                }
            } else {
                'Debug'
            } \
        }}/benchmark_sorted_array_tower {{ args }}

bmd: (benchmark 'debug')

bmr: (benchmark 'release')

bmrd: (benchmark 'relwithdebinfo')

bmm: (benchmark 'minsizerel')

# Run tests and generate a coverage report
build-cov build_type *args:
    just build {{ build_type }} --target coverage {{ args }}
    cat build/{{ \
            if lowercase(build_type) =~ 'rel' { \
                if lowercase(build_type) =~ 'deb' { \
                    'RelWithDebInfo' \
                } else { \
                    if lowercase(build_type) =~ 'min' { \
                        'MinSizeRel' \
                    } else { \
                        'Release' \
                    } \
                }
            } else {
                'Debug'
            } \
        }}/coverage_report/summary.txt

bcd: (build-cov 'debug')

bcr: (build-cov 'release')

bca: bcd bcr

# Show the coverage report in a web browser
show-cov build_type port='8070':
    python3 -m http.server --directory \
        build/{{ \
            if lowercase(build_type) =~ 'rel' { \
                if lowercase(build_type) =~ 'deb' { \
                    'RelWithDebInfo' \
                } else { \
                    if lowercase(build_type) =~ 'min' { \
                        'MinSizeRel' \
                    } else { \
                        'Release' \
                    } \
                }
            } else {
                'Debug'
            } \
        }}/coverage_report {{ port }}

scd: (show-cov 'debug')

scr: (show-cov 'release')


# Composite commands for testing different configurations
# =======================================================

# Clean --> Build
fresh-build build_type='debug' compiler='gcc' coverage='cov' modules='mod' sanitize='':
    just clean
    just init {{ build_type }} {{ compiler }} {{ coverage }} {{ modules }} {{ sanitize }}
    just build {{ build_type }}

# Clean --> Test
fresh-test build_type='debug' compiler='gcc' coverage='cov' modules='mod' sanitize='':
    just fresh-build {{ build_type }} {{ compiler }} {{ coverage }} \
        {{ modules }} {{ sanitize }}
    just test {{ build_type }}

# Clean --> Coverage report
fresh-cov build_type='debug' compiler='gcc' modules='mod' sanitize='':
    just fresh-build {{ build_type }} {{ compiler }} cov {{ modules }} {{ sanitize }}
    just build-cov {{ build_type }}

# Try building code and running tests for different configurations.
check-builds sanitize='address,undefined':
    just fresh-test debug gcc cov mod {{ sanitize }}
    just fresh-test debug gcc cov - {{ sanitize }}
    just fresh-test debug clang cov mod {{ sanitize }}
    just fresh-test debug clang cov - {{ sanitize }}

# Make a test report and a coverage report in debug.
make-reports compiler='gcc' modules='mod' sanitize='address,undefined':
    just clean
    just init debug {{ compiler }} cov {{ modules }} {{ sanitize }}
    just bd td bcd
