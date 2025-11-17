import React from 'react'
import { Link, useLocation, useNavigate } from 'react-router-dom'
import { useAuth } from '../context/AuthContext'

export default function Navbar() {
  const { user, logout } = useAuth()
  const location = useLocation()
  const navigate = useNavigate()

  return (
    <nav className="bg-white/90 backdrop-blur shadow-sm sticky top-0 z-40">
      <div className="container mx-auto px-4 py-3 flex items-center justify-between">
        <div className="flex items-center gap-6">
          <Link to="/" className="font-extrabold text-2xl text-gray-900">UniRide</Link>
          {user && (
            <div className="hidden md:flex items-center gap-6 text-sm">
              <button onClick={() => navigate('/ride')} className={`px-4 py-1.5 bg-blue-200 hover:bg-blue-700 border-none ${location.pathname.startsWith('/ride') ? 'font-semibold' : ''}`}>
                Ride</button>
              <button onClick={() => navigate('/dashboard')} className={`px-4 py-1.5 bg-blue-200 hover:bg-blue-700 border-none ${location.pathname.startsWith('/dashboard') ? 'font-semibold' : ''}`}>Dashboard</button>
            </div>
          )}
        </div>
        <div className="flex items-center gap-4">
          {user ? (
            <>
              <span className="hidden sm:block text-sm text-gray-700 mr-1">{user.name} ({user.email})</span>
              <button onClick={logout} className="px-2 py-1.5 rounded-xl bg-blue-700 text-white hover:bg-blue-800 font-medium">Logout</button>
            </>
          ) : (
            // <Link to="/" className="px-4 py-1.5 rounded-xl bg-blue-700 text-white hover:bg-blue-700 font-medium">Sign in</Link>
            <button className="px-4 py-1.5 rounded-xl bg-blue-700 text-white hover:bg-blue-800 font-medium">Sign in</button>
          )}
        </div>
      </div>
    </nav>
  )
}