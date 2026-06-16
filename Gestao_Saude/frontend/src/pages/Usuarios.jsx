import { useCallback, useEffect, useState } from 'react'
import { apiGet, apiSend } from '../api/client'
import {
  PageHeader,
  Card,
  Button,
  Alert,
  Spinner,
  Badge,
  StatusBadge,
  EmptyState,
  papelLabel,
  PAPEL_INFO,
} from '../components/ui'

const PAPEIS = ['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM', 'PACIENTE']

function estadoInicial() {
  return { nome: '', login: '', senha: '', papel: 'CADASTRO', paciente_id: '', medico_id: '' }
}

function FormUsuario({ onCreated }) {
  const [aberto, setAberto] = useState(false)
  const [v, setV] = useState(estadoInicial)
  const [erro, setErro] = useState('')
  const [salvando, setSalvando] = useState(false)

  function set(name, value) {
    setV((s) => ({ ...s, [name]: value }))
  }

  async function submit(e) {
    e.preventDefault()
    setErro('')
    setSalvando(true)
    try {
      // So envia o vinculo relevante ao papel escolhido.
      const params = { nome: v.nome, login: v.login, senha: v.senha, papel: v.papel }
      if (v.papel === 'MEDICO') params.medico_id = v.medico_id || '0'
      if (v.papel === 'PACIENTE') params.paciente_id = v.paciente_id || '0'
      await apiSend('POST', '/usuarios', params)
      setV(estadoInicial())
      setAberto(false)
      onCreated()
    } catch (err) {
      setErro(err.status === 400 ? 'Dados invalidos (login ja existe?).' : err.message)
    } finally {
      setSalvando(false)
    }
  }

  if (!aberto) {
    return <Button onClick={() => setAberto(true)}>+ Novo usuario</Button>
  }

  const inputCls =
    'mt-1 block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm focus:border-teal-500 focus:ring-2 focus:ring-teal-500/30 outline-none'

  return (
    <Card className="p-5">
      <form onSubmit={submit} className="space-y-4">
        {erro && <Alert>{erro}</Alert>}
        <div className="grid gap-4 sm:grid-cols-2 lg:grid-cols-3">
          <label className="text-sm text-slate-600">
            Nome
            <input className={inputCls} value={v.nome} onChange={(e) => set('nome', e.target.value)} required />
          </label>
          <label className="text-sm text-slate-600">
            Login
            <input className={inputCls} value={v.login} onChange={(e) => set('login', e.target.value)} required />
          </label>
          <label className="text-sm text-slate-600">
            Senha
            <input type="password" className={inputCls} value={v.senha} onChange={(e) => set('senha', e.target.value)} required />
          </label>
          <label className="text-sm text-slate-600">
            Papel
            <select className={inputCls} value={v.papel} onChange={(e) => set('papel', e.target.value)}>
              {PAPEIS.map((p) => (
                <option key={p} value={p}>{papelLabel(p)}</option>
              ))}
            </select>
          </label>
          {v.papel === 'MEDICO' && (
            <label className="text-sm text-slate-600">
              Medico ID (vinculo)
              <input type="number" className={inputCls} value={v.medico_id} onChange={(e) => set('medico_id', e.target.value)} />
            </label>
          )}
          {v.papel === 'PACIENTE' && (
            <label className="text-sm text-slate-600">
              Paciente ID (vinculo)
              <input type="number" className={inputCls} value={v.paciente_id} onChange={(e) => set('paciente_id', e.target.value)} />
            </label>
          )}
        </div>
        <div className="flex gap-2">
          <Button type="submit" disabled={salvando}>{salvando ? 'Salvando...' : 'Salvar'}</Button>
          <Button type="button" variant="secondary" onClick={() => setAberto(false)}>Cancelar</Button>
        </div>
      </form>
    </Card>
  )
}

export default function Usuarios() {
  const [rows, setRows] = useState(null)
  const [erro, setErro] = useState('')

  const carregar = useCallback(() => {
    setRows(null)
    setErro('')
    apiGet('/usuarios')
      .then(setRows)
      .catch((e) => setErro(e.status === 403 ? 'Acesso restrito a administradores.' : e.message))
  }, [])

  useEffect(() => { carregar() }, [carregar])

  async function alternarStatus(u) {
    const acao = u.ativo ? 'inativar' : 'reativar'
    if (!window.confirm(`Confirmar ${acao} o usuario "${u.login}"?`)) return
    try {
      if (u.ativo) await apiSend('DELETE', `/usuarios/${u.id}`)
      else await apiSend('POST', `/usuarios/${u.id}/reativar`)
      carregar()
    } catch (e) {
      setErro(e.message)
    }
  }

  return (
    <div className="space-y-5">
      <PageHeader
        title="Usuarios"
        subtitle="Crie acessos individuais e controle o status de cada usuario."
        actions={<FormUsuario onCreated={carregar} />}
      />

      {erro && <Alert>{erro}</Alert>}
      {!erro && rows === null && <Spinner />}
      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <EmptyState title="Nenhum usuario" description="Crie o primeiro acesso com o botao acima." />
      )}

      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <div className="overflow-x-auto rounded-xl bg-white shadow-sm ring-1 ring-slate-200">
          <table className="min-w-full text-sm">
            <thead>
              <tr className="border-b border-slate-200 bg-slate-50 text-left text-slate-500">
                <th className="px-4 py-3 font-semibold">ID</th>
                <th className="px-4 py-3 font-semibold">Nome</th>
                <th className="px-4 py-3 font-semibold">Login</th>
                <th className="px-4 py-3 font-semibold">Papel</th>
                <th className="px-4 py-3 font-semibold">Vinculo</th>
                <th className="px-4 py-3 font-semibold">Status</th>
                <th className="px-4 py-3 font-semibold">Criado em</th>
                <th className="px-4 py-3 font-semibold">Acoes</th>
              </tr>
            </thead>
            <tbody>
              {rows.map((u) => (
                <tr key={u.id} className="border-b border-slate-100 last:border-0 hover:bg-slate-50/70">
                  <td className="px-4 py-3 text-slate-500">{u.id}</td>
                  <td className="px-4 py-3 font-medium text-slate-800">{u.nome || '—'}</td>
                  <td className="px-4 py-3 text-slate-700">{u.login}</td>
                  <td className="px-4 py-3">
                    <Badge tone={PAPEL_INFO[u.papel]?.tone}>{papelLabel(u.papel)}</Badge>
                  </td>
                  <td className="px-4 py-3 text-slate-500">
                    {u.medicoId ? `medico #${u.medicoId}` : u.pacienteId ? `paciente #${u.pacienteId}` : '—'}
                  </td>
                  <td className="px-4 py-3"><StatusBadge ativo={u.ativo} /></td>
                  <td className="px-4 py-3 text-slate-500 whitespace-nowrap">{u.criadoEm}</td>
                  <td className="px-4 py-3">
                    <Button
                      variant={u.ativo ? 'danger' : 'secondary'}
                      className="px-3 py-1"
                      onClick={() => alternarStatus(u)}
                    >
                      {u.ativo ? 'Inativar' : 'Reativar'}
                    </Button>
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
