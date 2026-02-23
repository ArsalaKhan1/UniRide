import React, { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAuth } from '../context/AuthContext'

export default function Landing() {
  const { loginWithGoogle } = useAuth()
  const navigate = useNavigate()

  const [enrollmentId, setEnrollmentId] = useState('')
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  const GSI_CLIENT_ID = (import.meta as any).env?.VITE_GOOGLE_CLIENT_ID || ''

  useEffect(() => {
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
        ;(window as any).google.accounts.id.initialize({
          client_id: GSI_CLIENT_ID,
          callback: handleCredentialResponse,
          ux_mode: 'popup',
          auto_select: false
        })
        ;(window as any).google.accounts.id.renderButton(
          document.getElementById('g_id_signin')!,
          { theme: 'outline', size: 'large', width: 320 }
        )
      } catch {}
    }
  }, [GSI_CLIENT_ID])

  const [pendingCredential, setPendingCredential] = useState<string | null>(null)
  const [googleEmail, setGoogleEmail] = useState<string | null>(null)

  const handleCredentialResponse = async (response: any) => {
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
      setPendingCredential(credential)
      setGoogleEmail(email)
      setError(null)
    } catch (e: any) {
      setError(e?.message || 'Google login failed')
    }
  }

  const confirmEnrollmentForPending = async () => {
    if (!pendingCredential) return setError('No Google credential present')
    if (!enrollmentId) return setError('Please enter your enrollment ID')
    setLoading(true)
    setError(null)
    try {
      await loginWithGoogle(pendingCredential, enrollmentId)
      setPendingCredential(null)
      setGoogleEmail(null)
      navigate('/ride')
    } catch (err: any) {
      setError(err?.message || 'Verification failed')
    } finally {
      setLoading(false)
    }
  }

  return (
    <div style={{ minHeight: '50vh', display: 'flex', flexDirection: 'column' }}>
      {/* Hero Section */}
      <div style={{ flex: 1, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'flex-start', paddingTop: '0.75rem' }}>
        <div style={{ textAlign: 'center', marginBottom: '1.5rem' }}>
          <h1 style={{ fontSize: '3rem', fontWeight: '800', color: '#111827', letterSpacing: '-0.025em', marginBottom: '0.25rem' }}>UniRide</h1>
          <p style={{ fontSize: '1.25rem', color: '#4b5563' }}>University ride-sharing for <span style={{ fontWeight: '600', color: '#2563eb' }}>cloud.neduet.edu.pk</span></p>
        </div>
      </div>

      {/* Feature Cards Section */}
      <section style={{ padding: '0 1rem 1.5rem' }}>
        <div style={{ maxWidth: '72rem', margin: '0 auto', display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(250px, 1fr))', gap: '1rem' }}>
          <div style={{ padding: '1.5rem', backgroundColor: '#dbeafe', borderRadius: '1.5rem', boxShadow: '0 10px 15px -3px rgba(0, 0, 0, 0.1)', border: '2px solid #bfdbfe', textAlign: 'center' }}>
            <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#111827', marginBottom: '0.75rem' }}>🔵 Offer</div>
            <div style={{ fontSize: '1rem', color: '#374151' }}>Share your car or bike ride with classmates heading the same way.</div>
          </div>
          <div style={{ padding: '1.5rem', backgroundColor: '#dbeafe', borderRadius: '1.5rem', boxShadow: '0 10px 15px -3px rgba(0, 0, 0, 0.1)', border: '2px solid #bfdbfe', textAlign: 'center' }}>
            <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#111827', marginBottom: '0.75rem' }}>🔵 Request</div>
            <div style={{ fontSize: '1rem', color: '#374151' }}>Find a car or rickshaw to travel together with fellow students.</div>
          </div>
          <div style={{ padding: '1.5rem', backgroundColor: '#dbeafe', borderRadius: '1.5rem', boxShadow: '0 10px 15px -3px rgba(0, 0, 0, 0.1)', border: '2px solid #bfdbfe', textAlign: 'center' }}>
            <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#111827', marginBottom: '0.75rem' }}>🔵 Chat</div>
            <div style={{ fontSize: '1rem', color: '#374151' }}>Coordinate pickup details and timing with your ride group easily.</div>
          </div>
        </div>
      </section>

      {/* Google Sign-In Section */}
      <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', paddingBottom: '2rem' }}>
        <div style={{ display: 'flex', flexDirection: 'column', alignItems: 'center', marginBottom: '1rem' }}>
          <div id="g_id_signin" />
          {!GSI_CLIENT_ID && (
            <div style={{ marginTop: '0.5rem', fontSize: '0.875rem', color: '#92400e', backgroundColor: '#fef3c7', padding: '0.5rem 0.75rem', borderRadius: '0.25rem' }}>Set VITE_GOOGLE_CLIENT_ID to enable Google Sign-In.</div>
          )}
          {error && error !== 'Server: Invalid Enrollment ID' && (
            <div style={{ marginTop: '0.75rem', fontSize: '0.875rem' }}>
              {(error === 'Server: Invalid token or enrollment ID' || error === 'Please sign in with your @cloud.neduet.edu.pk university email') ? (
                <span style={{ color: '#B91C1C' }}>{error}</span>
              ) : (
                <span>{error}</span>
              )}
            </div>
          )}
        </div>

        {pendingCredential && (
          <div style={{ width: '100%', maxWidth: '28rem', backgroundColor: '#eff6ff', borderRadius: '1rem', boxShadow: '0 25px 50px -12px rgba(0, 0, 0, 0.25)', padding: '2rem', border: '2px solid #bfdbfe' }}>
            <div style={{ fontSize: '1.25rem', fontWeight: '600', color: '#111827', marginBottom: '0.5rem' }}>Google account detected</div>
            <div style={{ fontSize: '0.875rem', color: '#4b5563', marginBottom: '1rem' }}>Signed in as: {googleEmail}</div>
            <div style={{ color: '#1f2937', marginBottom: '0.75rem' }}>Enter your Enrollment ID to complete verification:</div>
            <input
              value={enrollmentId}
              onChange={(e) => setEnrollmentId(e.target.value)}
              style={{ width: '70%', border: '2px solid #93c5fd', borderRadius: '0.75rem', padding: '0.75rem 1rem', fontSize: '1.125rem', outline: 'none' }}
              placeholder="Your enrollment ID"
            />
            {error && (
              <div style={{ marginTop: '0.75rem', fontSize: '0.875rem' }}>
                {(error === 'Server: Invalid Enrollment ID' || error === 'Please sign in with your @cloud.neduet.edu.pk university email') ? (
                  <span style={{ color: '#B91C1C' }}>{error}</span>
                ) : (
                  <span>{error}</span>
                )}
              </div>
            )}
            <div style={{ marginTop: '1.5rem', display: 'flex', justifyContent: 'flex-end', gap: '0.75rem' }}>
              <button onClick={()=>{setPendingCredential(null); setGoogleEmail(null);}} style={{ padding: '0.625rem 1.25rem', borderRadius: '0.75rem', backgroundColor: '#fef2f2', color: 'black', border: 'none', fontWeight: '500', cursor: 'pointer' }}>Cancel</button>
              <button onClick={confirmEnrollmentForPending} disabled={loading} style={{ padding: '0.625rem 1.25rem', borderRadius: '0.75rem', backgroundColor: '#1d4ed8', color: 'white', border: 'none', fontWeight: '500', cursor: 'pointer', opacity: loading ? 0.6 : 1 }}>{loading ? 'Verifying…' : 'Confirm'}</button>
            </div>
          </div>
        )}
      </div>
    </div>
  )
}