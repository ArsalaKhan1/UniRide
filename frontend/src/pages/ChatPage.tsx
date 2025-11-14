import React, { useEffect, useState } from 'react'
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
  const [userNames, setUserNames] = useState<Record<string,string>>({})
  const [error, setError] = useState<string | null>(null)

  // Get ride info to determine lead userID
  useEffect(() => {
    if (!rideId) return
    const fetchRideInfo = async () => {
      try {
        const res = await rideAPI.getAll()
        const ride = res.data.rides?.find((r: any) => r.rideID === Number(rideId))
        if (ride) {
          setRideLeadID(ride.leadUserID || ride.ownerID)
          // prime user name map with lead and current user
          setUserNames(prev => ({...prev, [ride.leadUserID || ride.ownerID]: ride.leadUserName || '' , [String(user?.id || '')]: user?.name || ''}))
          // fetch accepted passengers to get their names
          try {
            const acc = await rideAPI.getAcceptedPassengers(Number(rideId))
            const accepted = acc.data.accepted || []
            const mapUpdate: Record<string,string> = {}
            accepted.forEach((p:any) => { mapUpdate[String(p.userID)] = p.userName })
            setUserNames(prev => ({...prev, ...mapUpdate}))
          } catch (e) {
            // ignore
          }
          // Also fetch pending requests to include their senders in the username map
          try {
            const pending = await rideAPI.getRideRequests(Number(rideId))
            const requests = pending.data.requests || []
            const mapUpdate: Record<string,string> = {}
            requests.forEach((req:any) => { mapUpdate[String(req.userID)] = req.userName || req.userID })
            setUserNames(prev => ({...prev, ...mapUpdate}))
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

  // Periodically refresh all participant usernames to ensure user 3 can see user 2's name
  const fetchAllParticipants = async () => {
    if (!rideId) return
    try {
      // Fetch accepted passengers
      try {
        const acc = await rideAPI.getAcceptedPassengers(Number(rideId))
        const accepted = acc.data.accepted || []
        const mapUpdate: Record<string,string> = {}
        accepted.forEach((p:any) => { mapUpdate[String(p.userID)] = p.userName })
        setUserNames(prev => ({...prev, ...mapUpdate}))
      } catch (e) {
        // ignore
      }
      // Also fetch pending requests
      try {
        const pending = await rideAPI.getRideRequests(Number(rideId))
        const requests = pending.data.requests || []
        const mapUpdate: Record<string,string> = {}
        requests.forEach((req:any) => { mapUpdate[String(req.userID)] = req.userName || req.userID })
        setUserNames(prev => ({...prev, ...mapUpdate}))
      } catch (e) {
        // ignore
      }
    } catch (e) {
      console.error('Failed to fetch all participants', e)
    }
  }

  useEffect(() => {
    fetch()
    const id = setInterval(fetch, 5000)
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
    return <div className="text-red-600">Please sign in to use chat.</div>
  }

  return (
    <div className="max-w-3xl mx-auto p-6">
      <h2 className="text-2xl font-bold mb-4 text-gray-900">Chat for Ride #{rideId}</h2>
      <div className="bg-white rounded-xl shadow-lg p-6">
        <div className="mb-4 h-96 overflow-y-auto border border-gray-200 rounded-lg p-4 bg-gray-50">
          {messages.length === 0 ? (
            <div className="text-gray-500 text-center py-8">No messages yet. Start the conversation!</div>
          ) : (
            messages.map((m, i) => {
              const isOwnMessage = m.sender === currentUserId
              const senderName = isOwnMessage ? 'You' : (userNames[m.sender] || m.sender)
              // Align own messages to the left as requested; recipient messages also left-aligned
              return (
                <div key={i} className="mb-4 flex flex-col items-start">
                  <div className="flex items-center text-xs text-gray-600 mb-1">
                    <span className="font-medium mr-2">{senderName}</span>  
                    {m.timestamp && <span className="text-gray-500">{m.timestamp}</span>}
                  </div>
                  <div className={`inline-block max-w-xs lg:max-w-md px-4 py-2 rounded-lg ${isOwnMessage ? 'bg-blue-600' : 'bg-white border border-gray-300'}`}>
                    <div className="text-gray-900">{m.text}</div>
                  </div>
                </div>
              )
            })
          )}
        </div>

        {error && <div className="mb-3 text-red-600 text-sm">{error}</div>}

        <form onSubmit={handleSend} className="flex gap-2">
          <input 
            value={text} 
            onChange={(e) => setText(e.target.value)}
            onKeyPress={(e) => e.key === 'Enter' && !e.shiftKey && handleSend()}
            className="flex-1 border border-gray-300 rounded-lg px-4 py-2 focus:ring-2 focus:ring-blue-500 focus:border-blue-500" 
            placeholder="Type a message..." 
            disabled={loading || !rideLeadID}
          />
          <button 
            type="submit"
            disabled={loading || !text.trim() || !rideLeadID} 
            className="px-6 py-2 rounded-lg bg-blue-600 text-white hover:bg-blue-700 disabled:opacity-60"
          >
            {loading ? 'Sending...' : 'Send'}
          </button>
        </form>
      </div>
    </div>
  )
}
