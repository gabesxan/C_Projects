export const REGIOES_DF = [
  [1, 'Plano Piloto'], [2, 'Gama'], [3, 'Taguatinga'], [4, 'Brazlândia'],
  [5, 'Sobradinho'], [6, 'Planaltina'], [7, 'Paranoá'], [8, 'Núcleo Bandeirante'],
  [9, 'Ceilândia'], [10, 'Guará'], [11, 'Cruzeiro'], [12, 'Samambaia'],
  [13, 'Santa Maria'], [14, 'São Sebastião'], [15, 'Recanto das Emas'],
  [16, 'Lago Sul'], [17, 'Riacho Fundo'], [18, 'Lago Norte'],
  [19, 'Candangolândia'], [20, 'Águas Claras'], [21, 'Riacho Fundo II'],
  [22, 'Sudoeste/Octogonal'], [23, 'Varjão'], [24, 'Park Way'],
  [25, 'SCIA/Estrutural'], [26, 'Sobradinho II'], [27, 'Jardim Botânico'],
  [28, 'Itapoã'], [29, 'SIA'], [30, 'Vicente Pires'], [31, 'Fercal'],
].map(([value, label]) => ({ value: String(value), label: `RA ${value} · ${label}` }))

export const TIPOS_EXAME = [
  { value: '1', label: 'Hemograma' },
  { value: '2', label: 'Raio-X' },
  { value: '3', label: 'Tomografia' },
  { value: '4', label: 'Ressonância magnética' },
  { value: '5', label: 'Eletrocardiograma' },
  { value: '6', label: 'Urina' },
  { value: '7', label: 'Ultrassonografia' },
]

export const STATUS_LABELS = {
  AGENDADO: 'Agendado',
  CANCELADO: 'Cancelado',
  CONCLUIDO: 'Concluído',
  SOLICITADO: 'Solicitado',
  AUTORIZADO: 'Autorizado',
  COLETADO: 'Coletado',
  EM_ANALISE: 'Em análise',
  INTERNADO: 'Internado',
  ALTA: 'Alta',
  DISPONIVEL: 'Disponível',
  OCUPADO: 'Ocupado',
  HIGIENIZACAO: 'Higienização',
  MANUTENCAO: 'Manutenção',
  BLOQUEADO: 'Bloqueado',
  ENTRADA: 'Entrada',
  SAIDA: 'Saída',
  AJUSTE: 'Ajuste',
}

export const statusLabel = (value) => STATUS_LABELS[value] ?? value
export const tipoExameLabel = (value) =>
  TIPOS_EXAME.find((tipo) => String(tipo.value) === String(value))?.label ?? `Tipo ${value}`
