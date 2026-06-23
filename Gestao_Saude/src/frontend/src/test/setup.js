// Setup global dos testes de componente: matchers do jest-dom (toBeInTheDocument
// etc.) e limpeza do DOM entre os testes.
import '@testing-library/jest-dom/vitest'
import { afterEach } from 'vitest'
import { cleanup } from '@testing-library/react'

afterEach(() => {
  cleanup()
})
