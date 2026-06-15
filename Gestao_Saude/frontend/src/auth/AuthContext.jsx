import { createContext, useContext, useEffect, useState } from 'react'
import {
  apiGet,
  setCredentials,
  clearCredentials,
  loadCredentials,
} from '../api/client'

// Estado global de sessao: papel e vinculos do usuario logado (via GET /me).
const AuthContext = createContext(null)

export function AuthProvider({ children }) {
  const [user, setUser] = useState(null) // { papel, pacienteId, medicoId }
  const [loading, setLoading] = useState(true)

  // Ao montar, tenta restaurar uma sessao salva e revalida com /me.
  useEffect(() => {
    const creds = loadCredentials()
    if (!creds) {
      setLoading(false)
      return
    }
    apiGet('/me')
      .then(setUser)
      .catch(() => clearCredentials())
      .finally(() => setLoading(false))
  }, [])

  async function login(loginName, senha) {
    setCredentials(loginName, senha)
    try {
      const me = await apiGet('/me')
      setUser(me)
      return me
    } catch (err) {
      clearCredentials()
      throw err
    }
  }

  function logout() {
    clearCredentials()
    setUser(null)
  }

  return (
    <AuthContext.Provider value={{ user, loading, login, logout }}>
      {children}
    </AuthContext.Provider>
  )
}

export function useAuth() {
  return useContext(AuthContext)
}
