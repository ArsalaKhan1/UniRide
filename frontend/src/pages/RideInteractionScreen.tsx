import React, { useState } from 'react'
import RideOfferForm from '../components/RideOfferForm'
import RideRequestForm from '../components/RideRequestForm'

export default function RideInteractionScreen() {
  const [choice, setChoice] = useState<null | 'offer' | 'request'>(null)

  if (!choice) return (
    <div className="container mx-auto p-4" style={{
      maxWidth: '800px',
      margin: '4rem auto 2rem',
      padding: '2.5rem',
      backgroundColor: '#ffffff',
      borderRadius: '1.5rem',
      boxShadow: '0 25px 50px -12px rgba(0, 0, 0, 0.25)',
      border: '2px solid #a9abab',
      textAlign: 'center'
    }}>
      <h1 style={{
        fontSize: '1.875rem',
        fontWeight: 'bold',
        marginBottom: '0.5rem',
        color: '#e07d2e'
      }}>Looking for a new ride?</h1>
      <p style={{
        marginBottom: '1.5rem',
        color: '#4b5563'
      }}>Fill in a few details and preferences, and you'll be finding one in no time!</p>
      <div style={{
        marginBottom: '1.5rem',
        fontWeight: '500',
        fontSize: '1.125rem',
        color: '#111827'
      }}>Are you a car/bike owner looking for a carpooling passenger?</div>
      <div style={{
        display: 'flex',
        gap: '1rem',
        justifyContent: 'center'
      }}>
        <button
          style={{
            padding: '1rem 2.5rem',
            backgroundColor: '#e07d2e',
            color: 'white',
            borderRadius: '0.75rem',
            fontWeight: '500',
            fontSize: '1.125rem',
            border: 'none',
            cursor: 'pointer',
            minWidth: '120px'
          }}
          onClick={() => setChoice('offer')}
          onMouseOver={(e) => (e.currentTarget.style.backgroundColor = '#c96b1f')}
          onMouseOut={(e) => (e.currentTarget.style.backgroundColor = '#e07d2e')}
        >Yes</button>
        <button
          style={{
            padding: '1rem 2.5rem',
            backgroundColor: 'white',
            color: '#111827',
            borderRadius: '0.75rem',
            fontWeight: '500',
            fontSize: '1.125rem',
            border: '2px solid #a9abab',
            cursor: 'pointer',
            minWidth: '120px'
          }}
          onClick={() => setChoice('request')}
          
          onMouseOver={(e) => (e.currentTarget.style.backgroundColor = '#f5f5f5')}
          onMouseOut={(e) => (e.currentTarget.style.backgroundColor = 'white')}
        >No</button>
      </div>
    </div>
  )

  return choice === 'offer' ? <RideOfferForm /> : <RideRequestForm />
}