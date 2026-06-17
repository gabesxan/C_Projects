import { useCallback, useEffect, useState } from 'react'
import { apiGet, apiSend } from '../api/client'
import { useAuth } from '../auth/AuthContext'
import {
  PageHeader,
  Card,
  StatCard,
  Badge,
  Alert,
  Spinner,
  EmptyState,
} from '../components/ui'

const STATUS = ['DISPONIVEL', 'HIGIENIZACAO', 'MANUTENCAO', 'BLOQUEADO']
const TONE = {
  DISPONIVEL: 'green',
  OCUPADO: 'red',
  HIGIENIZACAO: 'amber',
  MANUTENCAO: 'sky',
  BLOQUEADO: 'slate',
}

export default function Leitos() {
  const { user } = useAuth()
  const [leitos, setLeitos] = useState(null)
  const [ocupacao, setOcupacao] = useState(null)
  const [erro, setErro] = useState('')

  const podeGerenciar = user.papel === 'ADMIN' || user.papel === 'ENFERMAGEM'

  const carregar = useCallback(() => {
    setErro('')
    apiGet('/leitos').then(setLeitos).catch((e) => setErro(e.message))
    apiGet('/leitos/ocupacao').then(setOcupacao).catch(() => {})
  }, [])

  useEffect(() => { carregar() }, [carregar])

  async function alterar(leito, valor) {
    if (!valor) return
    try {
      await apiSend('POST', `/leitos/${leito.id}/status`, { valor })
      carregar()
    } catch (e) {
      setErro(e.message)
    }
  }

  return (
    <div className="space-y-6">
      <PageHeader
        title="Leitos"
        subtitle="Mapa de leitos, ocupacao e status (higienizacao, manutencao, bloqueio)."
      />

      {erro && <Alert>{erro}</Alert>}

      {ocupacao && (
        <div className="grid grid-cols-2 gap-4 md:grid-cols-4">
          <StatCard label="Taxa de ocupacao" value={`${ocupacao.taxaOcupacao}%`} />
          <StatCard label="Ocupados" value={ocupacao.ocupados} />
          <StatCard label="Disponiveis" value={ocupacao.disponiveis} />
          <StatCard label="Higienizacao" value={ocupacao.higienizacao} />
        </div>
      )}

      {!erro && leitos === null && <Spinner />}
      {!erro && Array.isArray(leitos) && leitos.length === 0 && (
        <EmptyState title="Nenhum leito" description="Cadastre leitos para gerenciar a ocupacao." />
      )}

      {!erro && Array.isArray(leitos) && leitos.length > 0 && (
        <div className="grid grid-cols-2 gap-4 sm:grid-cols-3 lg:grid-cols-4">
          {leitos.map((l) => (
            <Card key={l.id} className="p-4">
              <div className="flex items-center justify-between">
                <span className="text-lg font-bold text-slate-900">#{l.numero}</span>
                <Badge tone={TONE[l.status]}>{l.status}</Badge>
              </div>
              <p className="mt-1 text-xs text-slate-500">Ala {l.alaId}</p>
              {l.pacienteId > 0 && (
                <p className="mt-1 text-xs text-slate-500">Paciente #{l.pacienteId}</p>
              )}
              {podeGerenciar && l.status !== 'OCUPADO' && (
                <select
                  className="mt-3 block w-full rounded-lg border border-slate-300 px-2 py-1 text-xs"
                  value=""
                  onChange={(e) => alterar(l, e.target.value)}
                >
                  <option value="">Alterar status...</option>
                  {STATUS.filter((s) => s !== l.status).map((s) => (
                    <option key={s} value={s}>{s}</option>
                  ))}
                </select>
              )}
            </Card>
          ))}
        </div>
      )}
    </div>
  )
}
