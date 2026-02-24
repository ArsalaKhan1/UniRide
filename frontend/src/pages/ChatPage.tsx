import React, { useEffect, useState, useRef } from 'react'
import { useParams } from 'react-router-dom'
import { chatAPI, rideAPI } from '../api/client'
import { useAuth } from '../context/AuthContext'

type Message = {
  sender: string
  recipient: string
  text: string
  timestamp?: string
}

export default function ChatPage() {
  const { rideId } = useParams()
  const { user } = useAuth()
  const currentUserId = user ? String(user.id) : ''
  const [messages, setMessages] = useState<Message[]>([])
  const [text, setText] = useState('')
  const [loading, setLoading] = useState(false)
  const [rideLeadID, setRideLeadID] = useState<string | null>(null)
  const [userNames, setUserNames] = useState<Record<string, string>>({})
  const [error, setError] = useState<string | null>(null)
  const [rideStatus, setRideStatus] = useState<string>('open')
  const messagesEndRef = useRef<HTMLDivElement>(null)

  // Get ride info to determine lead userID
  useEffect(() => {
    if (!rideId) return
    const fetchRideInfo = async () => {
      try {
        const res = await rideAPI.getAll()
        const ride = res.data.rides?.find((r: any) => r.rideID === Number(rideId))
        if (ride) {
          setRideLeadID(ride.leadUserID || ride.ownerID)
          setRideStatus(ride.status || 'open')
          // prime user name map with lead and current user
          setUserNames(prev => ({ ...prev, [ride.leadUserID || ride.ownerID]: ride.leadUserName || '', [String(user?.id || '')]: user?.name || '' }))
          // fetch accepted passengers to get their names
          try {
            const acc = await rideAPI.getAcceptedPassengers(Number(rideId))
            const accepted = acc.data.accepted || []
            const mapUpdate: Record<string, string> = {}
            accepted.forEach((p: any) => { mapUpdate[String(p.userID)] = p.userName })
            setUserNames(prev => ({ ...prev, ...mapUpdate }))
          } catch (e) {
            // ignore
          }
        }
      } catch (e) {
        console.error('Failed to fetch ride info', e)
      }
    }
    fetchRideInfo()
  }, [rideId])

  const fetch = async () => {
    if (!rideId) return
    try {
      const res = await chatAPI.byRide(Number(rideId))
      const data = res.data
      setMessages(data.messages || [])
    } catch (e) {
      console.error('Failed to fetch chat', e)
    }
  }

  const formatTime = (timestamp?: string) => {
    if (!timestamp) return ''
    const date = new Date(timestamp)
    return date.toLocaleTimeString('en-US', { hour: 'numeric', minute: '2-digit', hour12: true })
  }

  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: 'smooth' })
  }, [messages])

  useEffect(() => {
    document.body.style.overflow = 'hidden'
    return () => {
      document.body.style.overflow = ''
    }
  }, [])

  // Periodically refresh all participant usernames to ensure user 3 can see user 2's name
  const fetchAllParticipants = async () => {
    if (!rideId) return
    try {
      // Fetch accepted passengers only
      try {
        const acc = await rideAPI.getAcceptedPassengers(Number(rideId))
        const accepted = acc.data.accepted || []
        const mapUpdate: Record<string, string> = {}
        accepted.forEach((p: any) => { mapUpdate[String(p.userID)] = p.userName })
        setUserNames(prev => ({ ...prev, ...mapUpdate }))
      } catch (e) {
        // ignore
      }
    } catch (e) {
      console.error('Failed to fetch all participants', e)
    }
  }

  useEffect(() => {
    fetch()
    const id = setInterval(() => {
      fetch()
      // Also refresh ride status
      if (rideId) {
        rideAPI.getAll().then(res => {
          const ride = res.data.rides?.find((r: any) => r.rideID === Number(rideId))
          if (ride) {
            setRideStatus(ride.status || 'open')
          }
        }).catch(() => { })
      }
    }, 5000)
    return () => clearInterval(id)
  }, [rideId])

  useEffect(() => {
    if (!rideId) return
    // Refresh participant names every 3 seconds to catch late-joining users
    fetchAllParticipants()
    const id = setInterval(fetchAllParticipants, 3000)
    return () => clearInterval(id)
  }, [rideId])

  const handleSend = async (e?: React.FormEvent) => {
    e?.preventDefault()
    if (!user || !rideId || !text.trim()) return
    if (!rideLeadID) {
      setError('Loading ride information...')
      return
    }
    if (rideStatus === 'completed') {
      setError('Cannot send messages for a completed ride')
      return
    }

    setLoading(true)
    setError(null)
    try {
      // Hub-and-spoke model: if user is lead, send with empty recipient (broadcast)
      // If user is passenger, send to lead
      const recipient = currentUserId === (rideLeadID || '') ? '' : rideLeadID
      const res = await chatAPI.send({
        sender: user.id,
        recipient: recipient,
        text: text.trim(),
        rideID: Number(rideId)
      })

      if (res.data.success) {
        setText('')
        fetch()
      } else {
        setError(res.data.error || 'Failed to send message')
      }
    } catch (e: any) {
      setError(e?.response?.data?.error || e?.message || 'Failed to send message')
    } finally {
      setLoading(false)
    }
  }

  if (!user) {
    return <div style={{ padding: '20px', color: '#dc2626' }}>Please sign in to use chat.</div>
  }

  const isCompleted = rideStatus === 'completed'
  const participantsList = Object.entries(userNames).filter(([id]) => id !== rideLeadID && id !== '')

  return (
    <div style={{
      display: 'flex',
      height: 'calc(100vh - 64px)',
      overflow: 'hidden',
      fontFamily: '-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif'
    }}>
      {/* Left Panel */}
      <div style={{
        width: '350px',
        backgroundColor: '#ffffff',
        borderRight: '1px solid #e5e7eb',
        overflowY: 'auto',
        display: 'flex',
        flexDirection: 'column'
      }}>
        {/* Ride Lead Section */}
        <div style={{
          padding: '24px',
          borderBottom: '2px solid #e5e7eb',
          backgroundColor: '#f0f9ff'
        }}>
          <div style={{ fontSize: '11px', fontWeight: '600', color: '#6b7280', letterSpacing: '0.5px', marginBottom: '12px' }}>RIDE LEAD</div>
          <div style={{ fontSize: '20px', fontWeight: '700', color: '#111827' }}>
            {userNames[rideLeadID || ''] || 'Loading...'}
          </div>
        </div>

        {/* Ride Status Section */}
        <div style={{ padding: '20px 24px', borderBottom: '1px solid #e5e7eb' }}>
          <div style={{ fontSize: '11px', fontWeight: '600', color: '#6b7280', letterSpacing: '0.5px', marginBottom: '8px' }}>RIDE STATUS</div>
          <div style={{ fontSize: '15px', fontWeight: '600', color: isCompleted ? '#dc2626' : '#059669', textTransform: 'capitalize' }}>
            {rideStatus}
          </div>
        </div>

        {/* Participants Section */}
        <div style={{ padding: '20px 24px' }}>
          <div style={{ fontSize: '11px', fontWeight: '600', color: '#6b7280', letterSpacing: '0.5px', marginBottom: '12px' }}>PARTICIPANTS</div>
          {participantsList.length === 0 ? (
            <div style={{ fontSize: '14px', color: '#9ca3af', fontStyle: 'italic' }}>No participants yet</div>
          ) : (
            <div style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
              {participantsList.map(([id, name]) => (
                <div key={id} style={{ fontSize: '15px', color: '#374151', padding: '4px 0' }}>
                  {name || id}
                </div>
              ))}
            </div>
          )}
        </div>
      </div>

      {/* Right Panel - Chat */}
      <div style={{
        flex: 1,
        display: 'flex',
        flexDirection: 'column',
        overflow: 'hidden'
      }}>
        {/* Messages Area */}
        <div style={{
          flex: 1,
          overflowY: 'auto',
          padding: '20px',
          backgroundColor: '#f9fafb'
        }}>
          {messages.length === 0 ? (
            <div style={{ textAlign: 'center', color: '#9ca3af', padding: '40px 0' }}>
              No messages yet. Start the conversation!
            </div>
          ) : (
            messages.map((m, i) => {
              const isOwnMessage = m.sender === currentUserId
              const senderName = userNames[m.sender] || m.sender
              return (
                <div key={i} style={{
                  display: 'flex',
                  flexDirection: 'column',
                  alignItems: isOwnMessage ? 'flex-end' : 'flex-start',
                  marginBottom: '6px'
                }}>
                  {!isOwnMessage && (
                    <div style={{ fontSize: '11px', fontWeight: 700, color: '#4d525e', marginBottom: '4px', paddingLeft: '12px' }}>
                      {senderName}
                    </div>
                  )}
                  <div style={{
                    maxWidth: '60%',
                    padding: '12px 16px',
                    borderRadius: '14px',
                    backgroundColor: isOwnMessage ? '#2563eb' : '#c1c9d1',
                    color: isOwnMessage ? '#ffffff' : '#111827',
                    wordWrap: 'break-word',
                    fontSize: '16px',
                    lineHeight: '1'
                  }}>
                    {m.text}
                  </div>
                  <div style={{ fontSize: '10px', color: '#9ca3af', marginTop: '4px', paddingLeft: '12px', paddingRight: '12px' }}>
                    {formatTime(m.timestamp)}
                  </div>
                </div>
              )
            })
          )}
          <div ref={messagesEndRef} />
        </div>

        {/* Input Area */}
        <div style={{
          flexShrink: 0,
          padding: '20px',
          borderTop: '1px solid #e5e7eb',
          backgroundColor: '#ffffff'
        }}>
          {isCompleted && (
            <div style={{
              marginBottom: '12px',
              padding: '12px',
              backgroundColor: '#fef3c7',
              border: '1px solid #fbbf24',
              borderRadius: '8px',
              fontSize: '14px',
              color: '#92400e'
            }}>
              This ride has been completed. Chat is disabled.
            </div>
          )}
          {error && (
            <div style={{
              marginBottom: '12px',
              padding: '12px',
              backgroundColor: '#fee2e2',
              border: '1px solid #ef4444',
              borderRadius: '8px',
              fontSize: '14px',
              color: '#991b1b'
            }}>
              {error}
            </div>
          )}
          <form onSubmit={handleSend} style={{ display: 'flex', gap: '12px' }}>
            <input
              value={text}
              onChange={(e) => setText(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && !e.shiftKey && handleSend()}
              style={{
                flex: 1,
                padding: '12px 16px',
                border: '1px solid #d1d5db',
                borderRadius: '24px',
                fontSize: '15px',
                outline: 'none',
                backgroundColor: isCompleted ? '#f3f4f6' : '#ffffff',
                cursor: isCompleted ? 'not-allowed' : 'text'
              }}
              placeholder={isCompleted ? "Chat disabled - ride completed" : "Type a message..."}
              disabled={loading || !rideLeadID || isCompleted}
              onFocus={(e) => !isCompleted && (e.target.style.borderColor = '#3b82f6')}
              onBlur={(e) => e.target.style.borderColor = '#d1d5db'}
            />
            <button
              type="submit"
              disabled={loading || !text.trim() || !rideLeadID || isCompleted}
              style={{
                padding: '12px 32px',
                borderRadius: '24px',
                border: 'none',
                backgroundColor: (loading || !text.trim() || !rideLeadID || isCompleted) ? '#9ca3af' : '#3b82f6',
                color: '#ffffff',
                fontSize: '15px',
                fontWeight: '500',
                cursor: (loading || !text.trim() || !rideLeadID || isCompleted) ? 'not-allowed' : 'pointer',
                transition: 'background-color 0.2s'
              }}
              onMouseEnter={(e) => {
                if (!loading && text.trim() && rideLeadID && !isCompleted) {
                  e.currentTarget.style.backgroundColor = '#2563eb'
                }
              }}
              onMouseLeave={(e) => {
                if (!loading && text.trim() && rideLeadID && !isCompleted) {
                  e.currentTarget.style.backgroundColor = '#3b82f6'
                }
              }}
            >
              {loading ? 'Sending...' : 'Send'}
            </button>
          </form>
        </div>
      </div>
    </div>
  )
}
