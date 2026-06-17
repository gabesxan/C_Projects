import { useEffect, useState } from 'react'
import { apiGet } from '../api/client'
import DataTable from '../components/DataTable'
import { PageHeader, Spinner, Card, Badge } from '../components/ui'

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
    </div>
  )
}
