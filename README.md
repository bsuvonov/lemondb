# LemonDB

## Introduction

- ./src  
   Contains the source code of `lemonDB`.
- ./test  
  Contains the test scripts of `lemonDB`.

### Details

- src/db  
  contains the source files related to the basic storing data structure
- src/query    
  contains the source files related to query implementation
- src/query/data    
  queries related to data manipulation
- src/query/management  
  queries related to table management
- src/query/utils  
  queries related to utilities
- main.cpp  
  program entry
- Manager.cpp/h  
  multi-thread manager

## Developer Quick Start

See INSTALL.md for instructions on building from source.

`ClangFormat` and `EditorConfig` are used to format codes.

Hint on using `ClangFormat`:
`find . -name "*.cpp" -o -name "*.h" | sed 's| |\\ |g' | xargs clang-format -i`

And make sure your code editor has `EditorConfig` support.

## Contributors

Bunyod Suvonov  
Wang Zhiyuan  
Wu Yixin  
Yan Weijie  

## Copyright

Lemonion Inc. 2018

