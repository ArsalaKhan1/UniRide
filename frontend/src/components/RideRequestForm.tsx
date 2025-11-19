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
    return <div style={{ color: '#dc2626', fontWeight: '600', textAlign: 'center', fontSize: '1rem' }}>You must be signed in to request a ride.</div>;
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

      let allMatches: any[] = []

      for (const rideType of types) {
        try {
          const { data } = await requestAPI.create({
            userID: userID,
            from,
            to,
            rideType: rideType,
            femalesOnly,
            searchOnly: true,
          })

          if (data.matches && Array.isArray(data.matches) && data.matches.length > 0) {
            allMatches = [...allMatches, ...data.matches]
          }
        } catch (e) {
          console.error(`Error searching ${rideType}:`, e)
        }
      }

      const uniqueMatches = allMatches.filter((ride, index, self) =>
        index === self.findIndex((r) => r.rideID === ride.rideID)
      )

      const filtered = uniqueMatches.filter((ride: any) => {
        if (ride.leadUserID === userID) return false
        const available = ride.availableSlots ?? 0
        if (available <= 0) return false
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
    <div style={{
      padding: '1rem',
      backgroundColor: '#dbeafe',
      borderRadius: '1rem',
      boxShadow: '0 25px 50px -12px rgba(0, 0, 0, 0.25)',
      maxWidth: '700px',
      margin: '0.5rem auto',
      border: '2px solid #bfdbfe',
      textAlign: 'center'
    }}>
      <h2 style={{ fontWeight: '600', fontSize: '1.25rem', marginBottom: '0.75rem', color: '#111827' }}>Request a Ride</h2>
      <form onSubmit={searchRides} style={{ marginBottom: '0.75rem' }}>
        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(auto-fit, minmax(200px, 1fr))', gap: '0.75rem' }}>
          <div>
            <label style={{ display: 'block', fontSize: '0.875rem', color: '#374151', marginBottom: '0.25rem' }}>From</label>
            <select 
              value={from} 
              onChange={e => handleFrom(e.target.value)} 
              disabled={to !== '' && to !== 'NED University'} 
              style={{ 
                width: '100%', 
                padding: '0.5rem', 
                border: '1px solid #d1d5db', 
                borderRadius: '0.5rem', 
                fontSize: '0.875rem',
                outline: 'none'
              }}
            >
              {locations.map(loc => <option key={loc} value={loc}>{loc}</option>)}
            </select>
          </div>
          <div>
            <label style={{ display: 'block', fontSize: '0.875rem', color: '#374151', marginBottom: '0.25rem' }}>To</label>
            <select 
              value={to} 
              onChange={e => handleTo(e.target.value)} 
              disabled={from !== '' && from !== 'NED University'} 
              style={{ 
                width: '100%', 
                padding: '0.5rem', 
                border: '1px solid #d1d5db', 
                borderRadius: '0.5rem', 
                fontSize: '0.875rem',
                outline: 'none'
              }}
            >
              {locations.map(loc => loc !== from && <option key={loc} value={loc}>{loc}</option>)}
            </select>
          </div>
        </div>
        {((user?.gender || '').toLowerCase() === 'female') && (
          <div style={{ marginTop: '0.75rem' }}>
            <label style={{ display: 'inline-flex', alignItems: 'center', color: '#1f2937', fontSize: '0.875rem' }}>
              <input type="checkbox" checked={femalesOnly} onChange={e => setFemalesOnly(e.target.checked)} style={{ marginRight: '0.5rem' }} />
              Females only
            </label>
          </div>
        )}
        <div style={{ marginTop: '0.75rem' }}>
          <label style={{ display: 'block', fontSize: '0.875rem', color: '#374151', marginBottom: '0.25rem' }}>Preferred Vehicle Types</label>
          <div style={{ display: 'flex', flexWrap: 'wrap', gap: '0.75rem', justifyContent: 'center' }}>
            {VEHICLE_TYPES.map(t => (
              <label key={t.value} style={{ display: 'inline-flex', alignItems: 'center', color: '#111827', fontSize: '0.875rem' }}>
                <input type="checkbox" checked={types.includes(t.value)} onChange={() => handleType(t.value)} style={{ marginRight: '0.5rem' }} /> {t.label}
              </label>
            ))}
          </div>
        </div>
        <div style={{ marginTop: '0.75rem' }}>
          <button 
            type="submit" 
            disabled={loading} 
            style={{ 
              padding: '0.5rem 1.25rem', 
              borderRadius: '0.5rem', 
              backgroundColor: '#1d4ed8', 
              color: 'white', 
              border: 'none',
              fontWeight: '500', 
              fontSize: '0.875rem',
              cursor: 'pointer',
              opacity: loading ? 0.6 : 1
            }}
          >
            {loading ? 'Searching…' : 'Search for Rides'}
          </button>
        </div>
      </form>
      {showFallbackModal && (
        <div style={{ backgroundColor: '#dbeafe', padding: '0.75rem', borderRadius: '0.75rem', boxShadow: '0 25px 50px -12px rgba(0, 0, 0, 0.25)', textAlign: 'center', maxWidth: '300px', margin: '0 auto', border: '2px solid #bfdbfe' }}>
          <h3 style={{ marginBottom: '0.5rem', fontWeight: '600', color: '#dc2626', fontSize: '1rem' }}>No matching rides found!</h3>
          <div style={{ color: '#374151', fontSize: '0.875rem' }}>Choose a fallback ride type to post:</div>
          <div style={{ margin: '0.75rem 0', display: 'flex', justifyContent: 'center', gap: '0.5rem' }}>
            <button 
              style={{ 
                padding: '0.5rem 0.75rem', 
                borderRadius: '0.5rem', 
                fontWeight: '500',
                fontSize: '0.875rem',
                border: fallbackType === 'rickshaw' ? 'none' : '2px solid #bfdbfe',
                backgroundColor: fallbackType === 'rickshaw' ? '#1d4ed8' : 'white',
                color: fallbackType === 'rickshaw' ? 'white' : '#111827',
                cursor: 'pointer'
              }} 
              onClick={() => setFallbackType('rickshaw')}
            >
              Rickshaw
            </button>
            <button 
              style={{ 
                padding: '0.5rem 0.75rem', 
                borderRadius: '0.5rem', 
                fontWeight: '500',
                fontSize: '0.875rem',
                border: fallbackType === 'carpool' ? 'none' : '2px solid #bfdbfe',
                backgroundColor: fallbackType === 'carpool' ? '#1d4ed8' : 'white',
                color: fallbackType === 'carpool' ? 'white' : '#111827',
                cursor: 'pointer'
              }} 
              onClick={() => setFallbackType('carpool')}
            >
              Car
            </button>
          </div>
          <button
            style={{ 
              marginTop: '0.25rem', 
              padding: '0.5rem 1rem', 
              borderRadius: '0.5rem', 
              backgroundColor: '#16a34a', 
              color: 'white', 
              border: 'none',
              fontWeight: '500',
              fontSize: '0.875rem',
              cursor: 'pointer',
              opacity: (fallbackType !== 'rickshaw' && fallbackType !== 'carpool') ? 0.6 : 1
            }}
            disabled={fallbackType !== 'rickshaw' && fallbackType !== 'carpool'}
            onClick={postFallbackRide}
          >
            Post Fallback Ride
          </button>
          <div>
            <button 
              style={{ 
                marginTop: '0.5rem', 
                textDecoration: 'underline', 
                color: '#4b5563', 
                fontWeight: '500',
                fontSize: '0.875rem',
                background: 'none',
                border: 'none',
                cursor: 'pointer'
              }} 
              onClick={() => { setShowFallbackModal(false); setFallbackType('') }}
            >
              Cancel
            </button>
          </div>
        </div>
      )}
      {rides.length > 0 && (
        <div>
          <div style={{ fontWeight: '600', marginBottom: '0.5rem', fontSize: '1rem', color: '#111827' }}>Matching rides:</div>
          <ul style={{ listStyle: 'none', padding: 0, margin: 0 }}>
            {rides.map(r => {
              const available = r.availableSlots ?? 0
              return (
                <li key={r.rideID} style={{ 
                  padding: '0.5rem', 
                  backgroundColor: '#f9fafb', 
                  borderRadius: '0.5rem', 
                  display: 'flex', 
                  flexDirection: 'column', 
                  alignItems: 'center', 
                  justifyContent: 'space-between',
                  marginBottom: '0.5rem',
                  fontSize: '0.875rem'
                }}>
                  <div style={{ color: '#111827', textAlign: 'center', marginBottom: '0.5rem' }}>
                    <span style={{ fontWeight: '600' }}>{r.from} → {r.to}</span> |
                    <span style={{ color: '#374151' }}>
                      {(r.leadUserName || r.leadUserID)} | type: {r.rideType} | seats: {available}
                      {r.currentCapacity > 1 && <span style={{ color: '#2563eb' }}> ({r.currentCapacity - 1} accepted)</span>}
                    </span>
                  </div>
                  <button
                    style={{ 
                      padding: '0.5rem 1rem', 
                      borderRadius: '0.5rem', 
                      backgroundColor: '#1d4ed8', 
                      color: 'white', 
                      border: 'none',
                      fontWeight: '500',
                      fontSize: '0.875rem',
                      cursor: 'pointer',
                      opacity: r.leadUserID === userID ? 0.6 : 1
                    }}
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
      {noMatch && <div style={{ marginTop: '0.5rem', color: '#92400e', backgroundColor: '#fef3c7', padding: '0.5rem', borderRadius: '0.5rem', fontSize: '0.875rem' }}>{noMatch}</div>}
      {confirmMsg && <div style={{ marginTop: '0.5rem', color: '#166534', backgroundColor: '#dcfce7', padding: '0.5rem', borderRadius: '0.5rem', fontSize: '0.875rem' }}>{confirmMsg}</div>}
    </div>
  )
}