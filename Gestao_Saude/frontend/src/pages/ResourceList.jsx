import { useEffect, useState } from 'react'
import { useParams } from 'react-router-dom'
import { apiGet } from '../api/client'
import { resourceByKey } from '../resources'
import DataTable from '../components/DataTable'

export default function ResourceList() {
  const { key } = useParams()
  const recurso = resourceByKey(key)
  const [rows, setRows] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    if (!recurso) return
    setRows(null)
    setErro('')
    apiGet(recurso.path)
      .then(setRows)
      .catch((e) => {
        if (e.status === 403) setErro('Voce nao tem acesso a este recurso.')
        else setErro(e.message)
      })
  }, [recurso])

  if (!recurso) {
    return <p className="text-slate-500">Recurso desconhecido.</p>
  }

  return (
    <div className="space-y-4">
      <div className="flex items-baseline justify-between">
        <h2 className="text-xl font-semibold text-slate-800">{recurso.label}</h2>
        {Array.isArray(rows) && (
          <span className="text-sm text-slate-500">{rows.length} registro(s)</span>
        )}
      </div>

      {erro && (
        <div className="rounded-lg bg-red-50 text-red-700 text-sm px-4 py-3">
          {erro}
        </div>
      )}

      {!erro && rows === null && (
        <p className="text-sm text-slate-400">Carregando...</p>
      )}

      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <p className="text-sm text-slate-500">Nenhum registro.</p>
      )}

      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <DataTable columns={recurso.columns} rows={rows} />
      )}
    </div>
  )
}
