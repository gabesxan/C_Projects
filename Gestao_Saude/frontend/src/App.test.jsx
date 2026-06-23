import { describe, it, expect, beforeEach, vi } from 'vitest'
import { render, screen } from '@testing-library/react'
import { MemoryRouter } from 'react-router-dom'

// Estado de auth controlavel pelos testes (via vi.hoisted para o mock acessar).
const auth = vi.hoisted(() => ({ state: { user: null, loading: false } }))

vi.mock('./auth/AuthContext', () => ({
  useAuth: () => auth.state,
  AuthProvider: ({ children }) => children,
}))

// Layout real puxa nav/icones; aqui basta renderizar a rota filha (Outlet).
vi.mock('./components/Layout', async () => {
  const { Outlet } = await import('react-router-dom')
  return { default: () => <Outlet /> }
})

// Paginas pesadas (graficos/fetch) viram stubs identificaveis.
vi.mock('./pages/Relatorios', () => ({ default: () => <div>STUB_RELATORIOS</div> }))
vi.mock('./pages/Dashboard', () => ({ default: () => <div>STUB_DASHBOARD</div> }))
vi.mock('./pages/Login', () => ({ default: () => <div>STUB_LOGIN</div> }))

import App from './App'

function renderEm(rota) {
  return render(
    <MemoryRouter initialEntries={[rota]}>
      <App />
    </MemoryRouter>,
  )
}

beforeEach(() => {
  auth.state = { user: null, loading: false }
})

describe('Guardas de rota (App)', () => {
  it('mostra "Carregando..." enquanto a sessao revalida', () => {
    auth.state = { user: null, loading: true }
    renderEm('/relatorios')
    expect(screen.getByText('Carregando...')).toBeInTheDocument()
  })

  it('sem sessao, redireciona para /login', () => {
    auth.state = { user: null, loading: false }
    renderEm('/relatorios')
    expect(screen.getByText('STUB_LOGIN')).toBeInTheDocument()
  })

  it('papel sem permissao ve "Acesso negado" (mantendo a rota)', () => {
    auth.state = { user: { papel: 'PACIENTE' }, loading: false }
    renderEm('/relatorios')
    expect(screen.getByText('Acesso negado')).toBeInTheDocument()
    expect(screen.queryByText('STUB_RELATORIOS')).not.toBeInTheDocument()
  })

  it('papel permitido acessa a area restrita', () => {
    auth.state = { user: { papel: 'ADMIN' }, loading: false }
    renderEm('/relatorios')
    expect(screen.getByText('STUB_RELATORIOS')).toBeInTheDocument()
  })

  it('primeiro acesso (trocarSenha) bloqueia o app ate a troca', () => {
    auth.state = { user: { papel: 'ADMIN', trocarSenha: true }, loading: false }
    renderEm('/')
    // A tela obrigatoria de troca substitui o conteudo normal do painel.
    expect(screen.queryByText('STUB_DASHBOARD')).not.toBeInTheDocument()
  })
})
