import React, { useEffect, useState } from 'react'
import { useParams } from 'react-router-dom'
import { chatAPI } from '../api/client'
import { useAuth } from '../context/AuthContext'

type Message = {
  sender: string
  text: string
}

export default function ChatPage() {
  const { rideId } = useParams()
  const { user } = useAuth()
  const [messages, setMessages] = useState<Message[]>([])
  const [text, setText] = useState('')
  const [loading, setLoading] = useState(false)

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

  useEffect(() => {
    fetch()
    // simple polling every 5s
    const id = setInterval(fetch, 5000)
    return () => clearInterval(id)
  }, [rideId])

  const handleSend = async () => {
    if (!user) return alert('Please sign in')
    if (!rideId) return
    try {
      await chatAPI.send({ sender: user.id, recipient: '', text, rideID: Number(rideId) })
      setText('')
      fetch()
    } catch (e: any) {
      alert(e?.message || 'Failed to send')
    }
  }

  return (
    <div>
      <h2 className="text-2xl font-semibold mb-4">Chat for ride {rideId}</h2>
      <div className="p-4 bg-white rounded shadow">
        <div style={{maxHeight:300, overflow:'auto'}} className="mb-3">
          {messages.map((m, i) => (
            <div key={i} className="mb-2">
              <div className="text-sm font-semibold">{m.sender}</div>
              <div>{m.text}</div>
            </div>
          ))}
        </div>

        <div className="flex gap-2">
          <input value={text} onChange={(e) => setText(e.target.value)} className="flex-1 border rounded px-2 py-1" placeholder="Type a message" />
          <button onClick={handleSend} className="px-3 py-1 rounded bg-primary text-white">Send</button>
        </div>
      </div>
    </div>
  )
}
