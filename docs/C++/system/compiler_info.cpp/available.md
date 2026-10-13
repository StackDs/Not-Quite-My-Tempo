# `available()`

**Contrato**

```cpp
CollectedValue<std::string> available(std::string value);
```

Envuelve `value` como información disponible y registra como fuente las macros predefinidas del compilador. El parámetro se recibe por valor; devuelve `CollectedValue<std::string>`.
