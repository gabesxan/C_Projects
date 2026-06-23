import { useCallback, useEffect, useRef, useState } from 'react'
import {
  listarAnexos,
  enviarAnexo,
  baixarAnexo,
  removerAnexo,
  arquivoParaBase64,
} from '../api/client'
import { Card, Button, Alert, Spinner, Badge } from './ui'
import { Icon, ICONS } from './icons'

// Tipos aceitos pelo backend (pdf, png, jpeg) e limite real de 5 MB.
const MIME_PERMITIDOS = ['application/pdf', 'image/png', 'image/jpeg', 'image/jpg']
const TAMANHO_MAX = 5 * 1024 * 1024

function formatarTamanho(bytes) {
  if (!bytes && bytes !== 0) return '—'
  if (bytes < 1024) return `${bytes} B`
  if (bytes < 1024 * 1024) return `${(bytes / 1024).toFixed(1)} KB`
  return `${(bytes / (1024 * 1024)).toFixed(1)} MB`
}

function rotuloMime(mime) {
  if (mime === 'application/pdf') return 'PDF'
  if (mime === 'image/png') return 'PNG'
  if (mime === 'image/jpeg' || mime === 'image/jpg') return 'JPEG'
  return mime
}

// Traduz erros da API para mensagens amigaveis (sem expor detalhes internos).
function mensagemErro(err) {
  if (err && err.status === 413) return 'Arquivo maior que 5 MB.'
  if (err && err.status === 400) return 'Formato não permitido.'
  return (err && err.message) || 'Falha ao processar o anexo.'
}

/*
 * AnexoPanel: painel reutilizavel de anexos para embutir em paginas customizadas.
 * Props:
 *  - entidade: nome logico da entidade (ex.: "exame", "paciente")
 *  - entidadeId: id da entidade dona dos anexos
 *  - disabled?: desabilita upload e remocao (modo somente leitura)
 */
export default function AnexoPanel({ entidade, entidadeId, disabled = false }) {
  const [itens, setItens] = useState(null)
  const [carregando, setCarregando] = useState(true)
  const [enviando, setEnviando] = useState(false)
  const [erro, setErro] = useState('')
  const [sucesso, setSucesso] = useState('')
  const [arquivo, setArquivo] = useState(null)
  const inputRef = useRef(null)

  const recarregar = useCallback(() => {
    setCarregando(true)
    setErro('')
    listarAnexos(entidade, entidadeId)
      .then((lista) => setItens(Array.isArray(lista) ? lista : []))
      .catch((e) => {
        setItens([])
        setErro(mensagemErro(e))
      })
      .finally(() => setCarregando(false))
  }, [entidade, entidadeId])

  useEffect(() => {
    queueMicrotask(recarregar)
  }, [recarregar])

  function selecionar(e) {
    setErro('')
    setSucesso('')
    setArquivo(e.target.files && e.target.files[0] ? e.target.files[0] : null)
  }

  function limparSelecao() {
    setArquivo(null)
    if (inputRef.current) inputRef.current.value = ''
  }

  async function enviar() {
    if (!arquivo) return
    setErro('')
    setSucesso('')

    if (!MIME_PERMITIDOS.includes(arquivo.type)) {
      setErro('Formato não permitido.')
      return
    }
    if (arquivo.size > TAMANHO_MAX) {
      setErro('Arquivo maior que 5 MB.')
      return
    }

    setEnviando(true)
    try {
      const conteudoB64 = await arquivoParaBase64(arquivo)
      await enviarAnexo({
        entidade,
        entidadeId,
        nome: arquivo.name,
        mime: arquivo.type,
        conteudoB64,
      })
      setSucesso('Anexo enviado com sucesso.')
      limparSelecao()
      recarregar()
    } catch (e) {
      setErro(mensagemErro(e))
    } finally {
      setEnviando(false)
    }
  }

  async function baixar(item) {
    setErro('')
    try {
      await baixarAnexo(item.id)
    } catch (e) {
      setErro(mensagemErro(e))
    }
  }

  async function remover(item) {
    const motivo = window.prompt(`Motivo para remover "${item.nome}":`)
    if (motivo === null) return
    if (!motivo.trim()) {
      setErro('Informe o motivo da remoção.')
      return
    }
    setErro('')
    setSucesso('')
    try {
      await removerAnexo(item.id, motivo.trim())
      setSucesso('Anexo removido.')
      recarregar()
    } catch (e) {
      setErro(mensagemErro(e))
    }
  }

  return (
    <Card className="p-5">
      <div className="flex flex-wrap items-end justify-between gap-3">
        <div>
          <h3 className="flex items-center gap-2 text-base font-semibold text-slate-900 dark:text-white">
            <Icon icon={ICONS.record} size={18} /> Anexos
          </h3>
          <p className="mt-1 text-xs text-slate-500 dark:text-slate-400">
            Formatos aceitos: PDF, PNG, JPEG. Tamanho máximo: 5 MB.
          </p>
        </div>
      </div>

      {!disabled && (
        <div className="mt-4 flex flex-wrap items-center gap-2">
          <input
            ref={inputRef}
            type="file"
            accept=".pdf,.png,.jpg,.jpeg,application/pdf,image/png,image/jpeg"
            onChange={selecionar}
            disabled={enviando}
            className="block w-full max-w-xs text-sm text-slate-600 file:mr-3 file:rounded-lg file:border-0 file:bg-teal-50 file:px-3 file:py-1.5 file:text-sm file:font-semibold file:text-teal-700 hover:file:bg-teal-100 dark:text-slate-300"
          />
          <Button onClick={enviar} disabled={!arquivo || enviando}>
            {enviando ? 'Enviando...' : 'Enviar'}
          </Button>
          {arquivo && !enviando && (
            <Button variant="secondary" onClick={limparSelecao}>
              Limpar
            </Button>
          )}
        </div>
      )}

      {erro && (
        <div className="mt-3">
          <Alert tone="red">{erro}</Alert>
        </div>
      )}
      {sucesso && (
        <div className="mt-3">
          <Alert tone="teal">{sucesso}</Alert>
        </div>
      )}

      <div className="mt-4">
        {carregando ? (
          <Spinner label="Carregando anexos..." />
        ) : itens && itens.length > 0 ? (
          <ul className="divide-y divide-slate-200/70 dark:divide-slate-700/70">
            {itens.map((a) => (
              <li key={a.id} className="flex flex-wrap items-center justify-between gap-3 py-3">
                <div className="min-w-0">
                  <p className="flex items-center gap-2 truncate text-sm font-medium text-slate-900 dark:text-white">
                    <span className="truncate">{a.nome}</span>
                    <Badge tone="slate">{rotuloMime(a.mime)}</Badge>
                  </p>
                  <p className="mt-0.5 text-xs text-slate-500 dark:text-slate-400">
                    {formatarTamanho(a.tamanho)}
                    {a.autorLogin ? ` · ${a.autorLogin}` : ''}
                    {a.criadoEm ? ` · ${a.criadoEm}` : ''}
                  </p>
                </div>
                <div className="flex shrink-0 items-center gap-2">
                  <Button variant="secondary" onClick={() => baixar(a)}>
                    Baixar
                  </Button>
                  {!disabled && (
                    <Button variant="danger" onClick={() => remover(a)}>
                      Remover
                    </Button>
                  )}
                </div>
              </li>
            ))}
          </ul>
        ) : (
          <p className="py-6 text-center text-sm text-slate-500 dark:text-slate-400">
            Nenhum anexo enviado.
          </p>
        )}
      </div>
    </Card>
  )
}
