# Running artefacts

If you have downloaded the binary aretfacts of this repository compiled using our Jenkins build nodes, you will need to manually set the path to the *lib* directory. This is done by setting the `LD_LIBRARY_PATH` environment variable. E.g. to run *daquiri* using *Bash*, execute the following line:

```
LD_LIBRARY_PATH=../lib ./daquiri
```