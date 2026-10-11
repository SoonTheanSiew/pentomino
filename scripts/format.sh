#!/usr/bin/env bash
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 SoonTheanSiew
#
# Apply and check the code style described in CONTRIBUTING.md.
#
#   scripts/format.sh                  format C++ in place, then check the rest
#   scripts/format.sh --check          change nothing, exit 1 on any problem
#   scripts/format.sh --names          also check naming with clang-tidy
#   scripts/format.sh --check --names  what CI runs
#
# Steps:
#   layout   clang-format with .clang-format (C++ only)
#   length   no line over 80 columns in C++, CMake and shell files. This
#            catches what clang-format cannot wrap, like long strings
#   files    C++ file names are UpperCamelCase, e.g. JsonLoader.hpp
#   guards   every header has an include guard named after its path, e.g.
#            generator/include/model/Edge.hpp uses
#            PENTO_GENERATOR_MODEL_EDGE_HPP
#   names    clang-tidy with .clang-tidy (only with --names). Needs
#            compile_commands.json, so configure a build first
#            (cmake -S . -B build)
#
# Environment:
#   CLANG_FORMAT   clang-format binary to use, e.g. clang-format-18
#   CLANG_TIDY     clang-tidy binary to use
#   BUILD_DIR      build directory holding compile_commands.json (build)

set -euo pipefail

readonly MAX_LINE_LENGTH=80
readonly SOURCE_DIRS=(core generator cli tests)

cd "$(dirname "${BASH_SOURCE[0]}")/.."

printUsage() {
    # The header comment above, without the license lines.
    sed -n '5,/^$/p' "${BASH_SOURCE[0]}" | sed -e 's/^# \{0,1\}//' -e '$d'
}

findTool() {
    local override="$1"
    shift
    if [[ -n "$override" ]]; then
        echo "$override"
        return
    fi
    local candidate
    for candidate in "$@"; do
        if command -v "$candidate" >/dev/null 2>&1; then
            echo "$candidate"
            return
        fi
    done
}

checkOnly=false
checkNames=false
for argument in "$@"; do
    case "$argument" in
        --check) checkOnly=true ;;
        --names) checkNames=true ;;
        -h | --help)
            printUsage
            exit 0
            ;;
        *)
            echo "usage: scripts/format.sh [--check] [--names]" >&2
            exit 2
            ;;
    esac
done

# Tracked and new files when inside git (ignored ones are skipped), otherwise a
# plain search. The same filter either way.
if git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
    listFiles() {
        git ls-files -z --cached --others --exclude-standard -- "$@"
    }
else
    listFiles() { find "$@" -type f -print0 2>/dev/null; }
fi

cppFiles=()
textFiles=()
while IFS= read -r -d '' path; do
    [[ -f "$path" ]] || continue
    case "$path" in
        *.cpp | *.hpp | *.h | *.cc)
            cppFiles+=("$path")
            textFiles+=("$path")
            ;;
        *CMakeLists.txt | *.cmake | *.sh) textFiles+=("$path") ;;
    esac
done < <(listFiles "${SOURCE_DIRS[@]}" CMakeLists.txt scripts)

failed=false

# --- layout ---------------------------------------------------------------
if [[ ${#cppFiles[@]} -gt 0 ]]; then
    clangFormat="$(findTool "${CLANG_FORMAT:-}" \
        clang-format clang-format-19 clang-format-18)"
    if [[ -z "$clangFormat" ]]; then
        echo "format.sh: clang-format not found." \
            "Install it or set CLANG_FORMAT." >&2
        exit 2
    fi

    if $checkOnly; then
        if "$clangFormat" --dry-run --Werror "${cppFiles[@]}"; then
            echo "format.sh: layout ok in ${#cppFiles[@]} files"
        else
            echo "format.sh: layout needs fixing; run scripts/format.sh" >&2
            failed=true
        fi
    else
        "$clangFormat" -i "${cppFiles[@]}"
        echo "format.sh: formatted ${#cppFiles[@]} files with" \
            "$("$clangFormat" --version | head -n1)"
    fi
fi

# --- length ---------------------------------------------------------------
if [[ ${#textFiles[@]} -gt 0 ]]; then
    if awk -v limit="$MAX_LINE_LENGTH" '
        length($0) > limit {
            printf "%s:%d: %d columns\n", FILENAME, FNR, length($0)
            found = 1
        }
        END { exit found }' "${textFiles[@]}" >&2; then
        echo "format.sh: no line over $MAX_LINE_LENGTH columns"
    else
        echo "format.sh: lines above are over $MAX_LINE_LENGTH columns;" \
            "shorten them by hand" >&2
        failed=true
    fi
fi

# --- files ----------------------------------------------------------------
badNames=()
for path in "${cppFiles[@]}"; do
    [[ "$(basename "$path")" =~ ^[A-Z][A-Za-z0-9]*\.(cpp|hpp|h|cc)$ ]] ||
        badNames+=("$path")
done
if [[ ${#badNames[@]} -eq 0 ]]; then
    echo "format.sh: file names ok"
else
    printf '%s: file name is not UpperCamelCase\n' "${badNames[@]}" >&2
    echo "format.sh: rename the files above by hand" >&2
    failed=true
fi

# --- guards ---------------------------------------------------------------
# generator/include/model/Edge.hpp -> PENTO_GENERATOR_MODEL_EDGE_HPP
guardFor() {
    local name
    name="$(echo "$1" | sed -E -e 's#/include/#/#' -e 's/\.[^.]*$//' \
        -e 's/([a-z0-9])([A-Z])/\1_\2/g' -e 's#/#_#g')"
    echo "PENTO_$(echo "$name" | tr '[:lower:]' '[:upper:]')_HPP"
}

badGuards=()
for path in "${cppFiles[@]}"; do
    [[ "$path" == *.hpp || "$path" == *.h ]] || continue
    guard="$(guardFor "$path")"
    if ! grep -qx "#ifndef $guard" "$path" ||
        ! grep -qx "#define $guard" "$path" ||
        [[ "$(tail -n 1 "$path")" != "#endif  // $guard" ]]; then
        badGuards+=("$path: expected include guard $guard")
    fi
done
if [[ ${#badGuards[@]} -eq 0 ]]; then
    echo "format.sh: include guards ok"
else
    printf '%s\n' "${badGuards[@]}" >&2
    echo "format.sh: fix the include guards above by hand" >&2
    failed=true
fi

# --- names ----------------------------------------------------------------
if $checkNames && [[ ${#cppFiles[@]} -gt 0 ]]; then
    clangTidy="$(findTool "${CLANG_TIDY:-}" \
        clang-tidy clang-tidy-19 clang-tidy-18)"
    buildDir="${BUILD_DIR:-build}"
    if [[ -z "$clangTidy" ]]; then
        echo "format.sh: clang-tidy not found." \
            "Install it or set CLANG_TIDY." >&2
        exit 2
    fi
    if [[ ! -f "$buildDir/compile_commands.json" ]]; then
        echo "format.sh: $buildDir/compile_commands.json not found;" \
            "run cmake -S . -B $buildDir or set BUILD_DIR" >&2
        exit 2
    fi

    # Headers are checked through the sources that include them.
    sourceFiles=()
    for path in "${cppFiles[@]}"; do
        [[ "$path" == *.cpp || "$path" == *.cc ]] && sourceFiles+=("$path")
    done
    # On macOS clang-tidy may not find the system headers by itself.
    tidyArgs=(--quiet -p "$buildDir")
    if [[ "$(uname)" == Darwin ]] && command -v xcrun >/dev/null 2>&1; then
        tidyArgs+=("--extra-arg=-isysroot$(xcrun --show-sdk-path)")
    fi
    if "$clangTidy" "${tidyArgs[@]}" "${sourceFiles[@]}"; then
        echo "format.sh: names ok"
    else
        echo "format.sh: naming problems above; rename by hand" >&2
        failed=true
    fi
fi

if $failed; then
    exit 1
fi
