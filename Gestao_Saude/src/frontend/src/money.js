// Valores monetarios trafegam em CENTAVOS (contrato do backend). Aqui ficam as
// conversoes para exibir e para enviar.

// Centavos (int) -> "R$ 1.234,56".
export function formatReais(centavos) {
  const n = Number(centavos) || 0
  return (n / 100).toLocaleString('pt-BR', {
    style: 'currency',
    currency: 'BRL',
  })
}

// Texto digitado em reais ("1.234,56" ou "1234.56") -> centavos (int) ou null.
export function parseReaisParaCentavos(texto) {
  if (texto == null) return null
  const limpo = String(texto).trim().replace(/\s|R\$/g, '')
  if (limpo === '') return null
  // Com virgula: ponto e separador de milhar (1.234,56). Sem virgula: ponto e
  // o separador decimal (1234.56).
  const normalizado = limpo.includes(',')
    ? limpo.replace(/\./g, '').replace(',', '.')
    : limpo
  const valor = Number(normalizado)
  if (!Number.isFinite(valor) || valor <= 0) return null
  return Math.round(valor * 100)
}
