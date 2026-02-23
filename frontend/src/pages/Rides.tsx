import React, { useEffect, useState, useRef } from 'react'
import { useAuth } from '../context/AuthContext'
import { useLocation } from 'react-router-dom'
import { rideAPI, userAPI } from '../api/client'
import CurrentRide from './CurrentRide'
import RideInteractionScreen from './RideInteractionScreen'

export default function Rides() {
  const { user } = useAuth()
  const location = useLocation()
  const [myRides, setMyRides] = useState<any[]>([])
  const [passengerRides, setPassengerRides] = useState<any[]>([])
  const [loading, setLoading] = useState(true)
  const [hasCheckedRides, setHasCheckedRides] = useState(false)
  const prevAcceptedCount = useRef(0)

  const fetchRides = async () => {
    if (!user) return
    try {
      const [ridesRes, acceptedRes] = await Promise.all([
        rideAPI.getAll(),
        userAPI.getAcceptedRequests(String(user.id))
      ])
      const owned = ridesRes.data.rides?.filter((r: any) => r.leadUserID === user.id) || []
      const accepted = acceptedRes.data.acceptedRequests || []
      
      setMyRides(owned)
      setPassengerRides(accepted)
    } catch (e) {
      console.error('Error fetching rides:', e)
    }
  }

  useEffect(() => {
    if (!user) {
      setLoading(false)
      setHasCheckedRides(true)
      return
    }

    // Fetch on mount or when refresh flag is set via navigation
    if (location.state?.refresh) {
      fetchRides().finally(() => {
        setLoading(false)
        setHasCheckedRides(true)
        // Clear the refresh flag from history to prevent re-fetching on re-render
        window.history.replaceState({}, document.title)
      })
    } else if (!hasCheckedRides) {
      // First load: fetch if we haven't checked rides yet
      fetchRides().finally(() => {
        setLoading(false)
        setHasCheckedRides(true)
      })
    }
  }, [user, location.state?.refresh])

  // Poll for ride status changes (both lead and passenger updates)
  useEffect(() => {
    if (!user || !hasCheckedRides) return

    const pollRideUpdates = async () => {
      try {
        // Fetch both rides and accepted requests to catch any status changes
        const [ridesRes, acceptedRes] = await Promise.all([
          rideAPI.getAll(),
          userAPI.getAcceptedRequests(String(user.id))
        ])
        const owned = ridesRes.data.rides?.filter((r: any) => r.leadUserID === user.id) || []
        const accepted = acceptedRes.data.acceptedRequests || []
        
        // Update state if anything changed
        setMyRides(owned)
        setPassengerRides(accepted)
        
        // Track accepted count for detecting new acceptances
        if (accepted.length > prevAcceptedCount.current) {
          prevAcceptedCount.current = accepted.length
        } else if (accepted.length < prevAcceptedCount.current) {
          // If count decreased, rides were completed/removed
          prevAcceptedCount.current = accepted.length
        }
      } catch (e) {
        console.error('Error polling ride updates:', e)
      }
    }

    const interval = setInterval(pollRideUpdates, 5000)
    return () => clearInterval(interval)
  }, [user, hasCheckedRides])

  if (loading || !hasCheckedRides) {
    return (
      <div style={{
        display: 'flex',
        alignItems: 'center',
        justifyContent: 'center',
        minHeight: '60vh',
        fontSize: '1rem',
        color: '#4b5563',
        fontWeight: '500'
      }}>
        Loading...
      </div>
    )
  }

  const activeRides = myRides.filter(r => r.status !== 'completed')
  const activePassengerRides = passengerRides.filter(r => r.status !== 'completed')
  const hasActiveRide = activeRides.length > 0 || activePassengerRides.length > 0

  return hasActiveRide ? <CurrentRide onRideEnded={fetchRides} /> : <RideInteractionScreen />
}
