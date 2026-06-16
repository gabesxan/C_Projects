import { useState } from 'react'
import { NavLink, Outlet } from 'react-router-dom'
import { useAuth } from '../auth/AuthContext'
import { navForRole } from '../nav'
import { Badge, papelLabel, PAPEL_INFO } from './ui'

function linkClass({ isActive }) {
  return [
    'flex items-center gap-3 rounded-lg px-3 py-2 text-sm font-medium transition',
    isActive
      ? 'bg-teal-600 text-white shadow-sm'
      : 'text-slate-300 hover:bg-slate-800 hover:text-white',
  ].join(' ')
}

function SidebarContent({ itens, onNavigate }) {
  return (
    <>
      <div className="flex items-center gap-3 px-2 py-1">
        <div className="flex h-10 w-10 items-center justify-center rounded-xl bg-teal-600 text-xl">
          ⚕️
        </div>
        <div>
          <p className="text-base font-bold leading-tight text-white">SIGEH-DF</p>
          <p className="text-xs text-slate-400">Gestao Hospitalar</p>
        </div>
      </div>

      <nav className="mt-6 space-y-1">
        {itens.map((item) => (
          <NavLink
            key={item.to}
            to={item.to}
            end={item.end}
            className={linkClass}
            onClick={onNavigate}
          >
            <span aria-hidden className="text-base">{item.icon}</span>
            {item.label}
          </NavLink>
        ))}
      </nav>
    </>
  )
}

export default function Layout() {
  const { user, logout } = useAuth()
  const itens = navForRole(user.papel)
  const [menuAberto, setMenuAberto] = useState(false)
  const tone = PAPEL_INFO[user.papel]?.tone ?? 'slate'

  return (
    <div className="min-h-screen bg-slate-50 lg:flex">
      {/* Sidebar fixa (desktop) */}
      <aside className="hidden w-64 shrink-0 bg-slate-900 p-4 lg:block">
        <SidebarContent itens={itens} />
      </aside>

      {/* Drawer (mobile) */}
      {menuAberto && (
        <div className="fixed inset-0 z-40 lg:hidden">
          <div
            className="absolute inset-0 bg-slate-900/50"
            onClick={() => setMenuAberto(false)}
          />
          <aside className="absolute left-0 top-0 h-full w-64 bg-slate-900 p-4">
            <SidebarContent itens={itens} onNavigate={() => setMenuAberto(false)} />
          </aside>
        </div>
      )}

      <div className="flex min-w-0 flex-1 flex-col">
        <header className="sticky top-0 z-30 flex items-center justify-between gap-3 border-b border-slate-200 bg-white/90 px-4 py-3 backdrop-blur lg:px-8">
          <button
            className="rounded-lg p-2 text-slate-600 hover:bg-slate-100 lg:hidden"
            onClick={() => setMenuAberto(true)}
            aria-label="Abrir menu"
          >
            ☰
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
              onClick={logout}
              className="rounded-lg border border-slate-300 px-3 py-1.5 text-sm font-medium text-slate-700 hover:bg-slate-50"
            >
              Sair
            </button>
          </div>
        </header>

        <main className="mx-auto w-full max-w-6xl flex-1 p-4 lg:p-8">
          <Outlet />
        </main>
      </div>
    </div>
  )
}
