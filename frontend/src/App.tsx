import React from 'react'
import { Routes, Route, Navigate } from 'react-router-dom'
import Landing from './pages/Landing'
import Dashboard from './pages/Dashboard'
import ChatPage from './pages/ChatPage'
import RideInteractionScreen from './pages/RideInteractionScreen'
import { AuthProvider, useAuth } from './context/AuthContext'
import Navbar from './components/Navbar'

function PrivateRoute({ children }: { children: JSX.Element }) {
  const { user } = useAuth()
  return user ? children : <Navigate to="/" />
}

export default function App() {
  return (
    <AuthProvider>
      <div className="min-h-screen bg-gray-100">
        <Navbar />
        <div className="container mx-auto p-4">
          <Routes>
            <Route path="/" element={<Landing />} />
            <Route path="/ride" element={
              <PrivateRoute>
                <RideInteractionScreen />
              </PrivateRoute>
            } />
            <Route
              path="/dashboard"
              element={
                <PrivateRoute>
                  <Dashboard />
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
      </div>
    </AuthProvider>
  )
}
