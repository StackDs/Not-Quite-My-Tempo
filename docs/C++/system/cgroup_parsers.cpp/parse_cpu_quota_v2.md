# `parse_cpu_quota_v2()`

**Contrato**

```cpp
CollectedValue<CpuQuota> parse_cpu_quota_v2(const std::string &input,
                                            const std::string &source);
```

`input` contiene cuota y periodo v2 separados por whitespace; `source` registra el origen. Devuelve `unlimited` para `max`, disponible para números positivos válidos y `malformed` para formato o valores inválidos.
