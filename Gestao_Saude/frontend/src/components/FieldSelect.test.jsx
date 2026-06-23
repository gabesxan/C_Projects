import { useState } from 'react'
import { describe, it, expect, vi, beforeEach } from 'vitest'
import { render, screen, fireEvent, waitFor } from '@testing-library/react'
import userEvent from '@testing-library/user-event'

// O cliente HTTP e mockado: os testes controlam o que a API "devolve".
vi.mock('../api/client', () => ({
  apiGet: vi.fn(),
}))
import { apiGet } from '../api/client'
import { ApiSelect, SearchSelect, RefSelect } from './FieldSelect'

beforeEach(() => {
  vi.clearAllMocks()
})

describe('ApiSelect', () => {
  it('carrega as opcoes da API e emite o id ao selecionar', async () => {
    apiGet.mockResolvedValue([
      { id: 1, nome: 'Ana' },
      { id: 2, nome: 'Bia' },
    ])
    const onChange = vi.fn()
    render(<ApiSelect path="/medicos" value="" onChange={onChange} />)

    // Enquanto carrega, o select fica desabilitado com o rotulo "Carregando...".
    expect(screen.getByText('Carregando...')).toBeInTheDocument()

    // Depois do fetch, as opcoes aparecem.
    await waitFor(() => expect(screen.getByRole('option', { name: 'Ana' })).toBeInTheDocument())
    expect(apiGet).toHaveBeenCalledWith('/medicos')

    fireEvent.change(screen.getByRole('combobox'), { target: { value: '2' } })
    expect(onChange).toHaveBeenCalledWith('2')
  })

  it('mostra a mensagem de erro quando a API falha', async () => {
    apiGet.mockRejectedValue(new Error('falha de rede'))
    render(<ApiSelect path="/medicos" value="" onChange={() => {}} />)
    await waitFor(() => expect(screen.getByText('falha de rede')).toBeInTheDocument())
  })
})

describe('SearchSelect', () => {
  // Wrapper controlado: reflete o value escolhido de volta no componente, como
  // faz o formulario real (so assim o modo "selecionado" aparece).
  function SearchControlado({ onChange }) {
    const [value, setValue] = useState('')
    return (
      <SearchSelect
        path="/pacientes/buscar"
        value={value}
        onChange={(v) => { setValue(v); onChange(v) }}
        optionLabel={(r) => `${r.nome} · ${r.documento}`}
      />
    )
  }

  it('busca pelo termo digitado e seleciona pelo nome guardando o id', async () => {
    apiGet.mockResolvedValue([{ id: 7, nome: 'Joao', documento: '123' }])
    const onChange = vi.fn()
    const user = userEvent.setup()
    render(<SearchControlado onChange={onChange} />)

    await user.type(screen.getByPlaceholderText(/buscar por nome/i), 'jo')

    // Apos o debounce, a API e consultada com ?q= e o resultado aparece.
    await waitFor(() =>
      expect(apiGet).toHaveBeenCalledWith('/pacientes/buscar?q=jo'),
    )
    const opcao = await screen.findByRole('button', { name: 'Joao · 123' })
    await user.click(opcao)

    expect(onChange).toHaveBeenCalledWith('7')
    // Modo "selecionado": mostra o rotulo e o botao Trocar.
    expect(screen.getByText('Joao · 123')).toBeInTheDocument()
    expect(screen.getByRole('button', { name: 'Trocar' })).toBeInTheDocument()
  })
})

describe('RefSelect', () => {
  it('usa SearchSelect quando o campo define searchPath', async () => {
    apiGet.mockResolvedValue([])
    render(
      <RefSelect
        field={{ searchPath: '/pacientes/buscar', placeholder: 'Buscar paciente' }}
        value=""
        onChange={() => {}}
      />,
    )
    expect(screen.getByPlaceholderText('Buscar paciente')).toBeInTheDocument()
  })

  it('usa ApiSelect (dropdown) quando o campo define path', async () => {
    apiGet.mockResolvedValue([{ id: 1, nome: 'Cardiologia' }])
    render(<RefSelect field={{ path: '/medicos' }} value="" onChange={() => {}} />)
    await waitFor(() => expect(screen.getByRole('combobox')).toBeInTheDocument())
  })
})
