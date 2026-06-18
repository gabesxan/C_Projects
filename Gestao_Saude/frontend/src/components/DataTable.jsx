import { Badge, StatusBadge, Button } from './ui'

// Tabela generica: recebe as colunas (config) e as linhas (JSON da API).
// Tipos de coluna suportados:
//   - 'badge'  -> renderiza o valor como badge (tom opcional via col.tone(v))
//   - 'status' -> renderiza um badge Ativo/Inativo a partir de booleano/0-1
// Se onDelete for fornecido, adiciona uma coluna de acao com o botao de remover.
function formatCell(value) {
  if (value === true) return 'Sim'
  if (value === false) return 'Nao'
  if (value === null || value === undefined || value === '') return '—'
  return String(value)
}

function Cell({ col, row }) {
  const value = row[col.key]

  if (col.type === 'status') {
    return <StatusBadge ativo={value} />
  }

  if (col.type === 'badge') {
    if (value === null || value === undefined || value === '') return '—'
    return <Badge tone={col.tone ? col.tone(value) : 'slate'}>{value}</Badge>
  }

  return formatCell(value)
}

export default function DataTable({
  columns,
  rows,
  onDelete,
  onRowClick,
  onAction,
  actionLabel = 'Acao',
  deleteLabel = 'Remover',
}) {
  return (
    <div className="overflow-x-auto rounded-xl bg-white shadow-sm ring-1 ring-slate-200">
      <table className="min-w-full text-sm">
        <thead>
          <tr className="border-b border-slate-200 bg-slate-50 text-left text-slate-500">
            {columns.map((col) => (
              <th key={col.key} className="px-4 py-3 font-semibold whitespace-nowrap">
                {col.label}
              </th>
            ))}
            {(onDelete || onAction) && <th className="px-4 py-3 font-semibold">Acoes</th>}
          </tr>
        </thead>
        <tbody>
          {rows.map((row, i) => (
            <tr
              key={row.id ?? i}
              onClick={onRowClick ? () => onRowClick(row) : undefined}
              className={[
                'border-b border-slate-100 last:border-0 hover:bg-slate-50/70',
                onRowClick ? 'cursor-pointer' : '',
              ].join(' ')}
            >
              {columns.map((col) => (
                <td key={col.key} className="px-4 py-3 text-slate-700 whitespace-nowrap">
                  <Cell col={col} row={row} />
                </td>
              ))}
              {(onDelete || onAction) && (
                <td className="px-4 py-3 whitespace-nowrap">
                  <div className="flex gap-2">
                    {onAction && (
                      <Button
                        variant="secondary"
                        className="px-3 py-1"
                        onClick={(e) => {
                          e.stopPropagation()
                          onAction(row)
                        }}
                      >
                        {actionLabel}
                      </Button>
                    )}
                    {onDelete && (
                      <Button
                        variant="danger"
                        className="px-3 py-1"
                        onClick={(e) => {
                          e.stopPropagation()
                          onDelete(row)
                        }}
                      >
                        {deleteLabel}
                      </Button>
                    )}
                  </div>
                </td>
              )}
            </tr>
          ))}
        </tbody>
      </table>
    </div>
  )
}
