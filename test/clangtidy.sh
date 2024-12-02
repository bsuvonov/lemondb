#!/bin/bash

# Find all .cpp and .h files
files=$(find ../src -name "*.cpp" -o -name "*.h")

#!/bin/bash

# Find all .cpp and .h files in the src directory and its subdirectories
files=$(find ../src -name "*.cpp" -o -name "*.h")

# Path to compile_commands.json
compile_commands_path="../build/compile_commands.json"

# Run clang-tidy on each file
for file in $files; do
  clang-tidy "$file" -p="$compile_commands_path" -checks='-*,clang-analyzer-*,-clang-analyzer-alpha*,cppcoreguidelines-*,bugprone-suspicious-string-compare,google-global-names-in-headers,clang-diagnostic-*,misc-*,performance-*' \
  -warnings-as-errors='codequality-no-header-guard,cppcoreguidelines-init-variables,codequality-no-public-member-variables,readability-redundant-*,performance' \
  --extra-arg=-D__clang_analyzer__ \
  --extra-arg-before=-std=c++20 \
  --extra-arg-before=-x --extra-arg-before=c++ \
  -header-filter='^(\.\./src/.*|.*\.h)$' \
  -- \
  -Wsystem-headers \
  # -fix -fix-errors
  # clang-tidy's fix is NOT believable...
done