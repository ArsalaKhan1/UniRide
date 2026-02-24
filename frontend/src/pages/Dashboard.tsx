import React, { useEffect, useState } from 'react'
import { useAuth } from '../context/AuthContext'
import { rideAPI } from '../api/client'
import { Link } from 'react-router-dom'
import NotificationBanner from '../components/NotificationBanner'

export default function Dashboard() {
  const { user } = useAuth()
  const [myRides, setMyRides] = useState<any[]>([])
  const [pendingRequests, setPendingRequests] = useState<{ [rideId: number]: any[] }>({})
  const [acceptedPassengers, setAcceptedPassengers] = useState<{ [rideId: number]: any[] }>({})
  const [loading, setLoading] = useState(false)

  const refreshMyRides = async () => {
    if (!user) return
    setLoading(true)
    try {
      const res = await rideAPI.getAll()
      const owned = res.data.rides?.filter((r: any) => r.leadUserID === user.id) || []
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
          setPendingRequests(pr => ({ ...pr, [ride.rideID]: res.data.requests || [] }))
          // Fetch accepted passengers for this ride
          try {
            const acceptedRes = await rideAPI.getAcceptedPassengers(ride.rideID)
            setAcceptedPassengers(ap => ({ ...ap, [ride.rideID]: acceptedRes.data.accepted || [] }))
          } catch (e) {
            console.error('Failed to fetch accepted passengers', e)
          }
        } catch { }
      }
      poll();
      intervals.push(window.setInterval(poll, 7000))
    })
    return () => intervals.forEach(clearInterval)
  }, [myRides])

  const handleRespond = async (rideID: number, reqUserID: string, accept: boolean) => {
    try {
      await rideAPI.respondToRequest({ rideID, userID: reqUserID, accept })
      setPendingRequests(pr => ({ ...pr, [rideID]: pr[rideID]?.filter((r: any) => r.userID !== reqUserID) || [] }))
      // Refresh accepted passengers for this ride immediately so the lead sees the chat button
      try {
        const acceptedRes = await rideAPI.getAcceptedPassengers(rideID)
        setAcceptedPassengers(ap => ({ ...ap, [rideID]: acceptedRes.data.accepted || [] }))
      } catch (e) {
        console.error('Failed to fetch accepted passengers after response', e)
      }
      refreshMyRides()
    } catch (e) {
      console.error('Failed to respond to request', e)
    }
  }

  const handleStartRide = async (rideID: number) => {
    if (!user) return
    try {
      await rideAPI.startRide(rideID, String(user.id))
      setMyRides(rides => rides.map(r => r.rideID === rideID ? { ...r, status: 'started' } : r))
      refreshMyRides()
    } catch (e: any) {
      const errorMsg = e?.response?.data?.error
      if (errorMsg === 'Ride is already started') {
        setMyRides(rides => rides.map(r => r.rideID === rideID ? { ...r, status: 'started' } : r))
        return
      }
      alert(errorMsg || 'Failed to start ride')
    }
  }

  const handleEndRide = async (rideID: number) => {
    if (!user) return
    if (!confirm('Are you sure you want to end this ride? This will disable chat for all participants.')) {
      return
    }
    try {
      await rideAPI.endRide(rideID, String(user.id))
      refreshMyRides()
    } catch (e: any) {
      alert(e?.response?.data?.error || 'Failed to end ride')
    }
  }


  return (
    <div className="max-w-6xl mx-auto p-10">
      <h2 className="text-3xl font-bold mb-8 text-gray-900">Dashboard</h2>
      <NotificationBanner />
      {myRides.length > 0 ? myRides.map(ride => {
        const accepted = acceptedPassengers[ride.rideID] || []
        const hasAccepted = accepted.length > 0
        const hasPendingRequests = (pendingRequests[ride.rideID] || []).length > 0
        const rideStatus = ride.status || 'open'
        const isStarted = rideStatus === 'started'
        const isCompleted = rideStatus === 'completed'
        const canStart = !isStarted && !isCompleted
        const canEnd = isStarted && !isCompleted
        const showChat = (hasAccepted || hasPendingRequests || isStarted) && !isCompleted

        return (
          <div key={ride.rideID} className="my-6 p-8 bg-blue-100 rounded-3xl shadow-xl border-2 border-blue-200">
            <div className="flex items-center justify-between mb-4">
              <div>
                <div className="text-xl font-semibold text-gray-900">Ride #{ride.rideID} to {ride.to}</div>
                <div className="text-sm text-gray-600 mt-1">
                  Type: {ride.rideType} | Lead: {ride.leadUserName || ride.leadUserID} |
                  Status: <span className={`font-semibold ${rideStatus === 'started' ? 'text-green-600' :
                    rideStatus === 'completed' ? 'text-gray-500' :
                      'text-blue-600'
                    }`}>
                    {rideStatus.toUpperCase()}
                  </span>
                </div>
              </div>
              <div className="flex gap-3">
                {canStart && (
                  <div>
                    <button
                      onClick={() => handleStartRide(ride.rideID)}
                      className="px-5 py-2.5 bg-green-600 text-white rounded-xl hover:bg-green-600 font-medium"
                    >
                      Start Ride
                    </button>
                  </div>
                )}

                {canEnd && (
                  <div>
                    <button
                      onClick={() => handleEndRide(ride.rideID)}
                      className="px-5 py-2.5 bg-red-600 text-white rounded-xl hover:bg-red-600 font-medium"
                    >
                      End Ride
                    </button>
                  </div>
                )}

                {showChat && (
                  <div>
                    <Link
                      to={`/chat/${ride.rideID}`}
                      className="text-sm text-blue-700 underline font-medium"
                    >
                      Open Chat
                    </Link>
                  </div>
                )}
              </div>
              </div>
              {accepted.length > 0 && (
                <div className="mb-4 p-5 bg-green-50 rounded-xl border-2 border-green-200">
                  <div className="font-semibold text-green-800 mb-2">
                    Accepted Passengers ({accepted.length}):
                  </div>

                  <ul className="text-sm text-green-700 space-y-2">
                    {accepted.map((p: any) => (
                      <li key={p.userID} className="flex items-center gap-3">
                        <span>• {p.userName || p.userID}</span>
                      </li>
                    ))}
                  </ul>
                </div>
              )}

              {pendingRequests[ride.rideID]?.length > 0 && <>
                <div className="mt-4 font-semibold text-gray-800 mb-3">Pending join requests:</div>
                <ul className="mt-2 space-y-3">
                  {pendingRequests[ride.rideID].map((req: any) => (
                    <li key={req.userID} className="flex items-center justify-between p-4 bg-green rounded-xl border-2 border-blue-200">
                      <span className="text-gray-900 font-medium">User: {req.userName || req.userID}</span>
                      <div className="flex gap-3">
                        <button onClick={() => handleRespond(ride.rideID, req.userID, true)} className="px-4 py-2 bg-green-100 text-white rounded-xl hover:bg-green-100 font-medium">Accept</button>
                        <button onClick={() => handleRespond(ride.rideID, req.userID, false)} className="px-4 py-2 bg-red-100 text-white rounded-xl hover:bg-red-100 font-medium">Reject</button>
                      </div>
                    </li>
                  ))}
                </ul>
              </>}
            </div>
            )
      }) : <div className="text-gray-600">{loading ? 'Loading your rides…' : 'No rides created by you yet.'}</div>}
          </div >
        )
      }
