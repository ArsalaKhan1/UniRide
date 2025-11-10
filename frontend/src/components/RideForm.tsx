import React, { useState } from 'react'
import { useAuth } from '../context/AuthContext'
import { rideAPI } from '../api/client'

export default function RideForm() {
  const { user } = useAuth()
  const [from, setFrom] = useState('')
  const [to, setTo] = useState('')
  const [rideType, setRideType] = useState('carpool')
  const [femalesOnly, setFemalesOnly] = useState(false)
  const [seats, setSeats] = useState(1)
  const [loading, setLoading] = useState(false)
  const [error, setError] = useState<string | null>(null)

  const handleSubmit = async (e: React.FormEvent) => {
    e.preventDefault()
    setLoading(true)
    setError(null)
    try {
      if (!user) throw new Error('Not authenticated')
      const payload = {
        userID: user.id,
        from,
        to,
        rideType,
        femalesOnly,
      }
      await rideAPI.offer(payload)
      setFrom('')
      setTo('')
      setSeats(1)
      setRideType('carpool')
      setFemalesOnly(false)
      // TODO: refresh rides list
    } catch (err: any) {
      setError(err?.message || 'Failed to create ride')
    } finally {
      setLoading(false)
    }
  }

  return (
    <form onSubmit={handleSubmit} className="space-y-3">

      <div>
        <label className="block text-sm">From</label>
        <input value={from} onChange={(e) => setFrom(e.target.value)} className="w-full border rounded px-2 py-1" />
      </div>

      <div>
        <label className="block text-sm">To</label>
        <input value={to} onChange={(e) => setTo(e.target.value)} className="w-full border rounded px-2 py-1" />
      </div>

      <div>
        <label className="block text-sm">Ride Type</label>
        <select value={rideType} onChange={(e) => setRideType(e.target.value)} className="w-full border rounded px-2 py-1">
          <option value="bike">Bike</option>
          <option value="rickshaw">Rickshaw</option>
          <option value="carpool">Carpool</option>
        </select>
      </div>

      <div>
        <label className="inline-flex items-center">
          <input type="checkbox" checked={femalesOnly} onChange={(e) => setFemalesOnly(e.target.checked)} />
          <span className="ml-2">Females only</span>
        </label>
      </div>

      <div>
        <label className="block text-sm">Seats (for owners)</label>
        <input type="number" value={seats} min={1} onChange={(e) => setSeats(Number(e.target.value))} className="w-24 border rounded px-2 py-1" />
      </div>

      {error && <div className="text-red-600">{error}</div>}

      <div>
        <button disabled={loading} className="px-4 py-2 rounded bg-primary text-white">
          {loading ? 'Offering...' : 'Offer Ride'}
        </button>
      </div>
    </form>
  )
}
