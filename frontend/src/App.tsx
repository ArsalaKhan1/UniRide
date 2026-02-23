import React from 'react'
import { Routes, Route, Navigate, useLocation } from 'react-router-dom'
import Landing from './pages/Landing'
import Rides from './pages/Rides'
import RideHistory from './pages/RideHistory'
import ChatPage from './pages/ChatPage'
import { AuthProvider, useAuth } from './context/AuthContext'
import Navbar from './components/Navbar'

function PrivateRoute({ children }: { children: JSX.Element }) {
  const { user } = useAuth()
  return user ? children : <Navigate to="/" />
}

function AppContent() {
  const location = useLocation()
  const showNavbar = location.pathname !== '/'
  const isChatPage = location.pathname.startsWith('/chat/')

  return (
    <div className="min-h-screen bg-gray-100">
      {showNavbar && <Navbar />}
          <Routes>
            <Route path="/" element={<Landing />} />
            <Route path="/rides" element={
              <PrivateRoute>
                <Rides />
              </PrivateRoute>
            } />
            <Route path="/dashboard" element={<Navigate to="/rides" replace />} />
            <Route
              path="/ride-history"
              element={
                <PrivateRoute>
                  <RideHistory />
                </PrivateRoute>
              }
            />
            <Route
              path="/chat/:rideId"
              element={
                <PrivateRoute>
                  <ChatPage />
                </PrivateRoute>
              }
            />
          </Routes>
    </div>
  )
}

export default function App() {
  return (
    <AuthProvider>
      <AppContent />
    </AuthProvider>
  )
}
