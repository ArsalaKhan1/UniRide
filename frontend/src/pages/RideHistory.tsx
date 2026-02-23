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
    <div style={{marginRight: 'auto', padding: '10px' }}>
      <h2 style={{ fontSize: '32px', fontWeight: 'bold', marginBottom: '32px', color: '#111827', textAlign: 'center' }}>Ride History</h2>
      {completedRides.length > 0 ? completedRides.map(ride => {
        return (
          <div key={ride.rideID} style={{ margin: '24px 0 24px 100px', padding: '32px', backgroundColor: '#f3f4f6', borderRadius: '24px', boxShadow: '0 10px 15px -3px rgba(0, 0, 0, 0.1)', border: '2px solid #e5e7eb', maxWidth: '800px' }}>
            <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
              <div>
                <div style={{ fontSize: '20px', fontWeight: '600', color: '#111827' }}>Ride - {ride.from} to {ride.to}</div>
                <div style={{ fontSize: '14px', color: '#4b5563', marginTop: '4px' }}>
                  Type: {ride.rideType} | Lead: {ride.leadUserName || ride.leadUserID} |
                  Status: <span style={{ fontWeight: '600', color: '#6b7280' }}>COMPLETED</span>
                </div>
              </div>
            </div>
          </div>
        )
      }) : <div style={{ color: '#4b5563', fontSize: '16px' }}>{loading ? 'Loading your rides…' : 'No completed rides yet.'}</div>}
    </div>
  )
}
