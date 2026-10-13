# `target_architecture_name()`

**Contrato**

```cpp
std::string target_architecture_name();
```

No recibe argumentos. Traduce macros de arquitectura de compilación a nombres como `x86_64`, `aarch64` o `riscv`; devuelve cadena vacía si no reconoce una.
