import React from 'react'
import { Link } from 'react-router-dom'
import { useAuth } from '../context/AuthContext'

export default function Navbar() {
  const { user, logout } = useAuth()

  return (
    <nav className="bg-white shadow">
      <div className="container mx-auto px-4 py-3 flex items-center justify-between">
        <div>
          <Link to="/" className="font-bold text-xl text-primary">UniRide</Link>
        </div>
        <div className="flex items-center gap-4">
          {user ? (
            <>
              <span className="text-sm">{user.name} ({user.email})</span>
              <Link to="/dashboard" className="px-3 py-1 rounded hover:bg-gray-100">Dashboard</Link>
              <button onClick={logout} className="px-3 py-1 rounded bg-red-500 text-white">Logout</button>
            </>
          ) : (
            <Link to="/" className="px-3 py-1 rounded bg-primary text-white">Sign in</Link>
          )}
        </div>
      </div>
    </nav>
  )
}
