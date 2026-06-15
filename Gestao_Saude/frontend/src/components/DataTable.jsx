// Tabela generica: recebe as colunas (config) e as linhas (JSON da API).
function formatCell(value) {
  if (value === true) return 'Sim'
  if (value === false) return 'Nao'
  if (value === null || value === undefined || value === '') return '—'
  return String(value)
}

export default function DataTable({ columns, rows }) {
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
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )
}
