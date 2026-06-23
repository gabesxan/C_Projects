import { useEffect, useState } from 'react'
import { NavLink, Outlet } from 'react-router-dom'
import { useAuth } from '../auth/AuthContext'
import { navForRole, SECTIONS } from '../nav'
import { Badge, papelLabel, PAPEL_INFO } from './ui'
import { ICONS, Icon } from './icons'

function linkClass({ isActive }) {
  return [
    'group flex items-center gap-3 rounded-2xl px-3 py-2.5 text-sm font-medium transition-all duration-200',
    isActive
      ? 'bg-white/12 text-white shadow-sm ring-1 ring-white/10'
      : 'text-slate-300 hover:bg-white/8 hover:text-white',
  ].join(' ')
}

function NavItem({ item, onNavigate }) {
  return (
    <NavLink to={item.to} end={item.end} className={linkClass} onClick={onNavigate}>
      <Icon icon={ICONS[item.icon]} className="text-slate-300 transition group-hover:text-white" size={18} />
      {item.label}
    </NavLink>
  )
}

// Agrupa os itens visiveis em secoes (na ordem de SECTIONS). Itens sem secao
// (ex.: Painel) ficam no topo, fora de qualquer grupo.
function SidebarContent({ itens, onNavigate }) {
  const semSecao = itens.filter((i) => !i.section)
  const grupos = SECTIONS.map((nome) => ({
    nome,
    itens: itens.filter((i) => i.section === nome),
  })).filter((g) => g.itens.length > 0)

  return (
    <>
      <div className="flex items-center gap-3 px-2 py-1">
        <div className="flex h-11 w-11 items-center justify-center rounded-2xl bg-teal-500/16 text-teal-200 ring-1 ring-teal-300/20">
          <Icon icon={ICONS.hospital} size={22} />
        </div>
        <div>
          <p className="text-base font-bold leading-tight text-white">SIGEH-DF</p>
          <p className="text-xs text-slate-400">Gestao Hospitalar</p>
        </div>
      </div>

      <nav className="mt-6 space-y-5">
        {semSecao.length > 0 && (
          <div className="space-y-1">
            {semSecao.map((item) => (
              <NavItem key={item.to} item={item} onNavigate={onNavigate} />
            ))}
          </div>
        )}
        {grupos.map((g) => (
          <div key={g.nome} className="space-y-1">
            <p className="px-3 pb-1 text-[11px] font-semibold uppercase tracking-wider text-slate-500">
              {g.nome}
            </p>
            {g.itens.map((item) => (
              <NavItem key={item.to} item={item} onNavigate={onNavigate} />
            ))}
          </div>
        ))}
      </nav>
    </>
  )
}

function FooterContato() {
  const contatos = [
    { label: 'Email', value: 'atendimento@sigehdf.gov.br', icon: 'mail' },
    { label: 'Telefone', value: '(61) 3333-0000', icon: 'phone' },
    { label: 'Endereço', value: 'Brasília, DF', icon: 'location' },
    { label: 'Horário', value: 'Segunda a sexta, 8h às 18h', icon: 'clock' },
  ]

  return (
    <footer className="mt-auto border-t border-slate-200/70 px-4 py-3 text-xs text-slate-500 dark:border-white/10 dark:text-slate-400 lg:px-8">
      <div className="mx-auto flex w-full max-w-6xl flex-col gap-2 sm:flex-row sm:flex-wrap sm:items-center sm:justify-between">
        <div className="flex items-center justify-center gap-2 sm:justify-start">
          <span className="font-semibold text-slate-600 dark:text-slate-300">Fale Conosco</span>
          <span className="hidden text-slate-300 dark:text-slate-700 sm:inline">|</span>
          <span className="hidden sm:inline">Atendimento SIGEH-DF</span>
        </div>

        <div className="flex flex-wrap items-center justify-center gap-x-4 gap-y-1.5 sm:justify-end">
          {contatos.map((contato) => (
            <span key={contato.label} className="inline-flex min-w-0 items-center gap-1.5">
              <Icon icon={ICONS[contato.icon]} className="shrink-0 text-slate-400 dark:text-slate-500" size={13} />
              <span className="sr-only">{contato.label}: </span>
              <span className="min-w-0 break-words leading-5">{contato.value}</span>
            </span>
          ))}
        </div>
      </div>
    </footer>
  )
}

export default function Layout() {
  const { user, logout } = useAuth()
  const itens = navForRole(user.papel)
  const [menuAberto, setMenuAberto] = useState(false)
  const [tema, setTema] = useState(() =>
    document.documentElement.classList.contains('dark') ? 'dark' : 'light'
  )
  const tone = PAPEL_INFO[user.papel]?.tone ?? 'slate'

  useEffect(() => {
    document.documentElement.classList.toggle('dark', tema === 'dark')
    localStorage.setItem('sigeh_theme', tema)
  }, [tema])

  return (
    <div className="min-h-screen bg-slate-50 dark:bg-slate-950 lg:flex">
      {/* Sidebar fixa (desktop) */}
      <aside className="hidden w-64 shrink-0 bg-slate-950 p-4 lg:block">
        <SidebarContent itens={itens} />
      </aside>

      {/* Drawer (mobile) */}
      {menuAberto && (
        <div className="fixed inset-0 z-40 lg:hidden">
          <div
            className="absolute inset-0 bg-slate-900/50"
            onClick={() => setMenuAberto(false)}
          />
          <aside className="absolute left-0 top-0 h-full w-64 bg-slate-950 p-4 shadow-2xl">
            <SidebarContent itens={itens} onNavigate={() => setMenuAberto(false)} />
          </aside>
        </div>
      )}

      <div className="flex min-w-0 flex-1 flex-col">
        <header className="sticky top-0 z-30 flex items-center justify-between gap-3 border-b border-slate-200 bg-white/90 px-4 py-3 backdrop-blur dark:border-slate-700 dark:bg-slate-900/90 lg:px-8">
          <button
            className="rounded-xl p-2 text-slate-600 hover:bg-slate-100 dark:text-slate-200 dark:hover:bg-slate-800 lg:hidden"
            onClick={() => setMenuAberto(true)}
            aria-label="Abrir menu"
          >
            <Icon icon={ICONS.menu} size={20} />
          </button>

          <div className="flex flex-1 items-center justify-end gap-3">
            <div className="text-right">
              <p className="text-sm font-semibold text-slate-800">
                {user.login ?? 'Sessao'}
              </p>
              <p className="text-xs text-slate-400">Logado</p>
            </div>
            <Badge tone={tone}>{papelLabel(user.papel)}</Badge>
            <button
              onClick={() => setTema((v) => (v === 'dark' ? 'light' : 'dark'))}
              className="rounded-xl border border-slate-300/80 bg-white/70 p-2 text-slate-700 shadow-sm hover:bg-white dark:border-slate-600 dark:bg-slate-900/70 dark:text-slate-100 dark:hover:bg-slate-800"
              aria-label="Alternar tema"
              title="Alternar tema"
            >
              <Icon icon={tema === 'dark' ? ICONS.sun : ICONS.moon} size={18} />
            </button>
            <button
              onClick={logout}
              className="rounded-xl border border-slate-300/80 bg-white/70 px-3 py-1.5 text-sm font-medium text-slate-700 shadow-sm hover:bg-white dark:border-slate-600 dark:bg-slate-900/70 dark:text-slate-100 dark:hover:bg-slate-800"
            >
              Sair
            </button>
          </div>
        </header>

        <main className="mx-auto w-full max-w-6xl flex-1 p-4 lg:p-8">
          <Outlet />
        </main>
        <FooterContato />
      </div>
    </div>
  )
}
