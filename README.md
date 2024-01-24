# ROSS Example

This is a small, ROSS example model to show how to have multiple LP types in [ROSS][], a
(massively) parallel discrete event simulator.

[ROSS]: https://github.com/ROSS-org/ROSS

# Compilation

The following are the instructions to download and compile:

```bash
git clone --recurse-submodules -j8 https://github.com/helq/ross-model-example
mkdir ross-model-example/build
cd ross-model-example/build
cmake .. -DCMAKE_INSTALL_PREFIX="$(pwd -P)/"
make install
```

After compiling, you will find the executable under the folder: `build/bin`

# Execution

An example of running in one core or two:

```bash
cd build
bin/modelbin --help
mpirun -np 2 bin/modelbin --sync=2 --batch=1 --pattern=5 --end=41
```

# Documentation

To generate the documentation, install [doxygen][] and dot (included in [graphviz][]), and
then run:

```bash
doxygen docs/Doxyfile
```

The documentation will be stored in `docs/html`.

[doxygen]: https://www.doxygen.nl/
[graphviz]: https://www.graphviz.org/
