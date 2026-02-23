import React, { useEffect, useState } from 'react'
import { useAuth } from '../context/AuthContext'
import { rideAPI } from '../api/client'
import { Link } from 'react-router-dom'
import NotificationBanner from '../components/NotificationBanner'

export default function CurrentRide() {
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
      refreshMyRides()
    } catch (e: any) {
      alert(e?.response?.data?.error || 'Failed to start ride')
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

  const activeRides = myRides.filter(r => r.status !== 'completed')

  return (
    <div style={{ margin: '0 auto', padding: '10px', maxWidth: '800px' }}>
      <h2 style={{ fontSize: '32px', fontWeight: 'bold', marginBottom: '32px', color: '#111827', textAlign: 'center' }}>Current Ride</h2>
      <NotificationBanner />
      {activeRides.length > 0 ? activeRides.map(ride => {
        const accepted = acceptedPassengers[ride.rideID] || []
        const hasAccepted = accepted.length > 0
        const hasPendingRequests = (pendingRequests[ride.rideID] || []).length > 0
        const rideStatus = ride.status || 'open'
        const isStarted = rideStatus === 'started'
        const isCompleted = rideStatus === 'completed'
        const canStart = !isStarted && !isCompleted
        const canEnd = isStarted && !isCompleted
        const showChat = (hasAccepted || hasPendingRequests || isStarted) && !isCompleted
        const statusColor = rideStatus === 'started' ? '#059669' : rideStatus === 'completed' ? '#6b7280' : '#2563eb'

        return (
          <div key={ride.rideID} style={{ margin: '24px 0', padding: '32px', backgroundColor: '#dbeafe', borderRadius: '24px', boxShadow: '0 10px 15px -3px rgba(0, 0, 0, 0.1)', border: '2px solid #bfdbfe' }}>
            <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', marginBottom: '16px' }}>
              <div>
                <div style={{ fontSize: '20px', fontWeight: '600', color: '#111827' }}>Ride - {ride.from} to {ride.to}</div>
                <div style={{ fontSize: '14px', color: '#4b5563', marginTop: '4px' }}>
                  Type: {ride.rideType} | Lead: {ride.leadUserName || ride.leadUserID} |
                  Status: <span style={{ fontWeight: '600', color: statusColor }}>
                    {rideStatus.toUpperCase()}
                  </span>
                </div>
              </div>
              <div style={{ display: 'flex', gap: '12px' }}>
                {canStart && (
                  <button
                    onClick={() => handleStartRide(ride.rideID)}
                    style={{ padding: '10px 20px', border: 'none', borderRadius: '12px', fontWeight: '500', cursor: 'pointer', fontSize: '14px', backgroundColor: '#059669', color: 'white' }}
                  >
                    Start Ride
                  </button>
                )}

                {canEnd && (
                  <button
                    onClick={() => handleEndRide(ride.rideID)}
                    style={{ padding: '10px 20px', border: 'none', borderRadius: '12px', fontWeight: '500', cursor: 'pointer', fontSize: '14px', backgroundColor: '#dc2626', color: 'white' }}
                  >
                    End Ride
                  </button>
                )}

                {showChat && (
                  <Link
                    to={`/chat/${ride.rideID}`}
                    style={{ fontSize: '14px', color: '#1d4ed8', textDecoration: 'underline', fontWeight: '500', display: 'flex', alignItems: 'center' }}
                  >
                    Open Chat
                  </Link>
                )}
              </div>
            </div>
            {accepted.length > 0 && (
              <div style={{ marginBottom: '16px', padding: '20px', backgroundColor: '#f0fdf4', borderRadius: '12px', border: '2px solid #bbf7d0' }}>
                <div style={{ fontWeight: '600', color: '#065f46', marginBottom: '8px' }}>
                  Accepted Passengers ({accepted.length}):
                </div>
                <ul style={{ fontSize: '14px', color: '#047857', listStyle: 'none', padding: 0, margin: 0 }}>
                  {accepted.map((p: any) => (
                    <li key={p.userID} style={{ margin: '8px 0', display: 'flex', alignItems: 'center', gap: '12px' }}>
                      <span>• {p.userName || p.userID}</span>
                    </li>
                  ))}
                </ul>
              </div>
            )}

            {pendingRequests[ride.rideID]?.length > 0 && (
              <>
                <div style={{ marginTop: '16px', fontWeight: '600', color: '#1f2937', marginBottom: '12px' }}>Pending join requests:</div>
                <ul style={{ marginTop: '8px', listStyle: 'none', padding: 0 }}>
                  {pendingRequests[ride.rideID].map((req: any) => (
                    <li key={req.userID} style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', padding: '16px', backgroundColor: '#dcfce7', borderRadius: '12px', border: '2px solid #bfdbfe', marginBottom: '12px' }}>
                      <span style={{ color: '#111827', fontWeight: '500' }}>User: {req.userName || req.userID}</span>
                      <div style={{ display: 'flex', gap: '12px' }}>
                        <button onClick={() => handleRespond(ride.rideID, req.userID, true)} style={{ padding: '8px 16px', backgroundColor: '#d1fae5', color: '#065f46', borderRadius: '12px', fontWeight: '500', border: 'none', cursor: 'pointer' }}>Accept</button>
                        <button onClick={() => handleRespond(ride.rideID, req.userID, false)} style={{ padding: '8px 16px', backgroundColor: '#fee2e2', color: '#991b1b', borderRadius: '12px', fontWeight: '500', border: 'none', cursor: 'pointer' }}>Reject</button>
                      </div>
                    </li>
                  ))}
                </ul>
              </>
            )}
          </div>
        )
      }) : null}
    </div>
  )
}
