import { useEffect, useState } from 'react'
import { apiGet } from '../api/client'
import DataTable from '../components/DataTable'

// Secoes do paciente, todas servidas pelas rotas /me/... (escopadas ao proprio).
const SECOES = [
  {
    titulo: 'Minhas receitas',
    path: '/me/receitas',
    columns: [
      { key: 'medicamento', label: 'Medicamento' },
      { key: 'dosagem', label: 'Dosagem' },
      { key: 'frequencia', label: 'Frequencia' },
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
      <h2 className="text-sm font-medium text-slate-500">{titulo}</h2>
      {erro && <p className="text-sm text-red-600">{erro}</p>}
      {!erro && rows === null && (
        <p className="text-sm text-slate-400">Carregando...</p>
      )}
      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <p className="text-sm text-slate-500">Nenhum registro.</p>
      )}
      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <DataTable columns={columns} rows={rows} />
      )}
    </section>
  )
}

export default function MinhaSaude() {
  return (
    <div className="space-y-8">
      <h1 className="text-xl font-semibold text-slate-800">Meus dados</h1>
      {SECOES.map((s) => (
        <Secao key={s.path} {...s} />
      ))}
    </div>
  )
}
