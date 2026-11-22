# cbang
Build tool for projects where you dont want to create a separate file for building.<br>
## Building
```console
gcc cbang.c -o cbang
```
You may even add it to `/usr/local/bin` if you plan to use it habitually.<br>

## Usage
Suppose a main.c, such as:
```c
//$ gcc main.c -o main && ./main
//$x ./main
//$c gcc main.c -o main
	
int main() {
	printf("Hello seamen!\n");
}

```
Now you may run cbang, such as `cbang main.c`, which would run the first command, which is the default, or you can do `cbang exec main.c` or `cbang build main.c` for the second and third commands respectfully.<br>

Cbang can be built using itself which means `cbang.c` serves as another nice example.<br>

### Passing in arguments
If you define your execution command as `//$x $BIN "$@"` for example, `$@` does its duty as a shell variable, that is, to be replaced with the arguments passed in to the executable. To pass in arguments to cbang, separate them with `--`, such as `cbang exec foo.c -- foo bar baz`.

### Automatic variables

Cbang provides two automatic variables, `$SRC`, and `$BIN`, defined with `setenv()`.<br>
`$SRC` expands to the file passed as an argument to cbang, while `$BIN` equals to `{prefix + $SRC + OUTPUT_EXT}`, where the prefix is either nothing, if `$SRC` has no directory part, or `./` otherwise, and `OUTPUT_EXT` is a flag by default set to `.o`, which can be defined when building.<br>

### Other

To set the output extension at runtime, pass in `--ext=...` as a flag. The max length of the extension is 16 chars.<br>
