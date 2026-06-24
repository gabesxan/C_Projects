import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, waitFor } from '@testing-library/react'
import userEvent from '@testing-library/user-event'

vi.mock('../api/client', () => ({
  listarNotificacoes: vi.fn(),
  contarNotificacoes: vi.fn(),
  marcarNotificacaoLida: vi.fn(),
  marcarTodasNotificacoesLidas: vi.fn(),
}))
import {
  listarNotificacoes,
  contarNotificacoes,
  marcarNotificacaoLida,
  marcarTodasNotificacoesLidas,
} from '../api/client'
import NotificationBell from './NotificationBell'

const ITENS = [
  { id: 10, titulo: 'Novo paciente na fila de consulta', mensagem: 'Senha C001 aguardando atendimento.', tipo: 'FILA', entidade: 'checkin', entidadeId: 0, lida: 0, criadoEm: '2026-06-24 10:00:00' },
  { id: 9, titulo: 'Paciente assumido', mensagem: 'Um medico assumiu um paciente.', tipo: 'ATENDIMENTO', entidade: 'checkin', entidadeId: 5, lida: 1, criadoEm: '2026-06-24 09:00:00' },
]

beforeEach(() => {
  vi.clearAllMocks()
  contarNotificacoes.mockResolvedValue({ naoLidas: 2 })
  listarNotificacoes.mockResolvedValue(ITENS)
  marcarNotificacaoLida.mockResolvedValue({})
  marcarTodasNotificacoesLidas.mockResolvedValue({})
})

afterEach(() => {
  vi.restoreAllMocks()
})

describe('NotificationBell', () => {
  it('mostra o contador de nao lidas (polling inicial)', async () => {
    render(<NotificationBell />)
    expect(await screen.findByText('2')).toBeInTheDocument()
    expect(contarNotificacoes).toHaveBeenCalled()
  })

  it('abre o painel e lista as notificacoes', async () => {
    const user = userEvent.setup()
    render(<NotificationBell />)
    await screen.findByText('2')

    await user.click(screen.getByRole('button', { name: 'Notificações' }))

    expect(await screen.findByText('Novo paciente na fila de consulta')).toBeInTheDocument()
    expect(screen.getByText('Paciente assumido')).toBeInTheDocument()
    expect(listarNotificacoes).toHaveBeenCalled()
  })

  it('marca uma notificacao como lida ao clicar nela', async () => {
    const user = userEvent.setup()
    render(<NotificationBell />)
    await screen.findByText('2')
    await user.click(screen.getByRole('button', { name: 'Notificações' }))

    await user.click(await screen.findByText('Novo paciente na fila de consulta'))

    await waitFor(() => expect(marcarNotificacaoLida).toHaveBeenCalledWith(10))
  })

  it('marca todas como lidas', async () => {
    const user = userEvent.setup()
    render(<NotificationBell />)
    await screen.findByText('2')
    await user.click(screen.getByRole('button', { name: 'Notificações' }))
    await screen.findByText('Novo paciente na fila de consulta')

    await user.click(screen.getByRole('button', { name: 'Marcar todas como lidas' }))

    await waitFor(() => expect(marcarTodasNotificacoesLidas).toHaveBeenCalled())
  })

  it('mostra estado vazio quando nao ha notificacoes', async () => {
    contarNotificacoes.mockResolvedValue({ naoLidas: 0 })
    listarNotificacoes.mockResolvedValue([])
    const user = userEvent.setup()
    render(<NotificationBell />)

    await user.click(screen.getByRole('button', { name: 'Notificações' }))
    expect(await screen.findByText('Nenhuma notificação.')).toBeInTheDocument()
  })
})
