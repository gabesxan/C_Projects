import { useCallback, useEffect, useState } from 'react'
import { apiGet, apiSend } from '../api/client'
import { formatReais, parseReaisParaCentavos } from '../money'
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

  const carregar = useCallback(() => {
    apiGet('/convenios').then(setConvenios).catch((e) => onErro(e.message))
  }, [onErro])

  useEffect(() => { carregar() }, [carregar])

  async function criar(e) {
    e.preventDefault()
    if (!nome.trim()) return
    try {
      await apiSend('POST', '/convenios', { nome })
      setNome('')
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
              <span className="text-sm text-slate-700">{c.nome}</span>
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
  const vazio = { paciente_id: '', forma: 'PARTICULAR', convenio_id: '', origem: '', descricao: '', valor: '' }
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
          Paciente ID
          <input className={inputCls} type="number" value={v.paciente_id} onChange={(e) => set('paciente_id', e.target.value)} />
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
              <td className="px-4 py-3 text-slate-700">{c.origem || '—'}</td>
              <td className="px-4 py-3 font-semibold text-slate-900">{formatReais(c.valorCentavos)}</td>
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

export default function Financeiro() {
  const [cobrancas, setCobrancas] = useState(null)
  const [demonstrativo, setDemonstrativo] = useState(null)
  const [erro, setErro] = useState('')

  const carregar = useCallback(() => {
    setErro('')
    apiGet('/cobrancas').then(setCobrancas).catch((e) => setErro(e.message))
    apiGet('/cobrancas/demonstrativo').then(setDemonstrativo).catch((e) => setErro(e.message))
  }, [])

  useEffect(() => { carregar() }, [carregar])

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

      <div className="grid grid-cols-1 gap-4 sm:grid-cols-3">
        <StatCard label="Recebido" value={demonstrativo ? formatReais(demonstrativo.recebidoCentavos) : null} />
        <StatCard label="Pendente" value={demonstrativo ? formatReais(demonstrativo.pendenteCentavos) : null} />
        <StatCard label="Glosado" value={demonstrativo ? formatReais(demonstrativo.glosadoCentavos) : null} />
      </div>

      <Convenios onErro={setErro} />
      <NovaCobranca onCriada={carregar} onErro={setErro} />

      <section className="space-y-2">
        <h2 className="text-sm font-semibold text-slate-600">Cobrancas</h2>
        <ListaCobrancas cobrancas={cobrancas} onMudarStatus={mudarStatus} />
      </section>
    </div>
  )
}
