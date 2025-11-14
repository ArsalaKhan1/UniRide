import React, { useEffect, useState } from 'react'
import { useAuth } from '../context/AuthContext'
import { rideAPI } from '../api/client'
import { Link } from 'react-router-dom'
import NotificationBanner from '../components/NotificationBanner'

export default function Dashboard() {
  const { user } = useAuth()
  const [myRides, setMyRides] = useState<any[]>([])
  const [pendingRequests, setPendingRequests] = useState<{[rideId:number]: any[]}>({})
  const [acceptedPassengers, setAcceptedPassengers] = useState<{[rideId:number]: any[]}>({})
  const [loading, setLoading] = useState(false)

  const refreshMyRides = async () => {
    if (!user) return
    setLoading(true)
    try {
      const res = await rideAPI.getAll()
      const owned = res.data.rides?.filter((r:any) => r.leadUserID===user.id) || []
      setMyRides(owned)
    } finally {
      setLoading(false)
    }
  }

  useEffect(() => {
    if (!user) return
    refreshMyRides()
    const intv = setInterval(refreshMyRides, 10000)
    return () => clearInterval(intv)
  }, [user])

  useEffect(() => {
    if (!myRides.length) return
    const intervals: number[] = []
    myRides.forEach(ride => {
      const poll = async () => {
        try {
          const res = await rideAPI.getRideRequests(ride.rideID)
          setPendingRequests(pr => ({...pr, [ride.rideID]: res.data.requests || []}))
          // Fetch accepted passengers for this ride
          try {
            const acceptedRes = await rideAPI.getAcceptedPassengers(ride.rideID)
            setAcceptedPassengers(ap => ({...ap, [ride.rideID]: acceptedRes.data.accepted || []}))
          } catch (e) {
            console.error('Failed to fetch accepted passengers', e)
          }
        } catch {}
      }
      poll();
      intervals.push(window.setInterval(poll, 7000))
    })
    return () => intervals.forEach(clearInterval)
  }, [myRides])

  const handleRespond = async (rideID:number, reqUserID:string, accept: boolean) => {
    try {
      await rideAPI.respondToRequest({ rideID, userID: reqUserID, accept })
      setPendingRequests(pr => ({...pr, [rideID]: pr[rideID]?.filter((r:any)=>r.userID!==reqUserID) || []}))
      // Refresh accepted passengers for this ride immediately so the lead sees the chat button
      try {
        const acceptedRes = await rideAPI.getAcceptedPassengers(rideID)
        setAcceptedPassengers(ap => ({...ap, [rideID]: acceptedRes.data.accepted || []}))
      } catch (e) {
        console.error('Failed to fetch accepted passengers after response', e)
      }
      refreshMyRides()
    } catch (e) {
      console.error('Failed to respond to request', e)
    }
  }

  return (
    <div className="max-w-4xl mx-auto p-6">
      <h2 className="text-3xl font-bold mb-6 text-gray-900">Dashboard</h2>
      <NotificationBanner />
      {myRides.length>0 ? myRides.map(ride => {
        const accepted = acceptedPassengers[ride.rideID] || []
        const hasAccepted = accepted.length > 0
        // Show chat button for lead when they have accepted at least one passenger
        return (
          <div key={ride.rideID} className="my-4 p-5 bg-white rounded-xl shadow-lg border border-gray-200">
            <div className="flex items-center justify-between mb-3">
              <div>
                <div className="text-lg font-semibold text-gray-900">Ride #{ride.rideID} to {ride.to}</div>
                <div className="text-sm text-gray-600">Type: {ride.rideType} | Lead: {ride.leadUserName||ride.leadUserID}</div>
              </div>
              {hasAccepted && (
                <Link
                  to={`/chat/${ride.rideID}`}
                  className="px-4 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700"
                >
                  Open Chat
                </Link>
              )}
            </div>
            {accepted.length > 0 && (
              <div className="mb-3 p-3 bg-green-50 rounded-lg">
                <div className="font-semibold text-green-800 mb-1">Accepted Passengers ({accepted.length}):</div>
                <ul className="text-sm text-green-700 space-y-1">
                  {accepted.map((p:any) => (
                    <li key={p.userID} className="flex items-center justify-between">
                      <span>• {p.userName || p.userID}</span>
                      <Link to={`/chat/${ride.rideID}`} className="ml-4 text-sm text-blue-600 underline">Open Chat</Link>
                    </li>
                  ))}
                </ul>
              </div>
            )}
            {pendingRequests[ride.rideID]?.length > 0 && <>
              <div className="mt-3 font-semibold text-gray-800">Pending join requests:</div>
              <ul className="mt-2 space-y-2">
                {pendingRequests[ride.rideID].map((req:any) => (
                  <li key={req.userID} className="flex items-center justify-between p-2 bg-gray-50 rounded">
                    <span className="text-gray-900">User: {req.userName||req.userID}</span>
                    <div className="flex gap-2">
                      <button onClick={()=>handleRespond(ride.rideID, req.userID, true)} className="px-3 py-1 bg-green-600 text-white rounded-lg hover:bg-green-700">Accept</button>
                      <button onClick={()=>handleRespond(ride.rideID, req.userID, false)} className="px-3 py-1 bg-red-600 text-white rounded-lg hover:bg-red-700">Reject</button>
                    </div>
                  </li>
                ))}
              </ul>
            </>}
          </div>
        )
      }):<div className="text-gray-600">{loading ? 'Loading your rides…' : 'No rides created by you yet.'}</div>}
    </div>
  )
}
