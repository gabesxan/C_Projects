import { NavLink, Outlet } from 'react-router-dom'
import { useAuth } from '../auth/AuthContext'
import { resourcesForRole } from '../resources'

function navClass({ isActive }) {
  return [
    'block rounded-lg px-3 py-2 text-sm',
    isActive
      ? 'bg-sky-600 text-white'
      : 'text-slate-700 hover:bg-slate-100',
  ].join(' ')
}

export default function Layout() {
  const { user, logout } = useAuth()
  const recursos = resourcesForRole(user.papel)

  return (
    <div className="min-h-screen bg-slate-100">
      <header className="bg-white shadow">
        <div className="px-6 py-4 flex items-center justify-between">
          <div>
            <h1 className="text-lg font-semibold text-slate-800">SIGEH-DF</h1>
            <p className="text-sm text-slate-500">
              Logado como{' '}
              <span className="font-medium text-slate-700">{user.papel}</span>
            </p>
          </div>
          <button
            onClick={logout}
            className="text-sm rounded-lg border border-slate-300 px-3 py-1.5 hover:bg-slate-50"
          >
            Sair
          </button>
        </div>
      </header>

      <div className="flex">
        <nav className="w-56 shrink-0 p-4 space-y-1">
          <NavLink to="/" end className={navClass}>
            Painel
          </NavLink>
          {recursos.map((r) => (
            <NavLink key={r.key} to={`/r/${r.key}`} className={navClass}>
              {r.label}
            </NavLink>
          ))}
          {(user.papel === 'ADMIN' || user.papel === 'MEDICO') && (
            <NavLink to="/relatorios" className={navClass}>
              Relatorios
            </NavLink>
          )}
          {user.papel === 'PACIENTE' && (
            <NavLink to="/minha-saude" className={navClass}>
              Meus dados
            </NavLink>
          )}
        </nav>

        <main className="flex-1 p-6">
          <Outlet />
        </main>
      </div>
    </div>
  )
}
