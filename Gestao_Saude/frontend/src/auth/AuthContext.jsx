import { createContext, useContext, useEffect, useState } from 'react'
import { apiGet, apiLogin, apiLogout, loadToken } from '../api/client'

// Estado global de sessao: papel e vinculos do usuario logado (via GET /me).
const AuthContext = createContext(null)

export function AuthProvider({ children }) {
  const [user, setUser] = useState(null) // { papel, pacienteId, medicoId }
  const [loading, setLoading] = useState(true)

  // Ao montar, tenta restaurar um token salvo e revalida com /me.
  useEffect(() => {
    const token = loadToken()
    if (!token) {
      setLoading(false)
      return
    }
    apiGet('/me')
      .then(setUser)
      .catch(() => apiLogout())
      .finally(() => setLoading(false))
  }, [])

  async function login(loginName, senha) {
    // POST /sessao valida as credenciais, registra a auditoria e devolve o
    // token (guardado pelo cliente) junto do perfil (papel + vinculos).
    const me = await apiLogin(loginName, senha)
    setUser(me)
    return me
  }

  async function logout() {
    await apiLogout()
    setUser(null)
  }

  // Apos a troca de senha, libera o app (some a exigencia de troca).
  function confirmarTrocaSenha() {
    setUser((u) => (u ? { ...u, trocarSenha: false } : u))
  }

  return (
    <AuthContext.Provider value={{ user, loading, login, logout, confirmarTrocaSenha }}>
      {children}
    </AuthContext.Provider>
  )
}

export function useAuth() {
  return useContext(AuthContext)
}
