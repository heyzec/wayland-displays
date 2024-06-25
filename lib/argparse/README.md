# argparse-cpp
A minimal reimplementation of Python's argparse library (https://docs.python.org/3/library/argparse.html)

The heavy lifting is done by the GNU getopt library. See the [official docs](https://www.gnu.org/software/libc/manual/html_node/Getopt.html) and the [man pages](https://www.man7.org/linux/man-pages/man3/getopt.3.html).

## Core Functionality

The library's support for command-line interfaces is built around an instance of `ArgumentParser`. It is a container for argument specifications and has options that apply to the parser as whole:

`argparse`
```python
parser = argparse.ArgumentParser(
                    prog='ProgramName',
                    description='What the program does',
                    epilog='Text at the bottom of help')
```
`argparse-cpp`
```cpp
ArgumentParser parser = ArgumentParser("ProgramName",
                                       "What the program does",
                                       "Text at the bottom of help");
```

The `ArgumentParser.add_argument()` method attaches individual argument specifications to the parser. It supports ~positional arguments~, options that accept values, and on/off flags:

`argparse`
```python
parser.add_argument('filename')           # positional argument
parser.add_argument('-c', '--count')      # option that takes a value
parser.add_argument('-v', '--verbose',
                    action='store_true')  # on/off flag
```
`argparse-cpp`
```cpp
// parser.add_argument('filename');           // positional argument
parser.add_argument("-c", "--count");      // option that takes a value
parser.add_argument("-v", "--verbose")
    ->action(STORE_TRUE);                  // on/off flag
```

The `ArgumentParser.parse_args()` method runs the parser and places the extracted data in a `Namespace` object:

`argparse`
```python
args = parser.parse_args()
print(args.filename, args.count, args.verbose)
```
`argparse-cpp`
```cpp
Namespace args = parser.parse_args();
// std::cout << args.get<std::string>("filename") << "\n"
std::cout << args.get<std::string>("count") << "\n"
std::cout << args.get<bool>("verbose") << "\n"
```
