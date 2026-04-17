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
- There are currently no caveats that have came up.

## Support
As `dd` has numerous features, implementing all of them at once is infeasible. Below, a table of features is listed in descending priority:

`dd` | WinDD
- | - 
Copy between two named files on disk. | :construction:
Copy a disk to a disk, a disk to a file, and vice versa. | :construction:
Block size support (`ibs`, `obs`, `bs`) | :x:
Skipping and seeking (`skip`, `iseek`, `oseek`, `seek`) | :x:
`count` option | :x:
Status indicator (`status`) | :x:
Friendly units (`b`, `k`, `m/M`, `g/G`, `kB`, `MB`, `GB`, etc.) | :x:
Conversions (`conv`, `cbs`, etc.) | :x:
Other flags (`iflag`, `oflag`, etc.) | :x:
Help menu (`--help`) | :x:
Obscure features not on here (see below on suggestions) | :x:

The first release will happen once basic functionality is achieved (all features up to status indicator). The full release will contain all the features above except for the last row.

## Contributing
To keep contributing easy, just follow these general guidelines:

- If you find a bug or have a suggestion, create an issue.
- If you have a fix or want to add a feature, create a pull request.
- Follow https://learn.microsoft.com/en-us/windows/win32/learnwin32/windows-coding-conventions, except *do not use Hungarian notation*. Even Microsoft admits: "Internally, the Windows team no longer uses it."
- To keep languages consistent, the official language of WinDD is C, the same as `dd`.
- Should this project outgrow these guidelines, more details will be published in issue and pull request templates.