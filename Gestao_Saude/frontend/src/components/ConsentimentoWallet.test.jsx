import { describe, it, expect, vi, beforeEach, afterEach } from 'vitest'
import { render, screen, waitFor } from '@testing-library/react'
import userEvent from '@testing-library/user-event'

vi.mock('../api/client', () => ({
  listarMeusConsentimentos: vi.fn(),
  revogarConsentimento: vi.fn(),
  listarMeuRelatorioAcessos: vi.fn(),
}))
import {
  listarMeusConsentimentos,
  revogarConsentimento,
  listarMeuRelatorioAcessos,
} from '../api/client'
import ConsentimentoWallet from './ConsentimentoWallet'

const CONSENTIMENTOS = [
  {
    id: 1,
    finalidade: 'COMPARTILHAMENTO_DADOS',
    versaoTermo: 'v1.0',
    status: 'CONCEDIDO',
    concedidoEm: '2026-06-01 10:00:00',
    revogadoEm: '',
    motivoRevogacao: '',
  },
  {
    id: 2,
    finalidade: 'PESQUISA_CLINICA',
    versaoTermo: 'v2.0',
    status: 'REVOGADO',
    concedidoEm: '2026-05-01 09:00:00',
    revogadoEm: '2026-05-20 14:00:00',
    motivoRevogacao: 'Não autorizo mais',
  },
]

const ACESSOS = {
  pacienteId: 1,
  acessos: [
    {
      id: 50,
      usuarioId: 3,
      usuarioLogin: 'dr.ana',
      acao: 'EXAME_RESULTADO',
      entidade: 'exame',
      entidadeId: 7,
      detalhe: '',
      criadoEm: '2026-06-02 11:00:00',
    },
  ],
}

beforeEach(() => {
  vi.clearAllMocks()
  listarMeusConsentimentos.mockResolvedValue(CONSENTIMENTOS)
  listarMeuRelatorioAcessos.mockResolvedValue(ACESSOS)
})

afterEach(() => {
  vi.restoreAllMocks()
})

describe('ConsentimentoWallet', () => {
  it('mostra consentimentos concedidos e revogados com status, versão e datas', async () => {
    render(<ConsentimentoWallet />)

    expect(await screen.findByText('COMPARTILHAMENTO_DADOS')).toBeInTheDocument()
    expect(screen.getByText('PESQUISA_CLINICA')).toBeInTheDocument()
    // Status badges e versao do termo.
    expect(screen.getByText('Concedido')).toBeInTheDocument()
    expect(screen.getByText('Revogado')).toBeInTheDocument()
    expect(screen.getByText('Termo v1.0')).toBeInTheDocument()
    expect(screen.getByText('Termo v2.0')).toBeInTheDocument()
    // Grupos separados (concedidos x revogados).
    expect(screen.getByText('Concedidos (1)')).toBeInTheDocument()
    expect(screen.getByText('Revogados (1)')).toBeInTheDocument()
    // Datas de concessao/revogacao e motivo da revogacao.
    expect(screen.getByText('2026-06-01 10:00:00')).toBeInTheDocument()
    expect(screen.getByText('2026-05-20 14:00:00')).toBeInTheDocument()
    expect(screen.getByText('Motivo: Não autorizo mais')).toBeInTheDocument()
  })

  it('mostra o relatório de acessos (quem, quando, ação)', async () => {
    render(<ConsentimentoWallet />)
    expect(await screen.findByText(/Lançou resultado de exame/)).toBeInTheDocument()
    expect(screen.getByText(/dr\.ana/)).toBeInTheDocument()
    expect(screen.getByText('2026-06-02 11:00:00')).toBeInTheDocument()
  })

  it('mostra estados vazios quando não há dados', async () => {
    listarMeusConsentimentos.mockResolvedValue([])
    listarMeuRelatorioAcessos.mockResolvedValue({ pacienteId: 1, acessos: [] })
    render(<ConsentimentoWallet />)
    expect(await screen.findByText('Nenhum consentimento registrado.')).toBeInTheDocument()
    expect(screen.getByText('Nenhum acesso registrado.')).toBeInTheDocument()
  })

  it('exige motivo para revogar (não chama o client sem motivo)', async () => {
    const user = userEvent.setup()
    render(<ConsentimentoWallet />)
    await screen.findByText('COMPARTILHAMENTO_DADOS')

    vi.spyOn(window, 'prompt').mockReturnValue('   ')
    await user.click(screen.getByRole('button', { name: 'Revogar consentimento' }))

    expect(await screen.findByText('Informe o motivo da revogação.')).toBeInTheDocument()
    expect(revogarConsentimento).not.toHaveBeenCalled()
  })

  it('revoga com motivo e recarrega a lista', async () => {
    revogarConsentimento.mockResolvedValue({ id: 1, status: 'REVOGADO' })
    const user = userEvent.setup()
    render(<ConsentimentoWallet />)
    await screen.findByText('COMPARTILHAMENTO_DADOS')

    vi.spyOn(window, 'prompt').mockReturnValue('Não autorizo mais o uso')
    await user.click(screen.getByRole('button', { name: 'Revogar consentimento' }))

    await waitFor(() =>
      expect(revogarConsentimento).toHaveBeenCalledWith(1, 'Não autorizo mais o uso'),
    )
    // Recarrega consentimentos (chamada inicial + recarregamento) e acessos.
    await waitFor(() => expect(listarMeusConsentimentos).toHaveBeenCalledTimes(2))
    expect(await screen.findByText('Consentimento revogado.')).toBeInTheDocument()
  })

  it('mostra erro quando a carga de consentimentos falha', async () => {
    listarMeusConsentimentos.mockRejectedValue(new Error('falha de rede'))
    render(<ConsentimentoWallet />)
    expect(await screen.findByText('falha de rede')).toBeInTheDocument()
  })
})
