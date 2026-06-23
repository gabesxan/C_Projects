import { useState } from 'react'
import { apiSend } from '../api/client'
import { RefSelect } from './FieldSelect'

// Estado inicial do formulario: todos os campos vazios.
function estadoInicial(fields) {
  const o = {}
  fields.forEach((f) => {
    o[f.name] = f.type === 'select' ? String(f.options[0]?.value ?? f.options[0] ?? '') : ''
  })
  return o
}

export default function ResourceForm({ recurso, onCreated }) {
  const [aberto, setAberto] = useState(false)
  const [valores, setValores] = useState(() => estadoInicial(recurso.createFields))
  const [erro, setErro] = useState('')
  const [salvando, setSalvando] = useState(false)

  function set(name, value) {
    setValores((v) => ({ ...v, [name]: value }))
  }

  async function submit(e) {
    e.preventDefault()
    setErro('')
    const ausente = recurso.createFields.find((f) => f.required && !String(valores[f.name] ?? '').trim())
    if (ausente) {
      setErro(`Selecione ou informe: ${ausente.label}.`)
      return
    }
    setSalvando(true)
    try {
      await apiSend('POST', recurso.path, valores)
      setValores(estadoInicial(recurso.createFields))
      setAberto(false)
      onCreated()
    } catch (err) {
      setErro(err.status === 400 ? 'Dados invalidos.' : err.message)
    } finally {
      setSalvando(false)
    }
  }

  if (!aberto) {
    return (
      <button
        onClick={() => setAberto(true)}
        className="rounded-lg bg-teal-600 text-white text-sm font-medium px-4 py-2 hover:bg-teal-700"
      >
        + Novo
      </button>
    )
  }

  return (
    <form onSubmit={submit} className="bg-white rounded-xl shadow p-5 space-y-4">
      {erro && (
        <div className="rounded-lg bg-red-50 text-red-700 text-sm px-3 py-2">
          {erro}
        </div>
      )}

      <div className="grid sm:grid-cols-2 lg:grid-cols-3 gap-4">
        {recurso.createFields.map((f) => (
          <label key={f.name} className="text-sm text-slate-600">
            {f.label}
            {f.type === 'ref' ? (
              <RefSelect field={f} value={valores[f.name]} onChange={(v) => set(f.name, v)} valores={valores} />
            ) : f.type === 'select' ? (
              <select
                value={valores[f.name]}
                onChange={(e) => set(f.name, e.target.value)}
                className="mt-1 block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm"
                required={f.required}
              >
                {f.options.map((o) => (
                  <option key={o.value ?? o} value={o.value ?? o}>
                    {o.label ?? o}
                  </option>
                ))}
              </select>
            ) : (
              <input
                type={f.type}
                value={valores[f.name]}
                placeholder={f.placeholder || ''}
                onChange={(e) => set(f.name, e.target.value)}
                className="mt-1 block w-full rounded-lg border border-slate-300 px-3 py-1.5 text-sm"
                required={f.required}
              />
            )}
          </label>
        ))}
      </div>

      <div className="flex gap-2">
        <button
          type="submit"
          disabled={salvando}
          className="rounded-lg bg-teal-600 text-white text-sm font-medium px-4 py-2 hover:bg-teal-700 disabled:opacity-60"
        >
          {salvando ? 'Salvando...' : 'Salvar'}
        </button>
        <button
          type="button"
          onClick={() => setAberto(false)}
          className="rounded-lg border border-slate-300 text-sm px-4 py-2 hover:bg-slate-50"
        >
          Cancelar
        </button>
      </div>
    </form>
  )
}
