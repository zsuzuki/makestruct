# makestruct

make C/C++ struct builder by toml description.

# build

```shell
$ cbuild
$ ninja
```

# usage

```shell
$ makestruct examples/test.toml
```

# toml-format

```toml
[struct]
name="param"

[[member]]
name="test"
size="uint8_t"
```
