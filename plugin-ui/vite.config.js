import react from '@vitejs/plugin-react'
import { defineConfig } from 'vite'

export default defineConfig({
  plugins: [react()],
  build: {
    outDir: '../plugin/Straudio/resources/web',
    emptyOutDir: true
  },
  base: './'
})
