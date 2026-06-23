import js from '@eslint/js'
import globals from 'globals'
import reactHooks from 'eslint-plugin-react-hooks'
import reactRefresh from 'eslint-plugin-react-refresh'
import { defineConfig, globalIgnores } from 'eslint/config'

export default defineConfig([
  globalIgnores(['dist']),
  {
    files: ['**/*.{js,jsx}'],
    extends: [
      js.configs.recommended,
      reactHooks.configs.flat.recommended,
      reactRefresh.configs.vite,
    ],
    languageOptions: {
      globals: globals.browser,
      parserOptions: { ecmaFeatures: { jsx: true } },
    },
    rules: {
      // Padrao de carregamento de dados deste projeto: efeitos disparam fetch
      // e fazem setState no .then. A regra estrita abaixo e de performance/DX
      // e fica como aviso, nao erro.
      'react-hooks/set-state-in-effect': 'warn',
      // ui.jsx exporta componentes + constantes/helpers de tema juntos; o
      // alerta de fast-refresh fica como aviso.
      'react-refresh/only-export-components': 'warn',
    },
  },
])
