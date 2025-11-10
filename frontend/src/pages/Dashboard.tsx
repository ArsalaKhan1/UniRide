import React, { useEffect, useState } from 'react'
import RideForm from '../components/RideForm'
import { rideAPI } from '../api/client'
import { useAuth } from '../context/AuthContext'

type Ride = {
  rideID: number
  leadUserID: string
  from: string
  to: string
  time?: string
  rideType?: string
  currentCapacity?: number
  maxCapacity?: number
  availableSlots?: number
  peopleJoined?: number
  spotsRemaining?: number
  femalesOnly?: boolean
}

export default function Dashboard() {
  const [rides, setRides] = useState<Ride[]>([])
  const [loading, setLoading] = useState(false)
  const { user } = useAuth()

  const fetchRides = async () => {
    setLoading(true)
    try {
      const res = await rideAPI.getAll()
      const data = res.data
      setRides(data.rides || [])
    } catch (e) {
      console.error('Failed to fetch rides', e)
    } finally {
      setLoading(false)
    }
  }

  useEffect(() => {
    fetchRides()
  }, [])

  const handleJoin = async (r: Ride) => {
    if (!user) return alert('Please sign in')
    try {
      await rideAPI.join({ userID: user.id, from: r.from, to: r.to, rideType: r.rideType })
      alert('Successfully joined')
      fetchRides()
    } catch (e: any) {
      alert(e?.message || 'Failed to join')
    }
  }

  return (
    <div>
      <h2 className="text-2xl font-semibold mb-4">Dashboard</h2>
      <div className="grid grid-cols-1 md:grid-cols-2 gap-6">
        <div className="p-4 bg-white rounded shadow">
          <h3 className="font-semibold mb-2">Offer a new ride</h3>
          <RideForm />
        </div>

        <div className="p-4 bg-white rounded shadow">
          <h3 className="font-semibold mb-2">Available rides</h3>
          {loading ? (
            <p>Loading…</p>
          ) : (
            <ul className="space-y-3">
              {rides.map((r) => (
                <li key={r.rideID} className="p-3 bg-gray-50 rounded">
                  <div className="flex justify-between items-center">
                    <div>
                      <div className="font-semibold">{r.from} → {r.to}</div>
                      <div className="text-sm">Type: {r.rideType} • Lead: {r.leadUserID}</div>
                    </div>
                    <div>
                      <button onClick={() => handleJoin(r)} className="px-3 py-1 rounded bg-primary text-white">Join</button>
                    </div>
                  </div>
                </li>
              ))}
            </ul>
          )}
        </div>
      </div>
    </div>
  )
}
