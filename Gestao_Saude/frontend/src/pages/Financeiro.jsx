import { useCallback, useEffect, useState } from 'react'
import { apiGet, apiSend } from '../api/client'
import { formatReais, parseReaisParaCentavos } from '../money'
import { SearchSelect } from '../components/FieldSelect'
import {
  PageHeader,
  StatCard,
  Card,
  Button,
  Alert,
  Spinner,
  Badge,
  EmptyState,
} from '../components/ui'

const inputCls =
  'mt-1 block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30 outline-none'

const STATUS_TONE = {
  PENDENTE: 'amber',
  AUTORIZADA: 'sky',
  PAGA: 'green',
  GLOSADA: 'red',
  CANCELADA: 'slate',
}

// Transicoes oferecidas na UI a partir do status atual. O backend aceita
// qualquer status valido enquanto a cobranca nao estiver encerrada
// (PAGA/GLOSADA/CANCELADA); aqui sugerimos apenas os caminhos sensatos.
const TRANSICOES = {
  PENDENTE: ['AUTORIZADA', 'PAGA', 'GLOSADA', 'CANCELADA'],
  AUTORIZADA: ['PAGA', 'GLOSADA', 'CANCELADA'],
}
const ACAO_LABEL = {
  AUTORIZADA: 'Autorizar',
  PAGA: 'Pagar',
  GLOSADA: 'Glosar',
  CANCELADA: 'Cancelar',
}
const ACAO_VARIANT = { GLOSADA: 'danger', CANCELADA: 'danger' }

// --- Convenios ------------------------------------------------------------
function Convenios({ onErro }) {
  const [convenios, setConvenios] = useState(null)
  const [nome, setNome] = useState('')
  const [cobertura, setCobertura] = useState('100')

  const carregar = useCallback(() => {
    apiGet('/convenios').then(setConvenios).catch((e) => onErro(e.message))
  }, [onErro])

  useEffect(() => { carregar() }, [carregar])

  async function criar(e) {
    e.preventDefault()
    if (!nome.trim()) return
    try {
      await apiSend('POST', '/convenios', { nome, cobertura_pct: cobertura || '100' })
      setNome('')
      setCobertura('100')
      carregar()
    } catch (err) {
      onErro(err.message)
    }
  }

  async function desativar(c) {
    if (!window.confirm(`Desativar o convenio "${c.nome}"?`)) return
    try {
      await apiSend('DELETE', `/convenios/${c.id}`)
      carregar()
    } catch (err) {
      onErro(err.message)
    }
  }

  return (
    <Card className="p-5">
      <p className="text-sm font-semibold text-slate-700">Convenios</p>
      <form onSubmit={criar} className="mt-3 flex flex-wrap items-end gap-2">
        <label className="text-sm text-slate-600">
          Novo convenio
          <input className={inputCls} value={nome} onChange={(e) => setNome(e.target.value)} />
        </label>
        <label className="text-sm text-slate-600">
          Cobertura (%)
          <input className={inputCls} type="number" min="0" max="100" value={cobertura}
            onChange={(e) => setCobertura(e.target.value)} />
        </label>
        <Button type="submit">Adicionar</Button>
      </form>

      {convenios === null && <div className="mt-3"><Spinner /></div>}
      {Array.isArray(convenios) && convenios.length === 0 && (
        <p className="mt-3 text-sm text-slate-500">Nenhum convenio cadastrado.</p>
      )}
      {Array.isArray(convenios) && convenios.length > 0 && (
        <ul className="mt-3 divide-y divide-slate-100">
          {convenios.map((c) => (
            <li key={c.id} className="flex items-center justify-between py-2">
              <span className="text-sm text-slate-700">
                {c.nome} <span className="text-slate-400">• cobre {c.coberturaPct}%</span>
              </span>
              <Button variant="danger" className="px-3 py-1" onClick={() => desativar(c)}>
                Desativar
              </Button>
            </li>
          ))}
        </ul>
      )}
    </Card>
  )
}

// --- Nova cobranca --------------------------------------------------------
function NovaCobranca({ onCriada, onErro }) {
  const vazio = { paciente_id: '', forma: 'PARTICULAR', convenio_id: '', origem: '', descricao: '', valor: '', vencimento: '', guia: '', guia_validade: '' }
  const [v, setV] = useState(vazio)
  const [convenios, setConvenios] = useState([])

  function set(k, val) { setV((s) => ({ ...s, [k]: val })) }

  useEffect(() => {
    apiGet('/convenios').then(setConvenios).catch(() => setConvenios([]))
  }, [])

  async function submit(e) {
    e.preventDefault()
    const centavos = parseReaisParaCentavos(v.valor)
    if (!v.paciente_id) { onErro('Informe o paciente.'); return }
    if (centavos === null) { onErro('Valor invalido.'); return }
    if (v.forma === 'CONVENIO' && !v.convenio_id) { onErro('Selecione o convenio.'); return }
    try {
      await apiSend('POST', '/cobrancas', {
        paciente_id: v.paciente_id,
        forma: v.forma,
        convenio_id: v.forma === 'CONVENIO' ? v.convenio_id : 0,
        origem: v.origem,
        descricao: v.descricao,
        valor_centavos: centavos,
        vencimento: v.vencimento,
        guia: v.forma === 'CONVENIO' ? v.guia : '',
        guia_validade: v.forma === 'CONVENIO' ? v.guia_validade : '',
      })
      setV(vazio)
      onCriada()
    } catch (err) {
      onErro(err.message)
    }
  }

  return (
    <Card className="p-5">
      <p className="text-sm font-semibold text-slate-700">Nova cobranca</p>
      <form onSubmit={submit} className="mt-3 grid grid-cols-1 gap-3 sm:grid-cols-2 lg:grid-cols-3">
        <label className="text-sm text-slate-600">
          Paciente
          <SearchSelect
            value={v.paciente_id}
            onChange={(val) => set('paciente_id', val)}
            path="/pacientes/buscar"
            optionLabel={(p) => `${p.nome}${p.documento ? ` · ${p.documento}` : ''}`}
          />
        </label>
        <label className="text-sm text-slate-600">
          Forma
          <select className={inputCls} value={v.forma} onChange={(e) => set('forma', e.target.value)}>
            <option value="PARTICULAR">Particular</option>
            <option value="CONVENIO">Convenio</option>
          </select>
        </label>
        {v.forma === 'CONVENIO' && (
          <label className="text-sm text-slate-600">
            Convenio
            <select className={inputCls} value={v.convenio_id} onChange={(e) => set('convenio_id', e.target.value)}>
              <option value="">Selecione...</option>
              {convenios.map((c) => (
                <option key={c.id} value={c.id}>{c.nome}</option>
              ))}
            </select>
          </label>
        )}
        <label className="text-sm text-slate-600">
          Valor (R$)
          <input className={inputCls} placeholder="0,00" value={v.valor} onChange={(e) => set('valor', e.target.value)} />
        </label>
        <label className="text-sm text-slate-600">
          Origem
          <input className={inputCls} placeholder="Ex.: consulta, exame" value={v.origem} onChange={(e) => set('origem', e.target.value)} />
        </label>
        <label className="text-sm text-slate-600">
          Descricao
          <input className={inputCls} value={v.descricao} onChange={(e) => set('descricao', e.target.value)} />
        </label>
        <label className="text-sm text-slate-600">
          Vencimento
          <input className={inputCls} type="date" value={v.vencimento} onChange={(e) => set('vencimento', e.target.value)} />
        </label>
        {v.forma === 'CONVENIO' && (
          <label className="text-sm text-slate-600">
            Guia (autorizacao)
            <input className={inputCls} value={v.guia} onChange={(e) => set('guia', e.target.value)} />
          </label>
        )}
        {v.forma === 'CONVENIO' && (
          <label className="text-sm text-slate-600">
            Validade da guia
            <input className={inputCls} type="date" value={v.guia_validade} onChange={(e) => set('guia_validade', e.target.value)} />
          </label>
        )}
        <div className="flex items-end">
          <Button type="submit">Lancar cobranca</Button>
        </div>
      </form>
    </Card>
  )
}

// --- Lista de cobrancas ---------------------------------------------------
function ListaCobrancas({ cobrancas, onMudarStatus }) {
  if (cobrancas === null) return <Spinner />
  if (cobrancas.length === 0) {
    return <EmptyState title="Sem cobrancas" description="Nenhuma cobranca lancada ainda." />
  }
  return (
    <div className="overflow-x-auto rounded-xl bg-white shadow-sm ring-1 ring-slate-200">
      <table className="min-w-full text-sm">
        <thead>
          <tr className="border-b border-slate-200 bg-slate-50 text-left text-slate-500">
            <th className="px-4 py-3 font-semibold">ID</th>
            <th className="px-4 py-3 font-semibold">Paciente</th>
            <th className="px-4 py-3 font-semibold">Forma</th>
            <th className="px-4 py-3 font-semibold">Origem</th>
            <th className="px-4 py-3 font-semibold">Valor</th>
            <th className="px-4 py-3 font-semibold">Coparticip.</th>
            <th className="px-4 py-3 font-semibold">Vencimento</th>
            <th className="px-4 py-3 font-semibold">Status</th>
            <th className="px-4 py-3 font-semibold">Acoes</th>
          </tr>
        </thead>
        <tbody>
          {cobrancas.map((c) => (
            <tr key={c.id} className="border-b border-slate-100 last:border-0 hover:bg-slate-50/70">
              <td className="px-4 py-3 text-slate-500">#{c.id}</td>
              <td className="px-4 py-3 text-slate-700">#{c.pacienteId}</td>
              <td className="px-4 py-3 text-slate-700">{c.forma}</td>
              <td className="px-4 py-3 text-slate-700">
                {c.origem || '—'}
                {c.guia && <span className="block text-xs text-slate-400">guia {c.guia}</span>}
              </td>
              <td className="px-4 py-3 font-semibold text-slate-900">{formatReais(c.valorCentavos)}</td>
              <td className="px-4 py-3 text-slate-700">
                {c.copartCentavos > 0 ? formatReais(c.copartCentavos) : '—'}
              </td>
              <td className="px-4 py-3 whitespace-nowrap">
                {c.vencimento || '—'}
                {c.vencida && <span className="ml-1"><Badge tone="red">Vencida</Badge></span>}
              </td>
              <td className="px-4 py-3">
                <Badge tone={STATUS_TONE[c.status]}>{c.status}</Badge>
                {c.motivo && <span className="ml-1 text-xs text-slate-400">({c.motivo})</span>}
              </td>
              <td className="px-4 py-3">
                <div className="flex flex-wrap gap-2">
                  {(TRANSICOES[c.status] || []).map((destino) => (
                    <Button
                      key={destino}
                      variant={ACAO_VARIANT[destino] || 'secondary'}
                      className="px-3 py-1"
                      onClick={() => onMudarStatus(c, destino)}
                    >
                      {ACAO_LABEL[destino]}
                    </Button>
                  ))}
                </div>
              </td>
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )
}

// --- Lotes de faturamento -------------------------------------------------
const LOTE_TONE = { ABERTO: 'amber', FECHADO: 'sky', PAGO: 'green' }

function Lotes({ cobrancas, onMudou, onErro }) {
  const [lotes, setLotes] = useState(null)
  const [convenios, setConvenios] = useState([])
  const [convenioId, setConvenioId] = useState('')
  const [sel, setSel] = useState(null)
  const [fatura, setFatura] = useState(null)

  const carregarLotes = useCallback(() => {
    apiGet('/lotes').then(setLotes).catch((e) => onErro(e.message))
  }, [onErro])

  useEffect(() => { carregarLotes() }, [carregarLotes])
  useEffect(() => {
    apiGet('/convenios').then(setConvenios).catch(() => setConvenios([]))
  }, [])
  useEffect(() => {
    queueMicrotask(() => {
      if (sel == null) { setFatura(null); return }
      apiGet(`/lotes/${sel}`).then(setFatura).catch((e) => onErro(e.message))
    })
  }, [sel, onErro])

  function recarregar() {
    carregarLotes()
    if (sel != null) apiGet(`/lotes/${sel}`).then(setFatura).catch(() => { })
    onMudou()
  }

  async function criar(e) {
    e.preventDefault()
    if (!convenioId) return
    try {
      const r = await apiSend('POST', '/lotes', { convenio_id: convenioId })
      setConvenioId('')
      carregarLotes()
      if (r && r.id) setSel(r.id)
    } catch (err) {
      onErro(err.message)
    }
  }

  async function acao(loteId, qual) {
    try {
      await apiSend('POST', `/lotes/${loteId}/${qual}`)
      recarregar()
    } catch (e) {
      onErro(e.message)
    }
  }

  async function vincular(loteId, cobrancaId, rota) {
    try {
      await apiSend('POST', `/lotes/${loteId}/${rota}`, { cobranca_id: cobrancaId })
      recarregar()
    } catch (e) {
      onErro(e.message)
    }
  }

  const loteSel = Array.isArray(lotes) ? lotes.find((l) => l.id === sel) : null
  // Cobrancas elegiveis para o lote selecionado: CONVENIO autorizadas, sem
  // lote e do mesmo convenio do lote.
  const elegiveis =
    loteSel && loteSel.status === 'ABERTO' && Array.isArray(cobrancas)
      ? cobrancas.filter(
        (c) =>
          c.forma === 'CONVENIO' &&
          c.status === 'AUTORIZADA' &&
          c.loteId === 0 &&
          c.convenioId === loteSel.convenioId,
      )
      : []

  return (
    <Card className="p-5">
      <p className="text-sm font-semibold text-slate-700">Lotes de faturamento</p>
      <form onSubmit={criar} className="mt-3 flex flex-wrap items-end gap-2">
        <label className="text-sm text-slate-600">
          Convenio
          <select className={inputCls} value={convenioId} onChange={(e) => setConvenioId(e.target.value)}>
            <option value="">Selecione...</option>
            {convenios.map((c) => (
              <option key={c.id} value={c.id}>{c.nome}</option>
            ))}
          </select>
        </label>
        <Button type="submit">Abrir lote</Button>
      </form>

      {lotes === null && <div className="mt-3"><Spinner /></div>}
      {Array.isArray(lotes) && lotes.length === 0 && (
        <p className="mt-3 text-sm text-slate-500">Nenhum lote.</p>
      )}
      {Array.isArray(lotes) && lotes.length > 0 && (
        <ul className="mt-3 divide-y divide-slate-100">
          {lotes.map((l) => (
            <li key={l.id} className="py-2">
              <div className="flex flex-wrap items-center justify-between gap-2">
                <button
                  type="button"
                  className="text-left text-sm text-slate-700 hover:text-teal-700"
                  onClick={() => setSel(sel === l.id ? null : l.id)}
                >
                  <span className="font-semibold">Lote #{l.id}</span> • {l.convenioNome} •{' '}
                  {l.quantidade} cobr. • {formatReais(l.totalCentavos)}
                </button>
                <div className="flex items-center gap-2">
                  <Badge tone={LOTE_TONE[l.status]}>{l.status}</Badge>
                  {l.status === 'ABERTO' && (
                    <Button variant="secondary" className="px-3 py-1" onClick={() => acao(l.id, 'fechar')}>Fechar</Button>
                  )}
                  {l.status === 'FECHADO' && (
                    <Button className="px-3 py-1" onClick={() => acao(l.id, 'pagar')}>Pagar</Button>
                  )}
                </div>
              </div>

              {sel === l.id && fatura && (
                <div className="mt-2 rounded-lg bg-slate-50 p-3 text-sm">
                  {fatura.itens.length === 0 ? (
                    <p className="text-slate-500">Sem cobrancas no lote.</p>
                  ) : (
                    <ul className="space-y-1">
                      {fatura.itens.map((it) => (
                        <li key={it.id} className="flex items-center justify-between gap-2">
                          <span className="text-slate-700">
                            #{it.id} {it.descricao || it.origem} — {formatReais(it.valorCentavos)}{' '}
                            <Badge tone={STATUS_TONE[it.status]}>{it.status}</Badge>
                          </span>
                          {l.status === 'ABERTO' && (
                            <Button variant="danger" className="px-2 py-0.5" onClick={() => vincular(l.id, it.id, 'remover')}>Remover</Button>
                          )}
                        </li>
                      ))}
                    </ul>
                  )}

                  {l.status === 'ABERTO' && (
                    <div className="mt-3">
                      <p className="text-xs font-semibold uppercase tracking-wide text-slate-400">
                        Cobrancas autorizadas elegiveis
                      </p>
                      {elegiveis.length === 0 ? (
                        <p className="text-slate-400">Nenhuma cobranca elegivel deste convenio.</p>
                      ) : (
                        <ul className="mt-1 space-y-1">
                          {elegiveis.map((c) => (
                            <li key={c.id} className="flex items-center justify-between gap-2">
                              <span className="text-slate-700">
                                #{c.id} {c.descricao || c.origem} — {formatReais(c.valorCentavos)}
                              </span>
                              <Button variant="secondary" className="px-2 py-0.5" onClick={() => vincular(l.id, c.id, 'cobrancas')}>Adicionar</Button>
                            </li>
                          ))}
                        </ul>
                      )}
                    </div>
                  )}
                </div>
              )}
            </li>
          ))}
        </ul>
      )}
    </Card>
  )
}

export default function Financeiro() {
  const [cobrancas, setCobrancas] = useState(null)
  const [demonstrativo, setDemonstrativo] = useState(null)
  const [erro, setErro] = useState('')

  const carregar = useCallback(() => {
    setErro('')
    apiGet('/cobrancas').then(setCobrancas).catch((e) => setErro(e.message))
    apiGet('/cobrancas/demonstrativo').then(setDemonstrativo).catch((e) => setErro(e.message))
  }, [])

  useEffect(() => { queueMicrotask(carregar) }, [carregar])

  async function mudarStatus(c, destino) {
    // GLOSADA/CANCELADA exigem justificativa (o backend tambem barra sem ela).
    let motivo = ''
    if (destino === 'GLOSADA' || destino === 'CANCELADA') {
      const r = window.prompt(`Motivo para ${ACAO_LABEL[destino].toLowerCase()} a cobranca #${c.id}:`)
      if (r === null || !r.trim()) return
      motivo = r.trim()
    }
    try {
      await apiSend('POST', `/cobrancas/${c.id}/status`, { valor: destino, motivo })
      carregar()
    } catch (e) {
      setErro(e.message)
    }
  }

  return (
    <div className="space-y-6">
      <PageHeader
        title="Financeiro"
        subtitle="Convenios, cobrancas e demonstrativo de faturamento."
      />

      {erro && <Alert>{erro}</Alert>}

      <div className="grid grid-cols-2 gap-4 sm:grid-cols-4">
        <StatCard label="Recebido" value={demonstrativo ? formatReais(demonstrativo.recebidoCentavos) : null} />
        <StatCard label="Pendente" value={demonstrativo ? formatReais(demonstrativo.pendenteCentavos) : null} />
        <StatCard label="Glosado" value={demonstrativo ? formatReais(demonstrativo.glosadoCentavos) : null} />
        <StatCard label="Vencido" value={demonstrativo ? formatReais(demonstrativo.vencidoCentavos) : null} />
      </div>

      <Convenios onErro={setErro} />
      <NovaCobranca onCriada={carregar} onErro={setErro} />

      <section className="space-y-2">
        <h2 className="text-sm font-semibold text-slate-600">Cobrancas</h2>
        <ListaCobrancas cobrancas={cobrancas} onMudarStatus={mudarStatus} />
      </section>

      <Lotes cobrancas={cobrancas} onMudou={carregar} onErro={setErro} />
    </div>
  )
}
