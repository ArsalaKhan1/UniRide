import React, { useState } from 'react'
import { useRideLocations } from '../hooks/useRideLocations'
import { useAuth } from '../context/AuthContext'
import { rideAPI } from '../api/client'

export default function RideOfferForm() {
  const [from, setFrom] = useState('NED University')
  const [to, setTo] = useState('')
  const [rideType, setRideType] = useState<'carpool' | 'bike'>('carpool')
  const [femalesOnly, setFemalesOnly] = useState(false)
  const [seats, setSeats] = useState(1)
  const [loading, setLoading] = useState(false)
  const [successMsg, setSuccessMsg] = useState<string | null>(null)
  const { user } = useAuth()
  const locations = useRideLocations()

  function handleFrom(value: string) {
    setFrom(value)
    if (value !== 'NED University') setTo('NED University')
  }
  function handleTo(value: string) {
    setTo(value)
    if (value !== 'NED University') setFrom('NED University')
  }

  const offerRide = async (e: React.FormEvent) => {
    e.preventDefault()
    setLoading(true)
    setSuccessMsg(null)
    try {
      if (!user) throw new Error('User not authenticated')
      await rideAPI.offer({
        userID: user.id,
        from,
        to,
        rideType,
        femalesOnly,
        seats,
      })
      setSuccessMsg('Your ride has been posted successfully! Passengers will start seeing your offer soon.')
    } catch (e: any) {
      setSuccessMsg(e?.message || 'Error creating ride')
    } finally {
      setLoading(false)
    }
  }

  return (
     // <form className="p-6 bg-white rounded-2xl shadow-lg max-w-2xl mx-auto mt-8" onSubmit={offerRide}>
    <form className="p-10 bg-blue-100 rounded-3xl shadow-xl max-w-4xl mx-auto mt-8 border-2 border-blue-200" onSubmit={offerRide}>
      <h2 className="font-semibold text-2xl mb-4 text-gray-900">Offer a Ride</h2>
      <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
        <div>
          <label className="block text-sm text-gray-700 mb-1">From</label>
          <select value={from} onChange={e => handleFrom(e.target.value)} disabled={to !== '' && to !== 'NED University'} className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500">
            {locations.map(loc => <option key={loc} value={loc}>{loc}</option>)}
          </select>
        </div>
        <div>
          <label className="block text-sm text-gray-700 mb-1">To</label>
          <select value={to} onChange={e => handleTo(e.target.value)} disabled={from !== '' && from !== 'NED University'} className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500">
            {locations.map(loc => loc !== from && <option key={loc} value={loc}>{loc}</option>)}
          </select>
        </div>
      </div>
      {/* Show females-only option only for female users */}
      {((user?.gender || '').toLowerCase() === 'female') && (
        <div className="mt-4">
          <label className="inline-flex items-center text-gray-800"><input type="checkbox" checked={femalesOnly} onChange={e => setFemalesOnly(e.target.checked)} className="mr-2" />Females only ride</label>
        </div>
      )}
      <div className="mt-4">
        <label className="block text-sm text-gray-700 mb-1">Vehicle Type</label>
        <div className="flex gap-6 text-gray-900">
          <label className="inline-flex items-center"><input type="radio" name="rtype" value="carpool" checked={rideType==='carpool'} onChange={()=>setRideType('carpool')} className="mr-2"/> Car</label>
          <label className="inline-flex items-center"><input type="radio" name="rtype" value="bike" checked={rideType==='bike'} onChange={()=>setRideType('bike')} className="mr-2"/> Bike</label>
        </div>
      </div>
      <div className="mt-4">
        <label className="block text-sm text-gray-700 mb-1">Seats Available</label>
        <select value={seats} onChange={e => setSeats(Number(e.target.value))} className="w-32 px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500">
          {rideType === 'bike' ? (
            <>
              <option value={1}>1</option>
              <option value={2}>2</option>
            </>
          ) : (
            <>
              <option value={1}>1</option>
              <option value={2}>2</option>
              <option value={3}>3</option>
              <option value={4}>4</option>
            </>
          )}
        </select>
      </div>
      <div className="mt-6">
        <button type="submit" disabled={loading} className="px-6 py-3 rounded-xl bg-blue-700 text-white hover:bg-blue-800 disabled:opacity-60 font-medium">{loading ? 'Posting…' : 'Post Ride Offer'}</button>
      </div>
      {successMsg && <div className="mt-3 text-green-700 font-medium">{successMsg}</div>}
    </form>
  )
}