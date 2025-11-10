import React, { createContext, useContext, useEffect, useState } from 'react'
import { authAPI } from '../api/client'

type User = {
  id: number
  name: string
  email: string
  token?: string
}

type AuthContextType = {
  user: User | null
  login: (user: User) => void
  logout: () => void
  loginWithGoogle: (idToken: string, enrollmentId: string) => Promise<void>
}

const AuthContext = createContext<AuthContextType | undefined>(undefined)

export const AuthProvider = ({ children }: { children: React.ReactNode }) => {
  const [user, setUser] = useState<User | null>(null)

  useEffect(() => {
    const raw = localStorage.getItem('uniride_user')
    if (raw) setUser(JSON.parse(raw))
  }, [])

  useEffect(() => {
    if (user) localStorage.setItem('uniride_user', JSON.stringify(user))
    else localStorage.removeItem('uniride_user')
  }, [user])

  const login = (u: User) => setUser(u)
  const logout = () => setUser(null)

  const loginWithGoogle = async (idToken: string, enrollmentId: string) => {
    try {
      const res = await authAPI.google(idToken, enrollmentId)
      const data = res.data
      if (data?.success) {
        const u: User = {
          id: data.user?.id || data.userID || data.user?.userID || data.user,
          name: data.user?.name || data.user?.username || '',
          email: data.user?.email || '',
          token: data.sessionToken || data.token,
        }
        setUser(u)
      } else {
        // propagate backend message if present
        throw new Error(data?.message || data?.error || 'Login failed')
      }
    } catch (err: any) {
      const serverMsg = err?.response?.data?.error || err?.response?.data?.message
      const status = err?.response?.status
      throw new Error(serverMsg ? `Server: ${serverMsg}` : err?.message || `Network error${status ? ` (${status})` : ''}`)
    }
  }

  return (
    <AuthContext.Provider value={{ user, login, logout, loginWithGoogle }}>
      {children}
    </AuthContext.Provider>
  )
}

export const useAuth = () => {
  const ctx = useContext(AuthContext)
  if (!ctx) throw new Error('useAuth must be used within AuthProvider')
  return ctx
}
