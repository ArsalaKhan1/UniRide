import React from 'react'
import { createRoot } from 'react-dom/client'
import { BrowserRouter } from 'react-router-dom'
import App from './App'
import './index.css'
import ErrorBoundary from './ErrorBoundary'

createRoot(document.getElementById('root')!).render(
  <React.StrictMode>
    <BrowserRouter future={{ v7_startTransition: true, v7_relativeSplatPath: true }}>
      <ErrorBoundary>
        <App />
      </ErrorBoundary>
    </BrowserRouter>
  </React.StrictMode>
)

// Global error hooks to show errors in console and help debugging
window.addEventListener('error', (e) => {
  console.error('Global error', e.error || e.message || e)
})
window.addEventListener('unhandledrejection', (e) => {
  console.error('Unhandled promise rejection', e.reason || e)
})
