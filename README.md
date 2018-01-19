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

# format(toml)

```toml
include=["cstdint","test.h"]
namespace="test"

[define]
ListMax=12

[struct]
name="param"
maxsize=20

# uint8_t test1{10}; // test member
[[struct.member]]
comment="test member"
name="test1"
size="uint8_t"
default=10

# char test2[4];
[[struct.member]]
name="test2"
size="char"
array=4

# uint16_t test3[ListMax];
[[struct.member]]
name="test3"
size="uint16_t"
array="ListMax"

# unsigned test4 : 3; // bit field
[[struct.member]]
comment="bit field"
name="test4"
size="unsigned"
bits=3

```

### TOML

format:
https://github.com/toml-lang/toml

cpptoml(library):
https://github.com/skystrife/cpptoml
