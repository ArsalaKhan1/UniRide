import React, { useState } from 'react'
import { useRideLocations } from '../hooks/useRideLocations'
import { useAuth } from '../context/AuthContext'
import { requestAPI, rideAPI } from '../api/client'

const VEHICLE_TYPES = [
  { label: 'Bike', value: 'bike' },
  { label: 'Rickshaw', value: 'rickshaw' },
  { label: 'Car', value: 'carpool' },
]

export default function RideRequestForm() {
  const locations = useRideLocations()
  const { user } = useAuth()
  const [from, setFrom] = useState('NED University')
  const [to, setTo] = useState('')
  const [types, setTypes] = useState<string[]>([])
  const [femalesOnly, setFemalesOnly] = useState(false)
  const [rides, setRides] = useState<any[]>([])
  const [noMatch, setNoMatch] = useState<string | null>(null)
  const [loading, setLoading] = useState(false)
  const [confirmMsg, setConfirmMsg] = useState('')
  const [showFallbackModal, setShowFallbackModal] = useState(false)
  const [fallbackType, setFallbackType] = useState('')

  if (!user) {
    return <div className="text-red-600 font-semibold">You must be signed in to request a ride.</div>;
  }
  const userID = user.id

  function handleFrom(val: string) {
    setFrom(val)
    if (val !== 'NED University') setTo('NED University')
  }
  function handleTo(val: string) {
    setTo(val)
    if (val !== 'NED University') setFrom('NED University')
  }

  function handleType(value: string) {
    setTypes((prev) => prev.includes(value) ? prev.filter(x => x !== value) : [...prev, value])
  }

  async function searchRides(e: React.FormEvent) {
    e.preventDefault()
    setLoading(true)
    setConfirmMsg('')
    setNoMatch(null)
    setRides([])
    setShowFallbackModal(false)
    
    try {
      if (!types.length) {
        setNoMatch('Please select at least one preferred vehicle type.')
        return
      }

      // Use backend's matching logic for EACH vehicle type
      // This ensures we leverage the location graph proximity checking
      let allMatches: any[] = []
      
      for (const rideType of types) {
        try {
          // Call backend's /request/create endpoint which uses findMatchingRides()
          // This includes the location graph proximity logic
          const { data } = await requestAPI.create({
            userID: userID,
            from,
            to,
            rideType: rideType,
            femalesOnly,
            // indicate this is a search-only call; do not auto-create rides on the backend
            searchOnly: true,
          })
          
          // If matches are returned, add them to our list
          if (data.matches && Array.isArray(data.matches) && data.matches.length > 0) {
            allMatches = [...allMatches, ...data.matches]
          }
        } catch (e) {
          console.error(`Error searching ${rideType}:`, e)
        }
      }

      // Remove duplicates by rideID
      const uniqueMatches = allMatches.filter((ride, index, self) =>
        index === self.findIndex((r) => r.rideID === ride.rideID)
      )

      // Additional client-side filtering for UI-specific requirements
      const filtered = uniqueMatches.filter((ride: any) => {
        // Skip if user is the lead
        if (ride.leadUserID === userID) return false
        
        // Check available slots
        const available = ride.availableSlots ?? 0
        if (available <= 0) return false
        
        // Females-only filter (backend already handles this, but double-check)
        if (femalesOnly && !ride.femalesOnly) return false
        
        return true
      })

      setRides(filtered)
      
      if (!filtered.length) {
        setShowFallbackModal(true)
      }
    } catch (e: any) {
      setNoMatch(e?.message || 'Error searching rides')
    } finally {
      setLoading(false)
    }
  }

  async function postFallbackRide() {
    if (fallbackType !== 'rickshaw' && fallbackType !== 'carpool') return;
    setShowFallbackModal(false);
    setLoading(true)
    try {
      const { data } = await requestAPI.create({
        userID: userID,
        from,
        to,
        rideType: fallbackType,
        femalesOnly,
      });
      setNoMatch(`No matching rides found — but you've posted your request as a new ${fallbackType === 'carpool' ? 'Car' : 'Rickshaw'} ride (Ride #${data.rideID ?? ''}). You're now the lead! Others will be able to join your ride.`);
    } catch (e: any) {
      setNoMatch(e?.message || 'Error posting fallback ride');
    } finally {
      setLoading(false)
      setFallbackType('');
    }
  }

  async function requestToJoin(rideID: number) {
    setLoading(true)
    try {
      await rideAPI.sendJoinRequest({ rideID, userID: userID })
      setConfirmMsg('Your request has been sent to the ride owner.')
    } catch (e: any) {
      setConfirmMsg(e?.message || 'Failed to send request.')
    } finally {
      setLoading(false)
    }
  }

  return (
    <div className="p-6 bg-white rounded-2xl shadow-lg max-w-2xl mx-auto mt-8">
      <h2 className="font-semibold text-2xl mb-4 text-gray-900">Request a Ride</h2>
      <form onSubmit={searchRides} className="mb-4">
        <div className="grid grid-cols-1 md:grid-cols-2 gap-4">
          <div>
            <label className="block text-sm text-gray-700 mb-1">From</label>
            <select value={from} onChange={e=>handleFrom(e.target.value)} disabled={to !== '' && to !== 'NED University'} className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500">
              {locations.map(loc => <option key={loc} value={loc}>{loc}</option>)}
            </select>
          </div>
          <div>
            <label className="block text-sm text-gray-700 mb-1">To</label>
            <select value={to} onChange={e=>handleTo(e.target.value)} disabled={from !== '' && from !== 'NED University'} className="w-full px-3 py-2 border border-gray-300 rounded-lg focus:ring-2 focus:ring-blue-500 focus:border-blue-500">
              {locations.map(loc => loc !== from && <option key={loc} value={loc}>{loc}</option>)}
            </select>
          </div>
        </div>
        <div className="mt-4">
          <label className="inline-flex items-center text-gray-800"><input type="checkbox" checked={femalesOnly} onChange={e => setFemalesOnly(e.target.checked)} className="mr-2" />Females only</label>
        </div>
        <div className="mt-4">
          <label className="block text-sm text-gray-700 mb-1">Preferred Vehicle Types</label>
          <div className="flex flex-wrap gap-4">
            {VEHICLE_TYPES.map(t => (
              <label key={t.value} className="inline-flex items-center text-gray-900">
                <input type="checkbox" checked={types.includes(t.value)} onChange={() => handleType(t.value)} className="mr-2"/> {t.label}
              </label>
            ))}
          </div>
        </div>
        <div className="mt-6">
          <button type="submit" disabled={loading} className="px-5 py-2.5 rounded-lg bg-blue-600 text-white hover:bg-blue-700 disabled:opacity-60">{loading ? 'Searching…' : 'Search for Rides'}</button>
        </div>
      </form>
      {showFallbackModal && (
        <div className="fixed inset-0 bg-black/50 flex items-center justify-center z-50">
          <div className="bg-white p-8 rounded-2xl shadow-xl text-center w-full max-w-md">
            <h3 className="mb-3 font-semibold text-gray-900">No matching rides found</h3>
            <div className="text-gray-700">Choose a fallback ride type to post:</div>
            <div className="my-4 flex justify-center gap-4">
              <button className={`px-4 py-2 rounded-lg ${fallbackType==='rickshaw'?'bg-blue-600 text-white':'bg-gray-100 text-gray-900'}`} onClick={()=>setFallbackType('rickshaw')}>Rickshaw</button>
              <button className={`px-4 py-2 rounded-lg ${fallbackType==='carpool'?'bg-blue-600 text-white':'bg-gray-100 text-gray-900'}`} onClick={()=>setFallbackType('carpool')}>Car</button>
            </div>
            <button
              className="mt-1 px-6 py-2 rounded-lg bg-green-600 text-white disabled:opacity-60"
              disabled={fallbackType !== 'rickshaw' && fallbackType !== 'carpool'}
              onClick={postFallbackRide}
            >Post Fallback Ride</button>
            <div>
              <button className="mt-4 underline text-gray-500" onClick={()=>{setShowFallbackModal(false); setFallbackType('')}}>Cancel</button>
            </div>
          </div>
        </div>
      )}
      {rides.length > 0 && (
        <div>
          <div className="font-semibold mb-2">Matching rides (including nearby locations):</div>
          <ul className="space-y-3">
            {rides.map(r => {
              const available = r.availableSlots ?? 0
              return (
                <li key={r.rideID} className="p-3 bg-gray-50 rounded-lg flex flex-col md:flex-row md:items-center md:justify-between">
                  <div className="text-gray-900">
                  <span className="font-semibold">{r.from} → {r.to}</span> | 
                  {/* Show leadDisplay (username - type - seats) when available, else fall back to separate fields */}
                  {/* Render a clear, structured display: username | type: <type> | seats available : <number> */}
                  <span className="text-gray-700">
                    {(r.leadUserName || r.leadUserID)} | type: {r.rideType} | seats available : {available}
                    {r.currentCapacity > 1 && <span className="text-blue-600"> ({r.currentCapacity - 1} already accepted)</span>}
                  </span>
                  </div>
                  <button
                    className="mt-2 md:mt-0 px-3 py-1.5 rounded-lg bg-gray-900 text-white hover:bg-black disabled:opacity-60"
                    onClick={() => requestToJoin(r.rideID)}
                    disabled={r.leadUserID === userID}>
                    Request to Join
                  </button>
                </li>
              )
            })}
          </ul>
        </div>
      )}
      {noMatch && <div className="mt-3 text-amber-700 bg-amber-50 px-3 py-2 rounded">{noMatch}</div>}
      {confirmMsg && <div className="mt-3 text-green-700 bg-green-50 px-3 py-2 rounded">{confirmMsg}</div>}
    </div>
  )
}