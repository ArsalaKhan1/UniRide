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
    // eslint-disable-next-line react-hooks/exhaustive-deps
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
    <div className="max-w-3xl mx-auto px-6 py-16">
      <div className="text-center">
        <h1 className="text-5xl font-extrabold text-gray-900 tracking-tight">UniRide</h1>
        <p className="mt-3 text-lg text-gray-600">University ride-sharing for <span className="font-semibold">cloud.neduet.edu.pk</span></p>
      </div>

      <div className="mt-10 flex flex-col items-center">
        <div id="g_id_signin" />
        {!GSI_CLIENT_ID && (
          <div className="mt-2 text-sm text-amber-700 bg-amber-50 px-3 py-2 rounded">Set VITE_GOOGLE_CLIENT_ID to enable Google Sign-In.</div>
        )}
        {error && error !== 'Server: Invalid Enrollment ID' && (
          <div className="mt-3 text-sm">
            {/* Only color certain server error strings red; do not show 'Server: Invalid Enrollment ID' here */}
            {(error === 'Server: Invalid token or enrollment ID' || error === 'Please sign in with your @cloud.neduet.edu.pk university email') ? (
              <span style={{ color: '#B91C1C' }}>{error}</span>
            ) : (
              <span>{error}</span>
            )}
          </div>
        )}
      </div>

      {pendingCredential && (
        <div className="mt-10 max-w-md mx-auto bg-white rounded-xl shadow-lg p-6">
          <div className="text-lg font-semibold">Google account detected</div>
          <div className="text-sm text-gray-600">Signed in as: {googleEmail}</div>
          <div className="mt-3 text-gray-800">Enter your Enrollment ID to complete verification:</div>
          <input
            value={enrollmentId}
            onChange={(e) => setEnrollmentId(e.target.value)}
            className="mt-2 w-full border border-gray-300 focus:ring-2 focus:ring-blue-500 focus:border-blue-500 rounded-lg px-3 py-2"
            placeholder="Your enrollment ID"
          />
          {error && (
            <div className="mt-3 text-sm">
              {(error === 'Server: Invalid Enrollment ID' || error === 'Please sign in with your @cloud.neduet.edu.pk university email') ? (
                <span style={{ color: '#B91C1C' }}>{error}</span>
              ) : (
                <span>{error}</span>
              )}
            </div>
          )}
          <div className="mt-4 flex justify-end gap-3">
            <button onClick={()=>{setPendingCredential(null); setGoogleEmail(null);}} className="px-4 py-2 rounded-lg bg-gray-100 text-gray-800 hover:bg-gray-200">Cancel</button>
            <button onClick={confirmEnrollmentForPending} disabled={loading} className="px-4 py-2 rounded-lg bg-blue-600 text-white hover:bg-blue-700 disabled:opacity-60">{loading ? 'Verifying…' : 'Confirm'}</button>
          </div>
        </div>
      )}

      <section className="mt-16">
        <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
          <div className="p-6 bg-white rounded-xl shadow">
            <div className="text-lg font-semibold">Offer</div>
            <div className="text-sm text-gray-600 mt-1">Share your car or bike ride with classmates.</div>
          </div>
          <div className="p-6 bg-white rounded-xl shadow">
            <div className="text-lg font-semibold">Request</div>
            <div className="text-sm text-gray-600 mt-1">Find a car or rickshaw to go together.</div>
          </div>
          <div className="p-6 bg-white rounded-xl shadow">
            <div className="text-lg font-semibold">Chat</div>
            <div className="text-sm text-gray-600 mt-1">Coordinate details with your group easily.</div>
          </div>
        </div>
      </section>
    </div>
  )
}
