import { Routes, Route, Navigate } from 'react-router-dom'
import { useAuth } from './auth/AuthContext'
import Login from './pages/Login'
import Layout from './components/Layout'
import Dashboard from './pages/Dashboard'
import ResourceList from './pages/ResourceList'
import PacienteDetalhe from './pages/PacienteDetalhe'
import Triagem from './pages/Triagem'
import Recepcao from './pages/Recepcao'
import Leitos from './pages/Leitos'
import Internacoes from './pages/Internacoes'
import Relatorios from './pages/Relatorios'
import Financeiro from './pages/Financeiro'
import Laboratorio from './pages/Laboratorio'
import TrocarSenha from './pages/TrocarSenha'
import TrocarSenhaObrigatoria from './pages/TrocarSenhaObrigatoria'
import MinhaSaude from './pages/MinhaSaude'
import Usuarios from './pages/Usuarios'
import Auditoria from './pages/Auditoria'
import AcessoNegado from './pages/AcessoNegado'

// Protege rotas que exigem sessao; aguarda a revalidacao inicial do /me.
function RequireAuth({ children }) {
  const { user, loading } = useAuth()
  if (loading) {
    return <div className="p-8 text-slate-500">Carregando...</div>
  }
  if (!user) {
    return <Navigate to="/login" replace />
  }
  // Primeiro acesso: bloqueia o app ate a troca da senha inicial.
  if (user.trocarSenha) {
    return <TrocarSenhaObrigatoria />
  }
  return children
}

// Restringe uma rota a papeis especificos; caso contrario mostra Acesso negado
// (mantendo a URL, para o usuario entender o que aconteceu).
function RequireRole({ roles, children }) {
  const { user } = useAuth()
  return roles.includes(user.papel) ? children : <AcessoNegado />
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
        <Route
          path="/paciente/:id"
          element={
            <RequireRole roles={['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM']}>
              <PacienteDetalhe />
            </RequireRole>
          }
        />
        <Route
          path="/triagem"
          element={
            <RequireRole roles={['ADMIN', 'MEDICO', 'ENFERMAGEM']}>
              <Triagem />
            </RequireRole>
          }
        />
        <Route
          path="/recepcao"
          element={
            <RequireRole roles={['ADMIN', 'CADASTRO', 'MEDICO', 'ENFERMAGEM']}>
              <Recepcao />
            </RequireRole>
          }
        />
        <Route
          path="/enfermagem"
          element={
            <RequireRole roles={['ADMIN', 'ENFERMAGEM', 'CADASTRO']}>
              <Leitos />
            </RequireRole>
          }
        />
        <Route
          path="/internacao"
          element={
            <RequireRole roles={['ADMIN', 'MEDICO', 'ENFERMAGEM']}>
              <Internacoes />
            </RequireRole>
          }
        />
        <Route
          path="/laboratorio"
          element={
            <RequireRole roles={['ADMIN', 'MEDICO']}>
              <Laboratorio />
            </RequireRole>
          }
        />
        <Route
          path="/relatorios"
          element={
            <RequireRole roles={['ADMIN', 'MEDICO']}>
              <Relatorios />
            </RequireRole>
          }
        />
        <Route
          path="/financeiro"
          element={
            <RequireRole roles={['ADMIN', 'CADASTRO']}>
              <Financeiro />
            </RequireRole>
          }
        />
        <Route
          path="/minha-saude/*"
          element={
            <RequireRole roles={['PACIENTE']}>
              <MinhaSaude />
            </RequireRole>
          }
        />
        <Route
          path="/admin/usuarios"
          element={
            <RequireRole roles={['ADMIN']}>
              <Usuarios />
            </RequireRole>
          }
        />
        <Route
          path="/admin/auditoria"
          element={
            <RequireRole roles={['ADMIN']}>
              <Auditoria />
            </RequireRole>
          }
        />
        <Route path="/trocar-senha" element={<TrocarSenha />} />
        <Route path="/acesso-negado" element={<AcessoNegado />} />
      </Route>
      <Route path="*" element={<Navigate to="/" replace />} />
    </Routes>
  )
}
