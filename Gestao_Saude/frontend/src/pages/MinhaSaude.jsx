import { useEffect, useState } from 'react'
import { apiGet } from '../api/client'
import { formatReais } from '../money'
import DataTable from '../components/DataTable'
import { PageHeader, Spinner, Card, Badge } from '../components/ui'

const STATUS_COBRANCA_TONE = {
  PENDENTE: 'amber',
  AUTORIZADA: 'sky',
  PAGA: 'green',
  GLOSADA: 'red',
  CANCELADA: 'slate',
}

// Cobrancas do proprio paciente: valor formatado em R$ e status como badge.
function MinhasCobrancas() {
  const [rows, setRows] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    apiGet('/me/cobrancas').then(setRows).catch((e) => setErro(e.message))
  }, [])

  return (
    <section className="space-y-2">
      <h2 className="text-sm font-semibold text-slate-600">Minhas cobrancas</h2>
      {erro && <p className="text-sm text-red-600">{erro}</p>}
      {!erro && rows === null && <Spinner />}
      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <p className="text-sm text-slate-500">Nenhuma cobranca.</p>
      )}
      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <ul className="divide-y divide-slate-100 rounded-xl bg-white shadow-sm ring-1 ring-slate-200">
          {rows.map((c) => (
            <li key={c.id} className="flex items-center justify-between px-4 py-3">
              <div>
                <p className="text-sm font-medium text-slate-800">
                  {c.descricao || c.origem || `Cobranca #${c.id}`}
                </p>
                <p className="text-xs text-slate-400">
                  {c.forma}
                  {c.vencimento ? ` • vence ${c.vencimento}` : ''}
                  {c.copartCentavos > 0 ? ` • coparticipacao ${formatReais(c.copartCentavos)}` : ''}
                </p>
              </div>
              <div className="flex items-center gap-3">
                {c.vencida && <Badge tone="red">Vencida</Badge>}
                <span className="text-sm font-semibold text-slate-900">{formatReais(c.valorCentavos)}</span>
                <Badge tone={STATUS_COBRANCA_TONE[c.status]}>{c.status}</Badge>
              </div>
            </li>
          ))}
        </ul>
      )}
    </section>
  )
}

// Secoes do paciente, todas servidas pelas rotas /me/... (escopadas ao proprio).
const SECOES = [
  {
    titulo: 'Meus agendamentos',
    path: '/me/agendamentos',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'medicoId', label: 'Medico' },
      { key: 'data', label: 'Data' },
      { key: 'horario', label: 'Horario' },
      { key: 'status', label: 'Status', type: 'badge' },
    ],
  },
  {
    titulo: 'Minhas receitas',
    path: '/me/receitas',
    columns: [
      { key: 'medicamento', label: 'Medicamento' },
      { key: 'dosagem', label: 'Dosagem' },
      { key: 'frequencia', label: 'Frequencia' },
      { key: 'via', label: 'Via' },
      { key: 'duracao', label: 'Duracao' },
      { key: 'observacoes', label: 'Observacoes' },
    ],
  },
  {
    titulo: 'Meus exames',
    path: '/me/exames',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'tipoExame', label: 'Tipo' },
      { key: 'dataSolicitacao', label: 'Solicitacao' },
      { key: 'status', label: 'Status' },
      { key: 'resultado', label: 'Resultado' },
    ],
  },
  {
    titulo: 'Meus prontuarios',
    path: '/me/prontuarios',
    columns: [
      { key: 'id', label: 'ID' },
      { key: 'data', label: 'Data' },
      { key: 'diagnostico', label: 'Diagnostico' },
      { key: 'conduta', label: 'Conduta' },
    ],
  },
]

function Secao({ titulo, path, columns }) {
  const [rows, setRows] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    apiGet(path)
      .then(setRows)
      .catch((e) => setErro(e.message))
  }, [path])

  return (
    <section className="space-y-2">
      <h2 className="text-sm font-semibold text-slate-600">{titulo}</h2>
      {erro && <p className="text-sm text-red-600">{erro}</p>}
      {!erro && rows === null && <Spinner />}
      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <p className="text-sm text-slate-500">Nenhum registro.</p>
      )}
      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <DataTable columns={columns} rows={rows} />
      )}
    </section>
  )
}

function Perfil() {
  const [p, setP] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    apiGet('/me/perfil').then(setP).catch((e) => setErro(e.message))
  }, [])

  if (erro) return null
  if (!p) return <Spinner />

  return (
    <Card className="p-5">
      <div className="flex items-center justify-between">
        <div>
          <p className="text-lg font-bold text-slate-900">{p.nome}</p>
          <p className="text-sm text-slate-500">
            {p.idade} anos • {p.tipoDocumento} {p.documento} • {p.telefone}
          </p>
        </div>
        {p.alergias
          ? <Badge tone="red">⚠️ {p.alergias}</Badge>
          : <Badge tone="green">Sem alergias</Badge>}
      </div>
    </Card>
  )
}

export default function MinhaSaude() {
  return (
    <div className="space-y-6">
      <PageHeader
        title="Meus dados"
        subtitle="Seu perfil, agendamentos, receitas, exames e prontuarios — so voce ve."
      />
      <Perfil />
      {SECOES.map((s) => (
        <Secao key={s.path} {...s} />
      ))}
      <MinhasCobrancas />
    </div>
  )
}
