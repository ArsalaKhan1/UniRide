import React, { useEffect, useState } from 'react'
import { useAuth } from '../context/AuthContext'
import { rideAPI } from '../api/client'

export default function Dashboard() {
  const { user } = useAuth()
  const [myRides, setMyRides] = useState<any[]>([])
  const [pendingRequests, setPendingRequests] = useState<{[rideId:number]: any[]}>({})
  const [loading, setLoading] = useState(false)

  const refreshMyRides = async () => {
    if (!user) return
    setLoading(true)
    try {
      const res = await rideAPI.getAll()
      const owned = res.data.rides?.filter((r:any) => r.leadUserID===user.id) || []
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

  useEffect(() => {
    if (!myRides.length) return
    const intervals: number[] = []
    myRides.forEach(ride => {
      const poll = async () => {
        try {
          const res = await rideAPI.getRideRequests(ride.rideID)
          setPendingRequests(pr => ({...pr, [ride.rideID]: res.data.requests || []}))
        } catch {}
      }
      poll();
      intervals.push(window.setInterval(poll, 7000))
    })
    return () => intervals.forEach(clearInterval)
  }, [myRides])

  const handleRespond = async (rideID:number, reqUserID:string, accept: boolean) => {
    try {
      await rideAPI.respondToRequest({ rideID, userID: reqUserID, accept })
      setPendingRequests(pr => ({...pr, [rideID]: pr[rideID]?.filter((r:any)=>r.userID!==reqUserID) || []}))
      refreshMyRides()
    } catch (e) {
      console.error('Failed to respond to request', e)
    }
  }

  return (
    <div>
      <h2 className="text-2xl font-semibold mb-4">Dashboard</h2>
      {myRides.length>0 ? myRides.map(ride => (
        <div key={ride.rideID} className="my-3 p-3 border rounded">
          <div>Ride {ride.rideID} to {ride.to} [{ride.rideType}]</div>
          <div><span className="font-semibold">You as Lead:</span> {ride.leadUserName||ride.leadUserID}</div>
          {pendingRequests[ride.rideID]?.length > 0 && <>
            <div className="mt-2 font-semibold">Pending join requests:</div>
            <ul>
              {pendingRequests[ride.rideID].map((req:any) => (
                <li key={req.userID} className="flex items-center gap-2 mt-1">
                  <span>User: {req.userName||req.userID}</span>
                  <button onClick={()=>handleRespond(ride.rideID, req.userID, true)} className="px-2 py-1 bg-green-500 text-white rounded">Accept</button>
                  <button onClick={()=>handleRespond(ride.rideID, req.userID, false)} className="px-2 py-1 bg-red-500 text-white rounded">Reject</button>
                </li>
              ))}
            </ul>
          </>}
        </div>
      )):<div>{loading ? 'Loading your rides…' : 'No rides created by you yet.'}</div>}
    </div>
  )
}
