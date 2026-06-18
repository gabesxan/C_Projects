import { useCallback, useEffect, useState } from 'react'
import { apiGet, apiSend } from '../api/client'
import { useAuth } from '../auth/AuthContext'
import {
  PageHeader,
  Card,
  Button,
  Alert,
  Spinner,
  Badge,
  EmptyState,
} from '../components/ui'

const inputCls =
  'mt-1 block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30 outline-none'

const STATUS_TONE = { AGUARDANDO: 'amber', EM_ATENDIMENTO: 'teal', ENCERRADO: 'slate' }

// Confirmacao de chegada: busca o paciente e gera a senha de fila.
function CheckinForm({ onFeito }) {
  const [q, setQ] = useState('')
  const [resultados, setResultados] = useState(null)
  const [destino, setDestino] = useState('TRIAGEM')
  const [erro, setErro] = useState('')
  const [ultima, setUltima] = useState(null)

  async function buscar(e) {
    e.preventDefault()
    setErro('')
    setResultados(null)
    try {
      setResultados(await apiGet(`/pacientes/buscar?q=${encodeURIComponent(q)}`))
    } catch (err) {
      setErro(err.message)
    }
  }

  async function checkin(p) {
    setErro('')
    try {
      const r = await apiSend('POST', '/checkins', { paciente_id: p.id, destino })
      setUltima({ nome: p.nome, senha: r.senha, destino: r.destino })
      setResultados(null)
      setQ('')
      onFeito()
    } catch (err) {
      setErro(err.message)
    }
  }

  // Atualiza um dado cadastral basico (telefone) na recepcao.
  async function atualizarTel(p) {
    const telefone = window.prompt(`Novo telefone de ${p.nome}:`, p.telefone || '')
    if (telefone === null || !telefone.trim()) return
    try {
      await apiSend('POST', `/pacientes/${p.id}/contato`, { telefone })
      window.alert('Contato atualizado.')
    } catch (err) {
      setErro(err.message)
    }
  }

  return (
    <Card className="p-5">
      <p className="text-sm font-semibold text-slate-700">Confirmar chegada</p>
      {erro && <div className="mt-3"><Alert>{erro}</Alert></div>}
      {ultima && (
        <div className="mt-3">
          <Alert tone="teal">
            Senha <strong>{ultima.senha}</strong> gerada para <strong>{ultima.nome}</strong> — direcionado a {ultima.destino}.
          </Alert>
        </div>
      )}
      <form onSubmit={buscar} className="mt-3 flex flex-wrap items-end gap-2">
        <label className="text-sm text-slate-600">
          Paciente (nome ou documento)
          <input className={inputCls} value={q} onChange={(e) => setQ(e.target.value)} />
        </label>
        <label className="text-sm text-slate-600">
          Destino
          <select className={inputCls} value={destino} onChange={(e) => setDestino(e.target.value)}>
            <option value="TRIAGEM">Triagem</option>
            <option value="CONSULTA">Consulta</option>
          </select>
        </label>
        <Button type="submit">Buscar</Button>
      </form>

      {Array.isArray(resultados) && resultados.length === 0 && (
        <p className="mt-3 text-sm text-slate-500">Nenhum paciente encontrado.</p>
      )}
      {Array.isArray(resultados) && resultados.length > 0 && (
        <ul className="mt-3 divide-y divide-slate-100">
          {resultados.map((p) => (
            <li key={p.id} className="flex items-center justify-between py-2">
              <span className="text-sm text-slate-700">
                {p.nome} <span className="text-slate-400">• {p.tipoDocumento} {p.documento} • {p.telefone}</span>
              </span>
              <div className="flex gap-2">
                <Button variant="secondary" onClick={() => atualizarTel(p)}>Atualizar tel.</Button>
                <Button onClick={() => checkin(p)}>Gerar senha</Button>
              </div>
            </li>
          ))}
        </ul>
      )}
    </Card>
  )
}

export default function Recepcao() {
  const { user } = useAuth()
  const [fila, setFila] = useState(null)
  const [erro, setErro] = useState('')

  const podeCheckin = user.papel === 'ADMIN' || user.papel === 'CADASTRO'

  const carregar = useCallback(() => {
    setErro('')
    apiGet('/checkins').then(setFila).catch((e) => setErro(e.message))
  }, [])

  useEffect(() => { carregar() }, [carregar])

  async function acao(id, qual) {
    try {
      await apiSend('POST', `/checkins/${id}/${qual}`)
      carregar()
    } catch (e) {
      setErro(e.message)
    }
  }

  return (
    <div className="space-y-6">
      <PageHeader
        title="Recepcao"
        subtitle="Confirme a chegada, gere a senha e acompanhe a fila de atendimento."
      />

      {podeCheckin && <CheckinForm onFeito={carregar} />}

      {erro && <Alert>{erro}</Alert>}
      {!erro && fila === null && <Spinner />}
      {!erro && Array.isArray(fila) && fila.length === 0 && (
        <EmptyState title="Fila vazia" description="Sem pacientes aguardando no momento." />
      )}

      {!erro && Array.isArray(fila) && fila.length > 0 && (
        <div className="overflow-x-auto rounded-xl bg-white shadow-sm ring-1 ring-slate-200">
          <table className="min-w-full text-sm">
            <thead>
              <tr className="border-b border-slate-200 bg-slate-50 text-left text-slate-500">
                <th className="px-4 py-3 font-semibold">Senha</th>
                <th className="px-4 py-3 font-semibold">Paciente</th>
                <th className="px-4 py-3 font-semibold">Destino</th>
                <th className="px-4 py-3 font-semibold">Status</th>
                <th className="px-4 py-3 font-semibold">Acoes</th>
              </tr>
            </thead>
            <tbody>
              {fila.map((c) => (
                <tr key={c.id} className="border-b border-slate-100 last:border-0 hover:bg-slate-50/70">
                  <td className="px-4 py-3 font-bold text-slate-900">{c.senha}</td>
                  <td className="px-4 py-3 text-slate-700">{c.pacienteNome || `#${c.pacienteId}`}</td>
                  <td className="px-4 py-3">
                    <Badge tone="sky">{c.destino}</Badge>
                    {c.prioridade >= 4 && <span className="ml-1"><Badge tone="red">prioridade {c.prioridade}</Badge></span>}
                  </td>
                  <td className="px-4 py-3"><Badge tone={STATUS_TONE[c.status]}>{c.status}</Badge></td>
                  <td className="px-4 py-3">
                    <div className="flex gap-2">
                      {c.status === 'AGUARDANDO' && (
                        <Button variant="secondary" className="px-3 py-1" onClick={() => acao(c.id, 'chamar')}>Chamar</Button>
                      )}
                      <Button variant="danger" className="px-3 py-1" onClick={() => acao(c.id, 'encerrar')}>Encerrar</Button>
                    </div>
                  </td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
  )
}
