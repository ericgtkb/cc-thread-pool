# A C++ thread pool implementation

## Usage
Copy the `thread_pool.h` file and use it.

## Test run
Using the meson build system, run the command

```
meson setup builddir
```

to generate build files.

Change to `builddir`.

```
cd builddir
```

Compile.

```
meson compile
```

Run the test with a simple function

```
./main
```

You should see the jobs are handled by a pool of threads.

