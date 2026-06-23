import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, waitFor } from '@testing-library/react'
import userEvent from '@testing-library/user-event'

vi.mock('../api/client', () => ({
  apiGet: vi.fn(),
  apiSend: vi.fn(),
}))

import { apiGet, apiSend } from '../api/client'
import { FormTransferencia } from './Internacoes'

beforeEach(() => {
  vi.clearAllMocks()
  apiGet.mockResolvedValue([
    { id: 8, numero: 204, alaId: 3, status: 'DISPONIVEL' },
    { id: 9, numero: 205, alaId: 3, status: 'OCUPADO' },
  ])
})

describe('FormTransferencia', () => {
  it('mostra apenas leitos disponiveis por rotulo e envia o id escolhido', async () => {
    apiSend.mockResolvedValue({ status: 'transferido' })
    const user = userEvent.setup()
    const onTransferred = vi.fn()

    render(
      <FormTransferencia
        internacao={{ id: 12 }}
        onCancel={vi.fn()}
        onTransferred={onTransferred}
      />,
    )

    expect(screen.getByRole('button', { name: 'Confirmar transferência' })).toBeDisabled()
    await waitFor(() => expect(screen.getByRole('option', { name: 'Leito 204 · Ala 3' })).toBeInTheDocument())
    expect(screen.queryByRole('option', { name: 'Leito 205 · Ala 3' })).not.toBeInTheDocument()

    await user.selectOptions(screen.getByLabelText('Leito de destino'), '8')
    await user.type(screen.getByLabelText('Responsável pela transferência'), 'Enf. Marta')
    await user.click(screen.getByRole('button', { name: 'Confirmar transferência' }))

    await waitFor(() => expect(apiSend).toHaveBeenCalledWith(
      'POST',
      '/internacoes/12/transferir',
      expect.objectContaining({ leito_id: '8', responsavel: 'Enf. Marta' }),
    ))
    expect(onTransferred).toHaveBeenCalled()
  })

  it('exibe estado vazio quando nao ha leitos disponiveis', async () => {
    apiGet.mockResolvedValue([{ id: 9, numero: 205, alaId: 3, status: 'OCUPADO' }])
    render(<FormTransferencia internacao={{ id: 12 }} onCancel={vi.fn()} onTransferred={vi.fn()} />)
    await waitFor(() => expect(screen.getByText('Nenhuma opção disponível.')).toBeInTheDocument())
  })
})
