import { Routes, Route, Navigate } from 'react-router-dom'
import { useAuth } from './auth/AuthContext'
import Login from './pages/Login'
import Layout from './components/Layout'
import Dashboard from './pages/Dashboard'
import ResourceList from './pages/ResourceList'
import Relatorios from './pages/Relatorios'

// Protege rotas que exigem sessao; aguarda a revalidacao inicial do /me.
function RequireAuth({ children }) {
  const { user, loading } = useAuth()
  if (loading) {
    return <div className="p-8 text-slate-500">Carregando...</div>
  }
  if (!user) {
    return <Navigate to="/login" replace />
  }
  return children
}

export default function App() {
  return (
    <Routes>
      <Route path="/login" element={<Login />} />
      <Route
        element={
          <RequireAuth>
            <Layout />
          </RequireAuth>
        }
      >
        <Route path="/" element={<Dashboard />} />
        <Route path="/r/:key" element={<ResourceList />} />
        <Route path="/relatorios" element={<Relatorios />} />
      </Route>
      <Route path="*" element={<Navigate to="/" replace />} />
    </Routes>
  )
}
