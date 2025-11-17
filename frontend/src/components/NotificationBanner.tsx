import React, { useEffect, useState } from 'react'
import { useAuth } from '../context/AuthContext'
import { userAPI } from '../api/client'
import { Link } from 'react-router-dom'

type AcceptedRequest = {
  rideID: number
  from: string
  to: string
  rideType: string
  leadUserID: string
  leadUserName?: string
}

export default function NotificationBanner() {
  const { user } = useAuth()
  const [acceptedRequests, setAcceptedRequests] = useState<AcceptedRequest[]>([])

  useEffect(() => {
    if (!user) return

    const poll = async () => {
      try {
        const res = await userAPI.getAcceptedRequests(user.id)
        setAcceptedRequests(res.data.acceptedRequests || [])
      } catch (e) {
        console.error('Failed to fetch accepted requests', e)
      }
    }

    poll()
    const interval = setInterval(poll, 5000)
    return () => clearInterval(interval)
  }, [user])

  if (!acceptedRequests.length) return null

  return (
    <div className="space-y-4 mb-6">
      {acceptedRequests.map((req) => (
        <div key={req.rideID} className="bg-green-50 border-2 border-green-300 p-6 rounded-3xl shadow-lg">
          <div className="flex items-center justify-between">
            <div>
              <div className="font-semibold text-green-800 text-lg">Request Accepted!</div>
              <div className="text-sm text-green-700 mt-2">
                Your request to join ride #{req.rideID} ({req.from} → {req.to}) has been accepted.
                Waiting for others to join before the ride starts.
              </div>
            </div>

            <Link
              to={`/chat/${req.rideID}`}
              className="text-sm text-blue-700 underline font-medium"
            >
              Open Chat
            </Link>
          </div>
        </div>
      ))}
    </div>
  )
}