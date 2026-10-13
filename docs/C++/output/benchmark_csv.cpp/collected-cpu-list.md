# `collected()` — lista de CPUs

**Contrato**

```cpp
std::string collected(
    const CollectedValue<std::vector<unsigned>> &value);
```

`value` es una colección de CPUs con estado y fuente. Devuelve estos datos y, si está disponible, la lista separada por comas. La sobrecarga especializa el formato vectorial para snapshots CSV.
