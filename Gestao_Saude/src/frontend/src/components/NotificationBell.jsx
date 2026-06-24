import { useCallback, useEffect, useRef, useState } from 'react'
import {
  listarNotificacoes,
  contarNotificacoes,
  marcarNotificacaoLida,
  marcarTodasNotificacoesLidas,
} from '../api/client'
import { ICONS, Icon } from './icons'

const TIPO_TONE = {
  FILA: 'text-sky-600',
  TRIAGEM: 'text-red-600',
  ATENDIMENTO: 'text-teal-600',
  INFO: 'text-slate-500',
}

// Sino de notificacoes in-app, disponivel para todos os papeis. Faz polling da
// contagem de nao lidas e carrega a lista ao abrir o painel.
export default function NotificationBell() {
  const [aberto, setAberto] = useState(false)
  const [naoLidas, setNaoLidas] = useState(0)
  const [itens, setItens] = useState(null)
  const [erro, setErro] = useState('')
  const intervalo = useRef(null)

  const atualizarContagem = useCallback(() => {
    contarNotificacoes()
      .then((r) => setNaoLidas(Number(r?.naoLidas) || 0))
      .catch(() => {}) // contagem e best-effort; nao polui a UI com erro
  }, [])

  // Polling da contagem (a cada 30s) enquanto o componente estiver montado.
  useEffect(() => {
    atualizarContagem()
    intervalo.current = window.setInterval(atualizarContagem, 30000)
    return () => window.clearInterval(intervalo.current)
  }, [atualizarContagem])

  function abrir() {
    setAberto(true)
    setItens(null)
    setErro('')
    listarNotificacoes()
      .then(setItens)
      .catch((e) => setErro(e.message || 'Falha ao carregar notificações.'))
  }

  async function marcarUma(n) {
    if (n.lida) return
    try {
      await marcarNotificacaoLida(n.id)
      setItens((lista) => (lista || []).map((x) => (x.id === n.id ? { ...x, lida: 1 } : x)))
      setNaoLidas((c) => Math.max(0, c - 1))
    } catch {
      // silencioso: a contagem se corrige no proximo polling
    }
  }

  async function marcarTodas() {
    try {
      await marcarTodasNotificacoesLidas()
      setItens((lista) => (lista || []).map((x) => ({ ...x, lida: 1 })))
      setNaoLidas(0)
    } catch {
      /* idem */
    }
  }

  return (
    <div className="relative">
      <button
        onClick={() => (aberto ? setAberto(false) : abrir())}
        className="relative rounded-xl border border-slate-300/80 bg-white/70 p-2 text-slate-700 shadow-sm hover:bg-white dark:border-slate-600 dark:bg-slate-900/70 dark:text-slate-100 dark:hover:bg-slate-800"
        aria-label="Notificações"
        title="Notificações"
      >
        <Icon icon={ICONS.bell} size={18} />
        {naoLidas > 0 && (
          <span className="absolute -right-1 -top-1 flex h-4 min-w-4 items-center justify-center rounded-full bg-red-600 px-1 text-[10px] font-bold text-white">
            {naoLidas > 9 ? '9+' : naoLidas}
          </span>
        )}
      </button>

      {aberto && (
        <>
          <div className="fixed inset-0 z-40" onClick={() => setAberto(false)} />
          <div className="absolute right-0 z-50 mt-2 w-80 overflow-hidden rounded-2xl border border-slate-200 bg-white shadow-xl dark:border-slate-700 dark:bg-slate-900">
            <div className="flex items-center justify-between border-b border-slate-100 px-4 py-2.5 dark:border-slate-800">
              <p className="text-sm font-semibold text-slate-800 dark:text-slate-100">Notificações</p>
              <button
                onClick={marcarTodas}
                className="text-xs font-medium text-teal-700 hover:underline disabled:text-slate-400 dark:text-teal-300"
                disabled={naoLidas === 0}
              >
                Marcar todas como lidas
              </button>
            </div>

            <div className="max-h-96 overflow-y-auto">
              {erro && <p className="px-4 py-6 text-center text-sm text-red-600">{erro}</p>}
              {!erro && itens === null && (
                <p className="px-4 py-6 text-center text-sm text-slate-400">Carregando...</p>
              )}
              {!erro && Array.isArray(itens) && itens.length === 0 && (
                <p className="px-4 py-8 text-center text-sm text-slate-400">Nenhuma notificação.</p>
              )}
              {!erro && Array.isArray(itens) && itens.length > 0 && (
                <ul className="divide-y divide-slate-100 dark:divide-slate-800">
                  {itens.map((n) => (
                    <li
                      key={n.id}
                      onClick={() => marcarUma(n)}
                      className={`cursor-pointer px-4 py-3 hover:bg-slate-50 dark:hover:bg-slate-800/60 ${
                        n.lida ? 'opacity-60' : ''
                      }`}
                    >
                      <div className="flex items-start gap-2">
                        {!n.lida && <span className="mt-1.5 h-2 w-2 shrink-0 rounded-full bg-teal-500" />}
                        <div className={n.lida ? 'pl-4' : ''}>
                          <p className={`text-sm font-semibold ${TIPO_TONE[n.tipo] || TIPO_TONE.INFO}`}>
                            {n.titulo}
                          </p>
                          {n.mensagem && (
                            <p className="mt-0.5 text-xs text-slate-600 dark:text-slate-300">{n.mensagem}</p>
                          )}
                          <p className="mt-1 text-[11px] text-slate-400">{n.criadoEm}</p>
                        </div>
                      </div>
                    </li>
                  ))}
                </ul>
              )}
            </div>
          </div>
        </>
      )}
    </div>
  )
}
