import React from 'react'

type State = { hasError: boolean; error?: any }

export default class ErrorBoundary extends React.Component<{}, State> {
  constructor(props: {}) {
    super(props)
    this.state = { hasError: false }
  }

  static getDerivedStateFromError(error: any) {
    return { hasError: true, error }
  }

  componentDidCatch(error: any, info: any) {
    // Log to console for now
    console.error('ErrorBoundary caught', error, info)
  }

  render() {
    if (this.state.hasError) {
      return (
        <div style={{padding:20}}>
          <h2>Something went wrong</h2>
          <pre style={{whiteSpace:'pre-wrap',color:'crimson'}}>{String(this.state.error)}</pre>
        </div>
      )
    }
    return this.props.children as any
  }
}
