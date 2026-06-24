<div align="center">

# 🖼️ Imagens do projeto

### Capturas de tela, diagramas e mídia da documentação

![Uso](https://img.shields.io/badge/uso-documentação-0b7285)
![Formatos](https://img.shields.io/badge/formatos-PNG%20%7C%20JPG%20%7C%20SVG-555)

</div>

---

## 🎯 Para que serve

Esta pasta guarda as **imagens usadas pela documentação** — capturas de tela,
diagramas e ilustrações referenciadas pelo [`README.md`](../README.md), pelo
[`manual.md`](../manual.md) e pelos documentos em [`docs/`](../docs).

---

## 🗂️ Como organizar

| Tipo | Sugestão de nome | Exemplo |
|---|---|---|
| Captura de tela | `tela-<contexto>.png` | `tela-login.png`, `tela-triagem.png` |
| Diagrama | `diagrama-<tema>.png/svg` | `diagrama-arquitetura.svg` |
| Fluxo | `fluxo-<assunto>.png` | `fluxo-triagem.png` |

> [!TIP]
> Prefira nomes **descritivos em kebab-case** e sem espaços. Para diagramas,
> SVG escala melhor; para telas, PNG preserva o texto com nitidez.

---

## 🔗 Como referenciar na documentação

```markdown
![Tela de triagem](imagens/tela-triagem.png)
```

> [!NOTE]
> **Assets internos da aplicação** (logos, ícones e imagens que entram no build)
> **não** ficam aqui — eles vivem em
> [`src/frontend/src/assets`](../src/frontend/src/assets) ou
> [`src/frontend/public`](../src/frontend/public), pois fazem parte do frontend.
