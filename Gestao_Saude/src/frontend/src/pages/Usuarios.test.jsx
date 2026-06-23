import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, waitFor } from '@testing-library/react'
import userEvent from '@testing-library/user-event'

vi.mock('../api/client', () => ({
  apiGet: vi.fn(),
  apiSend: vi.fn(),
}))

import { apiGet, apiSend } from '../api/client'
import { FormUsuario } from './Usuarios'

beforeEach(() => {
  vi.clearAllMocks()
  apiGet.mockImplementation((path) => {
    if (path === '/medicos') {
      return Promise.resolve([{ id: 42, nome: 'Dra. Helena', crm: 'CRM-42', especialidade: 'Cardiologia' }])
    }
    return Promise.resolve([])
  })
})

describe('FormUsuario', () => {
  it('seleciona o medico pelo nome e envia apenas o id interno', async () => {
    apiSend.mockResolvedValue({ id: 1 })
    const user = userEvent.setup()
    const onCreated = vi.fn()
    render(<FormUsuario onCreated={onCreated} />)

    await user.click(screen.getByRole('button', { name: '+ Novo usuario' }))
    await user.type(screen.getByLabelText('Nome'), 'Helena')
    await user.type(screen.getByLabelText('Login'), 'helena')
    await user.type(screen.getByLabelText('Senha'), 'segura123')
    await user.selectOptions(screen.getByLabelText('Papel'), 'MEDICO')

    await waitFor(() => expect(screen.getByRole('option', { name: /Dra. Helena · CRM-42/ })).toBeInTheDocument())
    await user.selectOptions(screen.getByLabelText('Médico vinculado'), '42')
    await user.click(screen.getByRole('button', { name: 'Salvar' }))

    await waitFor(() => expect(apiSend).toHaveBeenCalledWith('POST', '/usuarios', {
      nome: 'Helena',
      login: 'helena',
      senha: 'segura123',
      papel: 'MEDICO',
      medico_id: '42',
    }))
    expect(onCreated).toHaveBeenCalled()
  })

  it('bloqueia usuario medico sem vinculo selecionado', async () => {
    const user = userEvent.setup()
    render(<FormUsuario onCreated={vi.fn()} />)

    await user.click(screen.getByRole('button', { name: '+ Novo usuario' }))
    await user.type(screen.getByLabelText('Nome'), 'Helena')
    await user.type(screen.getByLabelText('Login'), 'helena')
    await user.type(screen.getByLabelText('Senha'), 'segura123')
    await user.selectOptions(screen.getByLabelText('Papel'), 'MEDICO')
    const select = screen.getByLabelText('Médico vinculado')
    await user.click(screen.getByRole('button', { name: 'Salvar' }))

    expect(select).toBeInvalid()
    expect(apiSend).not.toHaveBeenCalled()
  })
})
