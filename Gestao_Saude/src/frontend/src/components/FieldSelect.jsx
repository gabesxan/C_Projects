import { useEffect, useState } from 'react'
import { apiGet } from '../api/client'

const campoCls =
  'mt-1 block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm disabled:opacity-60'

// Combobox de busca: digita um termo, consulta a API (searchPath aceita ?q=) e
// escolhe pelo NOME, guardando o id. Usado para entidades amplas (pacientes),
// onde uma fonte nao escopada (/pacientes/buscar) evita esconder registros.
export function SearchSelect({ path, value, onChange, optionLabel, placeholder = 'Buscar por nome ou documento...' }) {
  const [q, setQ] = useState('')
  const [resultados, setResultados] = useState(null)
  const [aberto, setAberto] = useState(false)
  const [rotulo, setRotulo] = useState('')

  // Observacao: o modo "selecionado" so e exibido quando ha value E rotulo;
  // se o value for limpo externamente, a UI volta sozinha ao campo de busca
  // (sem precisar de efeito para zerar o rotulo).

  useEffect(() => {
    if (!aberto) return undefined
    const t = setTimeout(() => {
      apiGet(`${path}?q=${encodeURIComponent(q)}`)
        .then((rows) => setResultados(Array.isArray(rows) ? rows : []))
        .catch(() => setResultados([]))
    }, 250)
    return () => clearTimeout(t)
  }, [q, aberto, path])

  function escolher(r) {
    onChange(String(r.id))
    setRotulo(optionLabel ? optionLabel(r) : r.nome)
    setAberto(false)
  }

  if (value && rotulo) {
    return (
      <div className="mt-1 flex items-center gap-2">
        <span className="flex-1 rounded-lg border border-slate-300 bg-slate-50 px-3 py-1.5 text-sm text-slate-800">{rotulo}</span>
        <button
          type="button"
          onClick={() => { onChange(''); setRotulo(''); setQ(''); setResultados(null); setAberto(true) }}
          className="text-xs font-semibold text-teal-700 hover:underline"
        >
          Trocar
        </button>
      </div>
    )
  }

  return (
    <div className="relative mt-1">
      <input
        value={q}
        placeholder={placeholder}
        onChange={(e) => { setQ(e.target.value); setAberto(true) }}
        onFocus={() => setAberto(true)}
        className="block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm"
      />
      {aberto && resultados && (
        <ul className="absolute z-20 mt-1 max-h-56 w-full overflow-auto rounded-lg border border-slate-200 bg-white shadow-lg dark:border-slate-700 dark:bg-slate-900">
          {resultados.length === 0 && (
            <li className="px-3 py-2 text-xs text-slate-400">Nenhum resultado.</li>
          )}
          {resultados.map((r) => (
            <li key={r.id}>
              <button
                type="button"
                onClick={() => escolher(r)}
                className="block w-full px-3 py-2 text-left text-sm hover:bg-slate-50 dark:hover:bg-slate-800"
              >
                {optionLabel ? optionLabel(r) : r.nome}
              </button>
            </li>
          ))}
        </ul>
      )}
    </div>
  )
}

// Dropdown simples populado por uma rota de listagem da API. Suporta opcao
// vazia (allowEmpty) e filtro dependente de outros campos (filter(row, valores)).
export function ApiSelect({
  path, value, onChange, optionLabel, allowEmpty, emptyLabel, placeholder, filter, valores,
}) {
  const [opcoes, setOpcoes] = useState(null)
  const [erro, setErro] = useState('')

  useEffect(() => {
    let vivo = true
    apiGet(path)
      .then((rows) => { if (vivo) setOpcoes(Array.isArray(rows) ? rows : []) })
      .catch((e) => { if (vivo) setErro(e.message) })
    return () => { vivo = false }
  }, [path])

  const carregando = opcoes === null && !erro
  const lista = (opcoes || []).filter((o) => !filter || filter(o, valores))

  return (
    <>
      <select
        value={value}
        onChange={(e) => onChange(e.target.value)}
        disabled={carregando}
        className={campoCls}
      >
        <option value="">
          {carregando ? 'Carregando...' : allowEmpty ? (emptyLabel || '— nenhum —') : (placeholder || 'Selecione...')}
        </option>
        {lista.map((o) => (
          <option key={o.id} value={o.id}>
            {optionLabel ? optionLabel(o) : (o.nome ?? `#${o.id}`)}
          </option>
        ))}
      </select>
      {erro && <span className="mt-1 block text-xs text-red-600">{erro}</span>}
    </>
  )
}

// Campo de referencia (FK por nome) usado nos formularios genericos. Escolhe o
// modo conforme a config do campo: busca (searchPath) ou dropdown (path).
export function RefSelect({ field, value, onChange, valores }) {
  if (field.searchPath) {
    return (
      <SearchSelect
        path={field.searchPath}
        value={value}
        onChange={onChange}
        optionLabel={field.optionLabel}
        placeholder={field.placeholder}
      />
    )
  }
  return (
    <ApiSelect
      path={field.path}
      value={value}
      onChange={onChange}
      optionLabel={field.optionLabel}
      allowEmpty={field.allowEmpty}
      emptyLabel={field.emptyLabel}
      filter={field.filter}
      valores={valores}
    />
  )
}
