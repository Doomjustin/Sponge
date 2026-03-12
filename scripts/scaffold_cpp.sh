#!/usr/bin/env bash
set -euo pipefail

name="${1:-}"
dir_input="${2:-}"
workspace="${3:-}"
ns="${4:-}"

# VS Code task may omit an empty middle argument, causing positional shift:
# scaffold_cpp.sh <name> <workspace> [namespace]
if [[ -z "$workspace" && -n "$dir_input" ]]; then
  workspace="$dir_input"
  dir_input=""
fi

if [[ -z "$name" ]]; then
  printf "%s\n" "ERROR: file name cannot be empty."
  exit 1
fi

if [[ ! "$name" =~ ^[A-Za-z][A-Za-z0-9_]*$ ]]; then
  printf "%s\n" "ERROR: file name must start with a letter and contain only letters, digits, or underscores."
  exit 1
fi

if [[ -z "$workspace" ]]; then
  printf "%s\n" "ERROR: workspace path is required."
  exit 1
fi

if [[ -z "$dir_input" ]]; then
  dir="$workspace/src"
elif [[ "$dir_input" == /* ]]; then
  dir="$dir_input"
else
  dir="$workspace/$dir_input"
fi

dir=$(realpath -m "$dir")
workspace=$(realpath -m "$workspace")
mkdir -p "$dir"

rel_dir="$dir"
if [[ "$dir" == "$workspace"/* ]]; then
  rel_dir="${dir#$workspace/}"
fi

if [[ -z "$ns" ]]; then
  if [[ "$rel_dir" == "src" ]]; then
    ns="sponge"
  elif [[ "$rel_dir" == src/* ]]; then
    ns_suffix="${rel_dir#src/}"
    ns="sponge::$(printf "%s" "$ns_suffix" | sed 's|/|::|g')"
  else
    printf "%s\n" "INFO: target dir '$rel_dir' is not under src, namespace defaults to 'sponge'."
    ns="sponge"
  fi
fi

h="$dir/$name.h"
cpp="$dir/$name.cpp"
test="$dir/$name.test.cpp"

for f in "$h" "$cpp" "$test"; do
  if [[ -e "$f" ]]; then
    printf "%s\n" "ERROR: target file already exists:" "  - $f" "No files were overwritten."
    exit 2
  fi
done

guard_name=$(echo "$name" | tr "[:lower:]" "[:upper:]")
guard_core="$guard_name"
if [[ "$guard_core" == SPG_* ]]; then
  guard_core="${guard_core#SPG_}"
fi

if [[ "$guard_core" == SPONGE_* ]]; then
  guard="${guard_core}_H"
else
  guard="SPONGE_${guard_core}_H"
fi

cat > "$h" <<EOF
#ifndef $guard
#define $guard

namespace $ns {

} // namespace $ns

#endif // $guard
EOF

cat > "$cpp" <<EOF
#include "$name.h"

namespace $ns {

} // namespace $ns
EOF

cat > "$test" <<EOF
#include "$name.h"

#include <doctest/doctest.h>
EOF

printf "%s\n" \
  "Scaffold created successfully:" \
  "  name      : $name" \
  "  namespace : $ns" \
  "  dir       : $dir" \
  "  files:" \
  "    - $h" \
  "    - $cpp" \
  "    - $test"
