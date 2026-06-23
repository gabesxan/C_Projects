import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, waitFor } from '@testing-library/react'
import userEvent from '@testing-library/user-event'

vi.mock('../api/client', () => ({
  apiSend: vi.fn(),
  apiGet: vi.fn(),
  ApiError: class ApiError extends Error {
    constructor(status, body) {
      super((body && body.erro) || `HTTP ${status}`)
      this.status = status
      this.body = body
    }
  },
}))
import { apiGet, apiSend } from '../api/client'
import ResourceForm from './ResourceForm'

// Recurso de teste com um campo texto e um select (sem campos ref, para nao
// depender da API nos testes deste componente).
const recurso = {
  path: '/medicos',
  createFields: [
    { name: 'nome', label: 'Nome', type: 'text' },
    { name: 'especialidade', label: 'Especialidade', type: 'select', options: ['Cardiologia', 'Ortopedia'] },
  ],
}

beforeEach(() => {
  vi.clearAllMocks()
  apiGet.mockResolvedValue([])
})

describe('ResourceForm', () => {
  it('abre o formulario, envia os valores e notifica o sucesso', async () => {
    apiSend.mockResolvedValue({ status: 'criado' })
    const onCreated = vi.fn()
    const user = userEvent.setup()
    render(<ResourceForm recurso={recurso} onCreated={onCreated} />)

    // Comeca fechado, apenas com o botao "+ Novo".
    await user.click(screen.getByRole('button', { name: '+ Novo' }))

    await user.type(screen.getByLabelText('Nome'), 'Dra Ana')
    await user.selectOptions(screen.getByLabelText('Especialidade'), 'Ortopedia')
    await user.click(screen.getByRole('button', { name: 'Salvar' }))

    await waitFor(() => expect(onCreated).toHaveBeenCalledTimes(1))
    expect(apiSend).toHaveBeenCalledWith('POST', '/medicos', {
      nome: 'Dra Ana',
      especialidade: 'Ortopedia',
    })
  })

  it('mostra "Dados invalidos." quando a API responde 400', async () => {
    apiSend.mockRejectedValue({ status: 400, message: 'qualquer' })
    const onCreated = vi.fn()
    const user = userEvent.setup()
    render(<ResourceForm recurso={recurso} onCreated={onCreated} />)

    await user.click(screen.getByRole('button', { name: '+ Novo' }))
    await user.type(screen.getByLabelText('Nome'), 'X')
    await user.click(screen.getByRole('button', { name: 'Salvar' }))

    await waitFor(() => expect(screen.getByText('Dados invalidos.')).toBeInTheDocument())
    expect(onCreated).not.toHaveBeenCalled()
  })

  it('o select inicia com a primeira opcao por padrao', async () => {
    apiSend.mockResolvedValue({})
    const user = userEvent.setup()
    render(<ResourceForm recurso={recurso} onCreated={vi.fn()} />)
    await user.click(screen.getByRole('button', { name: '+ Novo' }))
    await user.click(screen.getByRole('button', { name: 'Salvar' }))
    await waitFor(() =>
      expect(apiSend).toHaveBeenCalledWith('POST', '/medicos', {
        nome: '',
        especialidade: 'Cardiologia',
      }),
    )
  })

  it('bloqueia o envio quando uma referencia obrigatoria esta vazia', async () => {
    const user = userEvent.setup()
    render(
      <ResourceForm
        recurso={{
          path: '/exames',
          createFields: [
            { name: 'paciente_id', label: 'Paciente', type: 'ref', path: '/pacientes', required: true },
          ],
        }}
        onCreated={vi.fn()}
      />,
    )

    await user.click(screen.getByRole('button', { name: '+ Novo' }))
    const select = screen.getByRole('combobox')
    await user.click(screen.getByRole('button', { name: 'Salvar' }))

    expect(select).toBeInvalid()
    expect(apiSend).not.toHaveBeenCalled()
  })
})
