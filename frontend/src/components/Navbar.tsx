import React from 'react'
import { Link, useLocation } from 'react-router-dom'
import { useAuth } from '../context/AuthContext'

export default function Navbar() {
  const { user, logout } = useAuth()
  const location = useLocation()

  return (
    <nav className="bg-white/90 backdrop-blur shadow-sm sticky top-0 z-40">
      <div className="container mx-auto px-4 py-3 flex items-center justify-between">
        <div className="flex items-center gap-6">
          <Link to="/" className="font-extrabold text-2xl text-gray-900">UniRide</Link>
          {user && (
            <div className="hidden md:flex items-center gap-3 text-sm">
              <Link to="/ride" className={`px-3 py-1 rounded hover:bg-gray-100 ${location.pathname.startsWith('/ride')?'font-semibold':''}`}>Ride</Link>
              <Link to="/dashboard" className={`px-3 py-1 rounded hover:bg-gray-100 ${location.pathname.startsWith('/dashboard')?'font-semibold':''}`}>Dashboard</Link>
            </div>
          )}
        </div>
        <div className="flex items-center gap-3">
          {user ? (
            <>
              <span className="hidden sm:block text-sm text-gray-700">{user.name} ({user.email})</span>
              <button onClick={logout} className="px-3 py-1.5 rounded-lg bg-gray-900 text-white hover:bg-black">Logout</button>
            </>
          ) : (
            <Link to="/" className="px-3 py-1.5 rounded-lg bg-blue-600 text-white hover:bg-blue-700">Sign in</Link>
          )}
        </div>
      </div>
    </nav>
  )
}