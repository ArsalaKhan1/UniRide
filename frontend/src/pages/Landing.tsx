import React, { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAuth } from '../context/AuthContext'

export default function Landing() {
  const { loginWithGoogle } = useAuth()
  const navigate = useNavigate()

  const [enrollmentId, setEnrollmentId] = useState('')
  const [idToken, setIdToken] = useState('')
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  const GSI_CLIENT_ID = (import.meta as any).env?.VITE_GOOGLE_CLIENT_ID || ''

  useEffect(() => {
    console.log('[Landing] GSI_CLIENT_ID =', GSI_CLIENT_ID)
    // Load Google Identity Services script if not already present
    if (!GSI_CLIENT_ID) return
    if ((window as any).google && (window as any).google.accounts) {
      tryRenderButton()
      return
    }

    const script = document.createElement('script')
    script.src = 'https://accounts.google.com/gsi/client'
    script.async = true
    script.defer = true
    script.onload = () => {
      tryRenderButton()
    }
    document.head.appendChild(script)

    function tryRenderButton() {
      try {
        console.log('[Landing] initializing GSI with client_id=', GSI_CLIENT_ID)
        ;(window as any).google.accounts.id.initialize({
          client_id: GSI_CLIENT_ID,
          callback: handleCredentialResponse,
          ux_mode: 'popup',
          auto_select: false
        })
        ;(window as any).google.accounts.id.renderButton(
          document.getElementById('g_id_signin')!,
          { theme: 'outline', size: 'large' }
        )
      } catch (e) {
        // ignore - script may not be fully ready
      }
    }
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [GSI_CLIENT_ID])

  // Callback from Google Identity Services
  // New flow: after receiving credential and validating domain, we DO NOT infer enrollment ID from email.
  // Instead we prompt the user to enter their enrollment ID, then call backend /auth/google with both idToken and enrollmentId.
  const [pendingCredential, setPendingCredential] = useState<string | null>(null)
  const [googleEmail, setGoogleEmail] = useState<string | null>(null)

  const handleCredentialResponse = async (response: any) => {
    console.log('[Landing] handleCredentialResponse', response)
    const credential = response?.credential
    if (!credential) return
    try {
      const parts = credential.split('.')
      if (parts.length < 2) throw new Error('Invalid token')
      const payload = JSON.parse(window.atob(parts[1]))
      const email: string = payload?.email || ''
      if (!email.endsWith('@cloud.neduet.edu.pk')) {
        setError('Please sign in with your @cloud.neduet.edu.pk university email')
        return
      }
      // Save credential and email, then ask user to enter their enrollment ID (stored separately in DB)
      setPendingCredential(credential)
      setGoogleEmail(email)
      setError(null)
    } catch (e: any) {
      setError(e?.message || 'Google login failed')
    }
  }

  const handleLogin = async (e?: React.FormEvent) => {
    e?.preventDefault()
    setLoading(true)
    setError(null)
    console.log('[Landing] handleLogin called, idToken:', idToken, 'enrollmentId:', enrollmentId)
    try {
      // Manual fallback: if user pasted an idToken and enrollmentId
      await loginWithGoogle(idToken || 'dev-token', enrollmentId || 'dev-user')
      navigate('/dashboard')
    } catch (err: any) {
      setError(err?.message || 'Login failed')
    } finally {
      setLoading(false)
    }
  }

  const confirmEnrollmentForPending = async () => {
    console.log('[Landing] confirmEnrollmentForPending called, pendingCredential:', pendingCredential, 'enrollmentId:', enrollmentId)
    if (!pendingCredential) return setError('No Google credential present')
    if (!enrollmentId) return setError('Please enter your enrollment ID')
    setLoading(true)
    setError(null)
    try {
      await loginWithGoogle(pendingCredential, enrollmentId)
      // clear pending
      setPendingCredential(null)
      setGoogleEmail(null)
      navigate('/dashboard')
    } catch (err: any) {
      setError(err?.message || 'Verification failed')
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="max-w-2xl mx-auto text-center">
      <h1 className="text-4xl font-bold text-primary mb-4">UniRide</h1>
      <p className="mb-6">University ride-sharing for cloud.neduet.edu.pk</p>

      <form onSubmit={handleLogin} className="space-y-3 text-left mx-auto" style={{maxWidth:400}}>
        <label className="block text-sm">Enrollment ID</label>
        <input value={enrollmentId} onChange={(e) => setEnrollmentId(e.target.value)} className="w-full border rounded px-2 py-1" placeholder="e.g. student123" />

        <label className="block text-sm">Google ID Token (or use dev token)</label>
        <input value={idToken} onChange={(e) => setIdToken(e.target.value)} className="w-full border rounded px-2 py-1" placeholder="paste idToken or leave blank for dev" />

        {error && <div className="text-red-600">{error}</div>}

        <div className="text-center">
          <button disabled={loading} className="px-6 py-2 rounded bg-primary text-white">{loading ? 'Signing in…' : 'Sign in with Google'}</button>
        </div>
      </form>

      <div className="mt-4 text-center">
        <div id="g_id_signin" />
        {!GSI_CLIENT_ID && (
          <div className="mt-2 text-sm text-gray-600">Set VITE_GOOGLE_CLIENT_ID in your environment to enable Google Sign-In.</div>
        )}

        {pendingCredential && (
          <div className="mt-4 p-3 bg-white rounded shadow text-left max-w-md mx-auto">
            <div className="font-semibold">Google account detected</div>
            <div className="text-sm text-gray-700">Signed in as: {googleEmail}</div>
            <div className="mt-2">Please enter your Enrollment ID (stored in university records) to complete verification:</div>
            <input value={enrollmentId} onChange={(e) => setEnrollmentId(e.target.value)} className="w-full border rounded px-2 py-1 mt-2" placeholder="Enter your enrollment ID" />
            <div className="mt-3 text-right">
              <button onClick={confirmEnrollmentForPending} disabled={loading} className="px-4 py-2 rounded bg-primary text-white">{loading ? 'Verifying…' : 'Confirm Enrollment ID'}</button>
            </div>
          </div>
        )}
      </div>

      <section className="mt-10 text-left">
        <h2 className="text-2xl font-semibold mb-2">How it works</h2>
        <ol className="list-decimal list-inside space-y-2">
          <li>Sign in with your university email</li>
          <li>Offer or join rides</li>
          <li>Chat with ride participants</li>
        </ol>
      </section>
    </div>
  )
}
