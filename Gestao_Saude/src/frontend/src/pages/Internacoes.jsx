import { useCallback, useEffect, useState } from 'react'
import { apiGet, apiSend } from '../api/client'
import { useAuth } from '../auth/AuthContext'
import { SearchSelect, ApiSelect } from '../components/FieldSelect'
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

function FormInternar({ onCriado }) {
  const [aberto, setAberto] = useState(false)
  const [v, setV] = useState({ paciente_id: '', ala_id: '', leito_id: '', data_entrada: '', responsavel: '' })
  const [erro, setErro] = useState('')
  const [salvando, setSalvando] = useState(false)

  function set(k, val) { setV((s) => ({ ...s, [k]: val })) }

  async function submit(e) {
    e.preventDefault()
    setErro('')
    setSalvando(true)
    try {
      await apiSend('POST', '/internacoes', v)
      setV({ paciente_id: '', ala_id: '', leito_id: '', data_entrada: '', responsavel: '' })
      setAberto(false)
      onCriado()
    } catch (err) {
      setErro(err.status === 400 ? 'Dados invalidos ou leito indisponivel.' : err.message)
    } finally {
      setSalvando(false)
    }
  }

  if (!aberto) return <Button onClick={() => setAberto(true)}>+ Internar paciente</Button>

  return (
    <Card className="p-5">
      <form onSubmit={submit} className="space-y-4">
        {erro && <Alert>{erro}</Alert>}
        <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
          <label className="text-sm text-slate-600">Paciente
            <SearchSelect
              value={v.paciente_id}
              onChange={(val) => set('paciente_id', val)}
              path="/pacientes/buscar"
              optionLabel={(p) => `${p.nome}${p.documento ? ` · ${p.documento}` : ''}`}
            />
          </label>
          <label className="text-sm text-slate-600">Ala
            <ApiSelect
              value={v.ala_id}
              onChange={(val) => setV((s) => ({ ...s, ala_id: val, leito_id: '' }))}
              path="/alas"
              optionLabel={(a) => a.nome}
            />
          </label>
          <label className="text-sm text-slate-600">Leito (disponivel)
            <ApiSelect
              value={v.leito_id}
              onChange={(val) => set('leito_id', val)}
              path="/leitos"
              placeholder={v.ala_id ? 'Selecione...' : 'Escolha a ala primeiro'}
              optionLabel={(l) => `Leito ${l.numero}`}
              filter={(l) => String(l.alaId) === String(v.ala_id) && l.status === 'DISPONIVEL'}
            />
          </label>
          <label className="text-sm text-slate-600">Data de entrada
            <input type="date" className={inputCls} value={v.data_entrada} onChange={(e) => set('data_entrada', e.target.value)} required />
          </label>
          <label className="text-sm text-slate-600">Responsavel
            <input className={inputCls} value={v.responsavel} onChange={(e) => set('responsavel', e.target.value)} required />
          </label>
        </div>
        <div className="flex gap-2">
          <Button type="submit" disabled={salvando}>{salvando ? 'Internando...' : 'Confirmar internacao'}</Button>
          <Button type="button" variant="secondary" onClick={() => setAberto(false)}>Cancelar</Button>
        </div>
      </form>
    </Card>
  )
}

export default function Internacoes() {
  const { user } = useAuth()
  const [rows, setRows] = useState(null)
  const [erro, setErro] = useState('')

  const podeInternar = user.papel === 'ADMIN' || user.papel === 'MEDICO'
  const podeAlta = user.papel === 'ADMIN' || user.papel === 'MEDICO'
  const podeTransferir = ['ADMIN', 'MEDICO', 'ENFERMAGEM'].includes(user.papel)

  const carregar = useCallback(() => {
    setRows(null)
    setErro('')
    apiGet('/internacoes').then(setRows).catch((e) => setErro(e.message))
  }, [])

  useEffect(() => { queueMicrotask(carregar) }, [carregar])

  async function darAlta(row) {
    const resumo = window.prompt('Resumo clinico da alta:')
    if (resumo === null) return
    const diagnostico = window.prompt('Diagnostico final:')
    if (diagnostico === null) return
    const orientacoes = window.prompt('Orientacoes ao paciente:') || ''
    const data = new Date().toISOString().slice(0, 10)
    try {
      await apiSend('POST', `/internacoes/${row.id}/alta`, { data, resumo, diagnostico, orientacoes })
      carregar()
    } catch (e) {
      setErro(e.status === 409 ? 'Alta requer resumo e diagnostico.' : e.message)
    }
  }

  async function transferir(row) {
    const leito_id = window.prompt('Transferir para o leito ID:')
    if (!leito_id) return
    const responsavel = window.prompt('Responsavel pela transferencia:') || ''
    const data = new Date().toISOString().slice(0, 10)
    try {
      await apiSend('POST', `/internacoes/${row.id}/transferir`, { leito_id, data, responsavel })
      carregar()
    } catch (e) {
      setErro(e.message)
    }
  }

  return (
    <div className="space-y-5">
      <PageHeader
        title="Internacoes"
        subtitle="Admissao, transferencia e alta — vinculadas ao leito."
        actions={podeInternar && <FormInternar onCriado={carregar} />}
      />

      {erro && <Alert>{erro}</Alert>}
      {!erro && rows === null && <Spinner />}
      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <EmptyState title="Nenhuma internacao" description="As internacoes aparecerao aqui." />
      )}

      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <div className="overflow-x-auto rounded-xl bg-white shadow-sm ring-1 ring-slate-200">
          <table className="min-w-full text-sm">
            <thead>
              <tr className="border-b border-slate-200 bg-slate-50 text-left text-slate-500">
                <th className="px-4 py-3 font-semibold">ID</th>
                <th className="px-4 py-3 font-semibold">Paciente</th>
                <th className="px-4 py-3 font-semibold">Ala/Leito</th>
                <th className="px-4 py-3 font-semibold">Entrada</th>
                <th className="px-4 py-3 font-semibold">Responsavel</th>
                <th className="px-4 py-3 font-semibold">Status</th>
                <th className="px-4 py-3 font-semibold">Acoes</th>
              </tr>
            </thead>
            <tbody>
              {rows.map((i) => (
                <tr key={i.id} className="border-b border-slate-100 last:border-0 hover:bg-slate-50/70">
                  <td className="px-4 py-3 text-slate-500">{i.id}</td>
                  <td className="px-4 py-3 text-slate-700">#{i.pacienteId}</td>
                  <td className="px-4 py-3 text-slate-600">{i.alaId} / {i.leitoId}</td>
                  <td className="px-4 py-3 text-slate-600">{i.dataEntrada}</td>
                  <td className="px-4 py-3 text-slate-600">{i.responsavel || '—'}</td>
                  <td className="px-4 py-3">
                    <Badge tone={i.status === 'INTERNADO' ? 'teal' : 'slate'}>{i.status}</Badge>
                  </td>
                  <td className="px-4 py-3">
                    {i.status === 'INTERNADO' ? (
                      <div className="flex gap-2">
                        {podeTransferir && (
                          <Button variant="secondary" className="px-3 py-1" onClick={() => transferir(i)}>Transferir</Button>
                        )}
                        {podeAlta && (
                          <Button className="px-3 py-1" onClick={() => darAlta(i)}>Alta</Button>
                        )}
                      </div>
                    ) : (
                      <span className="text-xs text-slate-400">Alta em {i.dataAlta || '—'}</span>
                    )}
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
