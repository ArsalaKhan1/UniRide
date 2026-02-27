import React, { useEffect, useState } from 'react'
import { useNavigate } from 'react-router-dom'
import { useAuth } from '../context/AuthContext'

export default function Landing() {
  const { loginWithGoogle, user } = useAuth()
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
      navigate('/rides', { replace: true })
    } catch (err: any) {
      setError(err?.message || 'Verification failed')
    } finally {
      setLoading(false)
    }
  }

  return (
    <div style={{ minHeight: '50vh', display: 'flex', flexDirection: 'column' }}>
      {/* Hero Section */}
      <div style={{ flex: 1, display: 'flex', flexDirection: 'column', alignItems: 'center', justifyContent: 'flex-start', paddingTop: '0.25rem' }}>
        <div style={{ textAlign: 'center', marginBottom: '0.75rem' }}>
          <img src="/logo.png" alt="UniRide Logo" style={{
            height: '130px',
            width: 'auto',
            marginBottom: '0.15rem'
          }} />
          <p style={{ fontSize: '1.25rem', color: '#010102ff' }}>University ride-sharing for <span style={{ fontWeight: '600', color: '#e07d2e' }}>cloud.neduet.edu.pk</span></p>
        </div>
      </div>

      {/* Feature Cards Section */}
      <section style={{ padding: '0 1rem 1.5rem' }}>
        <div style={{ maxWidth: '72rem', margin: '0 auto', display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(250px, 1fr))', gap: '1rem' }}>
          <div style={{ padding: '1.5rem', backgroundColor: '#e07d2e', borderRadius: '1.5rem', boxShadow: '0 10px 15px -3px rgba(0, 0, 0, 0.1)', border: '2px solid #a9abab', textAlign: 'center' }}>
            <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#ffffff', marginBottom: '0.75rem' }}>⚫ Offer</div>
            <div style={{ fontSize: '1rem', color: '#ffffff' }}>Share your car or bike ride with classmates heading the same way.</div>
          </div>
          <div style={{ padding: '1.5rem', backgroundColor: '#e07d2e', borderRadius: '1.5rem', boxShadow: '0 10px 15px -3px rgba(0, 0, 0, 0.1)', border: '2px solid #a9abab', textAlign: 'center' }}>
            <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#ffffff', marginBottom: '0.75rem' }}>⚫ Request</div>
            <div style={{ fontSize: '1rem', color: '#ffffff' }}>Find a car or rickshaw to travel together with fellow students.</div>
          </div>
          <div style={{ padding: '1.5rem', backgroundColor: '#e07d2e', borderRadius: '1.5rem', boxShadow: '0 10px 15px -3px rgba(0, 0, 0, 0.1)', border: '2px solid #a9abab', textAlign: 'center' }}>
            <div style={{ fontSize: '1.5rem', fontWeight: 'bold', color: '#ffffff', marginBottom: '0.75rem' }}>⚫ Chat</div>
            <div style={{ fontSize: '1rem', color: '#ffffff' }}>Coordinate pickup details and timing with your ride group easily.</div>
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
          <div style={{ width: '100%', maxWidth: '28rem', backgroundColor: '#e07d2e', borderRadius: '1rem', boxShadow: '0 25px 50px -12px rgba(0, 0, 0, 0.25)', padding: '2rem', border: '2px solid #a9abab' }}>
            <div style={{ fontSize: '1.25rem', fontWeight: '600', color: '#ffffff', marginBottom: '0.5rem' }}>Google account detected</div>
            <div style={{ fontSize: '0.875rem', color: '#ffffff', marginBottom: '1rem' }}>Signed in as: {googleEmail}</div>
            <div style={{ color: '#ffffff', marginBottom: '0.75rem' }}>Enter your Enrollment ID to complete verification:</div>
            <input
              value={enrollmentId}
              onChange={(e) => setEnrollmentId(e.target.value)}
              style={{ width: '70%', border: '2px solid #c96b1f', borderRadius: '0.75rem', padding: '0.75rem 1rem', fontSize: '1.125rem', outline: 'none', backgroundColor: '#faf8f5', color: '#111827' }}
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
              <button onClick={()=>{setPendingCredential(null); setGoogleEmail(null);}} style={{ padding: '0.625rem 1.25rem', borderRadius: '0.75rem', backgroundColor: '#a9abab', color: '#ffffff', border: 'none', fontWeight: '500', cursor: 'pointer' }}>Cancel</button>
              <button onClick={confirmEnrollmentForPending} disabled={loading} style={{ padding: '0.625rem 1.25rem', borderRadius: '0.75rem', backgroundColor: '#c96b1f', color: 'white', border: 'none', fontWeight: '500', cursor: 'pointer', opacity: loading ? 0.6 : 1 }}>{loading ? 'Verifying…' : 'Confirm'}</button>
            </div>
          </div>
        )}
      </div>
    </div>
  )
}