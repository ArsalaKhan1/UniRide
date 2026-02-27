import React, { useEffect, useState } from 'react'
import { useAuth } from '../context/AuthContext'
import { rideAPI } from '../api/client'

export default function RideHistory() {
  const { user } = useAuth()
  const [myRides, setMyRides] = useState<any[]>([])
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

  const completedRides = myRides.filter(r => r.status === 'completed')

  return (
    <div style={{padding: '2rem', maxWidth: '1000px', margin: '0 auto' }}>
      <h2 style={{ fontSize: '2.5rem', fontWeight: 'bold', marginBottom: '2rem', color: '#e07d2e', textAlign: 'center' }}>Ride History</h2>
      {completedRides.length > 0 ? completedRides.map(ride => {
        return (
          <div key={ride.rideID} style={{ margin: '1.5rem 0', padding: '1.5rem', backgroundColor: '#ffffff', borderRadius: '1rem', boxShadow: '0 4px 6px rgba(0, 0, 0, 0.1)', border: '3px solid #e07d2e' }}>
            <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between', gap: '1rem' }}>
              <div style={{ flex: 1, textAlign: 'left' }}>
                <div style={{ fontSize: '1.25rem', fontWeight: '700', color: '#e07d2e', marginBottom: '0.5rem' }}>{ride.from} → {ride.to}</div>
                <div style={{ fontSize: '0.95rem', color: '#6b7280', marginBottom: '0.25rem' }}><strong>Type:</strong> {ride.rideType}</div>
                <div style={{ fontSize: '0.95rem', color: '#6b7280', marginBottom: '0.25rem' }}><strong>Lead:</strong> {ride.leadUserName || ride.leadUserID}</div>
                <div style={{ fontSize: '0.95rem', color: '#10b981', fontWeight: '600' }}>✓ COMPLETED</div>
              </div>
            </div>
          </div>
        )
      }) : <div style={{ textAlign: 'center', padding: '2rem', color: '#6b7280', fontSize: '1rem' }}>{loading ? 'Loading your rides…' : 'No completed rides yet.'}</div>}
    </div>
  )
}
