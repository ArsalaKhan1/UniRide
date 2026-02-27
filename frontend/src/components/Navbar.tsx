import React, { useState } from 'react'
import { Link, useNavigate } from 'react-router-dom'
import { useAuth } from '../context/AuthContext'

export default function Navbar() {
  const { user, logout } = useAuth()
  const navigate = useNavigate()
  const [sidebarOpen, setSidebarOpen] = useState(false)

  const handleLogout = () => {
    logout()
    setSidebarOpen(false)
    navigate('/')
  }

  return (
    <>
      <nav style={{
        backgroundColor: 'rgba(255, 255, 255, 0.95)',
        backdropFilter: 'blur(10px)',
        boxShadow: '0 1px 3px rgba(0, 0, 0, 0.1)',
        position: 'sticky',
        top: 0,
        zIndex: 40
      }}>
        <div style={{
          maxWidth: '1200px',
          margin: '0 auto',
          padding: '0.5rem 1rem',
          display: 'flex',
          alignItems: 'center',
          justifyContent: 'space-between'
        }}>
          <Link to="/" style={{
            textDecoration: 'none',
            transition: 'opacity 0.3s',
            display: 'flex',
            alignItems: 'center'
          }} onMouseEnter={(e) => e.currentTarget.style.opacity = '0.8'} onMouseLeave={(e) => e.currentTarget.style.opacity = '1'}>
            <img src="/logo.png" alt="UniRide Logo" style={{
              height: '55px',
              width: 'auto'
            }} />
          </Link>
          
          <div>
            {user ? (
              <div
                onClick={() => setSidebarOpen(true)}
                style={{
                  width: '40px',
                  height: '40px',
                  borderRadius: '50%',
                  backgroundColor: '#e07d2e',
                  color: 'white',
                  display: 'flex',
                  alignItems: 'center',
                  justifyContent: 'center',
                  fontSize: '1.25rem',
                  fontWeight: 600,
                  cursor: 'pointer',
                  transition: 'background-color 0.3s'
                }}
                onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#c96b1f'}
                onMouseLeave={(e) => e.currentTarget.style.backgroundColor = '#e07d2e'}
              >
                {user.name.charAt(0).toUpperCase()}
              </div>
            ) : (
              <button 
                style={{
                  padding: '0.375rem 1rem',
                  borderRadius: '0.75rem',
                  backgroundColor: '#e07d2e',
                  color: 'white',
                  border: 'none',
                  fontWeight: 500,
                  cursor: 'pointer',
                  transition: 'background-color 0.3s'
                }}
                onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#c96b1f'}
                onMouseLeave={(e) => e.currentTarget.style.backgroundColor = '#e07d2e'}
              >
                Sign in
              </button>
            )}
          </div>
        </div>
      </nav>

      {/* Overlay */}
      {sidebarOpen && (
        <div
          onClick={() => setSidebarOpen(false)}
          style={{
            position: 'fixed',
            top: 0,
            left: 0,
            right: 0,
            bottom: 0,
            backgroundColor: 'rgba(0, 0, 0, 0.5)',
            zIndex: 50,
            transition: 'opacity 0.3s'
          }}
        />
      )}

      {/* Sidebar */}
      <div style={{
        position: 'fixed',
        top: 0,
        right: 0,
        bottom: 0,
        width: '300px',
        backgroundColor: '#ffffff',
        boxShadow: '-2px 0 8px rgba(0, 0, 0, 0.15)',
        zIndex: 51,
        transform: sidebarOpen ? 'translateX(0)' : 'translateX(100%)',
        transition: 'transform 0.3s ease-in-out',
        display: 'flex',
        flexDirection: 'column'
      }}>
        {/* Close button */}
        <div style={{ padding: '1rem', textAlign: 'right' }}>
          <button
            onClick={() => setSidebarOpen(false)}
            style={{
              background: 'none',
              border: 'none',
              fontSize: '1.5rem',
              cursor: 'pointer',
              color: '#6b7280',
              lineHeight: 1
            }}
          >
            ×
          </button>
        </div>

        {/* User info */}
        {user && (
          <div style={{ padding: '0 1.5rem 1.5rem' }}>
            <div style={{ fontWeight: 700, fontSize: '1.125rem', marginBottom: '0.25rem' }}>
              {user.name}
            </div>
            <div style={{ fontSize: '0.875rem', color: '#6b7280' }}>
              {user.email}
            </div>
          </div>
        )}

        {/* Navigation buttons */}
        <div style={{ padding: '0 1.5rem', flex: 1 }}>
          <button
            onClick={() => {
              navigate('/rides')
              setSidebarOpen(false)
            }}
            style={{
              width: '100%',
              padding: '0.75rem',
              marginBottom: '0.75rem',
              backgroundColor: '#a9abab',
              color: 'white',
              border: 'none',
              borderRadius: '0.5rem',
              fontSize: '1rem',
              fontWeight: 500,
              cursor: 'pointer',
              transition: 'background-color 0.3s'
            }}
            onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#92949e'}
            onMouseLeave={(e) => e.currentTarget.style.backgroundColor = '#a9abab'}
          >
            Rides
          </button>
          
          <button
            onClick={() => {
              navigate('/ride-history')
              setSidebarOpen(false)
            }}
            style={{
              width: '100%',
              padding: '0.75rem',
              backgroundColor: '#a9abab',
              color: 'white',
              border: 'none',
              borderRadius: '0.5rem',
              fontSize: '1rem',
              fontWeight: 500,
              cursor: 'pointer',
              transition: 'background-color 0.3s'
            }}
            onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#92949e'}
            onMouseLeave={(e) => e.currentTarget.style.backgroundColor = '#a9abab'}
          >
            Ride History
          </button>
        </div>

        {/* Logout button at bottom */}
        <div style={{ padding: '1.5rem' }}>
          <button
            onClick={handleLogout}
            style={{
              width: '100%',
              padding: '0.75rem',
              backgroundColor: '#e07d2e',
              color: 'white',
              border: 'none',
              borderRadius: '0.5rem',
              fontSize: '1rem',
              fontWeight: 500,
              cursor: 'pointer',
              transition: 'background-color 0.3s'
            }}
            onMouseEnter={(e) => e.currentTarget.style.backgroundColor = '#c96b1f'}
            onMouseLeave={(e) => e.currentTarget.style.backgroundColor = '#e07d2e'}
          >
            Logout
          </button>
        </div>
      </div>
    </>
  )
}