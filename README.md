# WinDD
WinDD aims to port the `dd` utility from Unix-adjacent systems to Windows. 
`dd`  (which stands for (d)ata (d)efinition) is a shell command for reading, writing and converting file data. 
The command supports reading and writing files, and if a driver is available to support file-like access, the command can access devices too.

WinDD tries to preserve the original functionality of `dd` as written in the GNU core utilities. 
We want an exact (or as close to exact) clone of `dd`'s features for Windows. 
This means the interface should be as close to the original version as possible.
Exceptions will be specified in later sections of this document.
They will mostly address changes in the interface due to fundamental differences between Unix and Windows.

## Usage
As WinDD is a near-precise port of `dd`, most usage questions can be answered here: https://www.gnu.org/savannah-checkouts/gnu/coreutils/manual/html_node/dd-invocation.html.
Certain caveats that make WinDD different due to OS design will be addressed below:
- Rather than detecting if `if=` or `of=` is a directory, Windows denies access when attempting to even open a directory. Therefore, it is impossible to distinguish trying to open a directory versus permissions issue.
- Certain rare errors may be different from Unix `dd` in their message due to implementation differences.
- The number of whole/partial blocks reported may differ from `dd` due to implementation differences.
- Windows will prevent you from reading a physical disk or volume with a custom block size.
- WinDD will report `\\.\PhysicalDrive#` as not found. You have to run it as administrator.

Due to these caveats, some hints will be provided in WinDD's output. They are best-effort guesses and may not represent the true situation.

## Support
As `dd` has numerous features, implementing all of them at once is infeasible. Below, a table of features is listed in descending priority:

`dd` | WinDD
--- | ---
Copy between two named files on disk. | :white_check_mark:
Copy a disk to a disk, a disk to a file, and vice versa. | :white_check_mark:
Block size support (`ibs`, `obs`, `bs`) | :white_check_mark:
Skipping and seeking (`skip`, `iseek`, `oseek`, `seek`) | :construction:
`count` option | :white_check_mark:
Status indicator (`status`) | :white_check_mark:
Friendly units (`b`, `k`, `m/M`, `g/G`, `kB`, `MB`, `GB`, etc.) | :construction:
Conversions (`conv`, `cbs`, etc.) | :x:
Other flags (`iflag`, `oflag`, etc.) | :x:
Help menu (`--help`) | :x:
Obscure features not on here (see below on suggestions) | :x:

The first release will happen once basic functionality is achieved (all features up to status indicator). The full release will contain all the features above except for the last row.

## Contributing
To keep contributing easy, just follow these general guidelines:

- If you find a bug or have a suggestion, create an issue.
- If you have a fix or want to add a feature, create a pull request.
- Although the official language `dd` is C, WinDD uses C++ as it has better support for object encapsulation, string manipulation, and its abstract data structures are safer than reimplementing in C.
- Should this project outgrow these guidelines, more details will be published in issue and pull request templates.

## Coding Conventions
WinDD follows (for the most part) the ISO C++20 standard. 
As WinDD is targeted for Windows, certain MSVC extensions are allowed. 
The code does not have to be portable to other operating systems.
- Classes, structs, namespaces, and types are `PascalCase`.
- Member functions and variables/struct fields are `camelCase`.
- Global functions and variables outside an OOP context are `snake_case`.
- Local variables and parameters are always snake case.
- Prefer brevity over verbosity: only use `this->` in ambiguous contexts (`var` could be in `snake_case` or `camelCase`, so use `this->var` to distinguish).
- Comments should describe why something is done. Well written code should describe the implementation itself.
- Prefer readability over specific language features unless justified (usually a comment should describe if not clear enough).
- Prefer smart memory management over manual memory management unless performant code demands otherwise.
- Treat C++ differently than C, and use the STL rather than implementing from scratch. 
- Rarely ever use `new/delete`, and try to avoid unsafe C idioms unless interfacing with the Windows API.
- Other conventions can be dealt with on a case-by-case basis.