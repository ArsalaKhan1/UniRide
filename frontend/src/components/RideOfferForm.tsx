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
    <form 
      style={{
        padding: '1rem',
        backgroundColor: '#dbeafe',
        borderRadius: '1rem',
        boxShadow: '0 25px 50px -12px rgba(0, 0, 0, 0.25)',
        maxWidth: '700px',
        margin: '0.5rem auto',
        border: '2px solid #bfdbfe',
        textAlign: 'center'
      }} 
      onSubmit={offerRide}
    >
      <h2 style={{ fontWeight: '600', fontSize: '1.25rem', marginBottom: '0.75rem', color: '#111827' }}>Offer a Ride</h2>
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
            Females only ride
          </label>
        </div>
      )}
      <div style={{ marginTop: '0.75rem' }}>
        <label style={{ display: 'block', fontSize: '0.875rem', color: '#374151', marginBottom: '0.25rem' }}>Vehicle Type</label>
        <div style={{ display: 'flex', gap: '1.5rem', justifyContent: 'center', color: '#111827' }}>
          <label style={{ display: 'inline-flex', alignItems: 'center', fontSize: '0.875rem' }}>
            <input type="radio" name="rtype" value="carpool" checked={rideType==='carpool'} onChange={()=>setRideType('carpool')} style={{ marginRight: '0.5rem' }} /> Car
          </label>
          <label style={{ display: 'inline-flex', alignItems: 'center', fontSize: '0.875rem' }}>
            <input type="radio" name="rtype" value="bike" checked={rideType==='bike'} onChange={()=>setRideType('bike')} style={{ marginRight: '0.5rem' }} /> Bike
          </label>
        </div>
      </div>
      <div style={{ marginTop: '0.75rem' }}>
        <label style={{ display: 'block', fontSize: '0.875rem', color: '#374151', marginBottom: '0.25rem' }}>Seats Available</label>
        <select 
          value={seats} 
          onChange={e => setSeats(Number(e.target.value))} 
          style={{ 
            width: '8rem', 
            padding: '0.5rem', 
            border: '1px solid #d1d5db', 
            borderRadius: '0.5rem', 
            fontSize: '0.875rem',
            outline: 'none'
          }}
        >
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
          {loading ? 'Posting…' : 'Post Ride Offer'}
        </button>
      </div>
      {successMsg && <div style={{ marginTop: '0.5rem', color: '#166534', fontWeight: '500', fontSize: '0.875rem' }}>{successMsg}</div>}
    </form>
  )
}