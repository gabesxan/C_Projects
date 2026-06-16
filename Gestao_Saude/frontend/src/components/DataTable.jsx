// Tabela generica: recebe as colunas (config) e as linhas (JSON da API).
// Se onDelete for fornecido, adiciona uma coluna de acao com o botao de remover.
function formatCell(value) {
  if (value === true) return 'Sim'
  if (value === false) return 'Nao'
  if (value === null || value === undefined || value === '') return '—'
  return String(value)
}

export default function DataTable({ columns, rows, onDelete, deleteLabel = 'Remover' }) {
  return (
    <div className="overflow-x-auto bg-white rounded-xl shadow">
      <table className="min-w-full text-sm">
        <thead>
          <tr className="border-b border-slate-200 text-left text-slate-500">
            {columns.map((col) => (
              <th key={col.key} className="px-4 py-3 font-medium whitespace-nowrap">
                {col.label}
              </th>
            ))}
            {onDelete && <th className="px-4 py-3 font-medium">Acoes</th>}
          </tr>
        </thead>
        <tbody>
          {rows.map((row, i) => (
            <tr
              key={row.id ?? i}
              className="border-b border-slate-100 last:border-0 hover:bg-slate-50"
            >
              {columns.map((col) => (
                <td key={col.key} className="px-4 py-3 text-slate-700 whitespace-nowrap">
                  {formatCell(row[col.key])}
                </td>
              ))}
              {onDelete && (
                <td className="px-4 py-3 whitespace-nowrap">
                  <button
                    onClick={() => onDelete(row)}
                    className="text-sm rounded-lg border border-red-200 text-red-700 px-3 py-1 hover:bg-red-50"
                  >
                    {deleteLabel}
                  </button>
                </td>
              )}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )
}
