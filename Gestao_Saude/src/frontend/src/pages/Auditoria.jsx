import { useEffect, useState } from 'react'
import { apiGet } from '../api/client'
import { PageHeader, Alert, Spinner, Badge, EmptyState } from '../components/ui'

// Tom de cor por tipo de acao auditada.
const ACAO_TONE = {
  LOGIN: 'sky',
  CRIAR: 'teal',
  PRESCREVER: 'teal',
  ALTA: 'violet',
  DESATIVAR: 'amber',
  SUSPENDER: 'amber',
  CANCELAR: 'red',
  REATIVAR: 'green',
}

export default function Auditoria() {
  const [rows, setRows] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    apiGet('/auditoria')
      .then(setRows)
      .catch((e) => setErro(e.status === 403 ? 'Acesso restrito a administradores.' : e.message))
  }, [])

  return (
    <div className="space-y-5">
      <PageHeader
        title="Auditoria"
        subtitle="Trilha das acoes sensiveis: login, alta, prescricao, cadastro e mudancas de status."
      />

      {erro && <Alert>{erro}</Alert>}
      {!erro && rows === null && <Spinner />}
      {!erro && Array.isArray(rows) && rows.length === 0 && (
        <EmptyState title="Sem registros" description="Nenhuma acao auditavel foi registrada ainda." />
      )}

      {!erro && Array.isArray(rows) && rows.length > 0 && (
        <div className="overflow-x-auto rounded-xl bg-white shadow-sm ring-1 ring-slate-200">
          <table className="min-w-full text-sm">
            <thead>
              <tr className="border-b border-slate-200 bg-slate-50 text-left text-slate-500">
                <th className="px-4 py-3 font-semibold">Quando</th>
                <th className="px-4 py-3 font-semibold">Usuario</th>
                <th className="px-4 py-3 font-semibold">Acao</th>
                <th className="px-4 py-3 font-semibold">Entidade</th>
                <th className="px-4 py-3 font-semibold">Detalhe</th>
              </tr>
            </thead>
            <tbody>
              {rows.map((r) => (
                <tr key={r.id} className="border-b border-slate-100 last:border-0 hover:bg-slate-50/70">
                  <td className="px-4 py-3 text-slate-500 whitespace-nowrap">{r.criadoEm}</td>
                  <td className="px-4 py-3 text-slate-700">{r.usuarioLogin || `#${r.usuarioId}`}</td>
                  <td className="px-4 py-3">
                    <Badge tone={ACAO_TONE[r.acao] ?? 'slate'}>{r.acao}</Badge>
                  </td>
                  <td className="px-4 py-3 text-slate-600 whitespace-nowrap">
                    {r.entidade}{r.entidadeId ? ` #${r.entidadeId}` : ''}
                  </td>
                  <td className="px-4 py-3 text-slate-500">{r.detalhe || '—'}</td>
                </tr>
              ))}
            </tbody>
          </table>
        </div>
      )}
    </div>
  )
}
