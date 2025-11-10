import axios from 'axios'

// Vite exposes env vars via import.meta.env and requires the VITE_ prefix for user-defined vars.
const API_BASE = (import.meta as any).env?.VITE_API_BASE || 'http://localhost:8080'

const api = axios.create({
  baseURL: API_BASE,
  headers: {
    'Content-Type': 'application/json',
  },
  withCredentials: true,
})

// Attach token from localStorage when present
api.interceptors.request.use((config) => {
  const raw = localStorage.getItem('uniride_user')
  if (raw) {
    try {
      const user = JSON.parse(raw)
      if (user?.token) {
        config.headers = config.headers || {}
        ;(config.headers as any).Authorization = `Bearer ${user.token}`
      }
    } catch (e) {
      // ignore
    }
  }
  return config
})

// High-level API wrappers matching UniRide API Guide
export const authAPI = {
  async google(idToken: string, enrollmentId: string) {
    return api.post('/auth/google/verify', { idToken, enrollmentId })
  },
}

export const userAPI = {
  async setPreferences(payload: any) {
    return api.post('/user/preferences', payload)
  },
  async getPreferences(userID: string) {
    return api.get(`/user/preferences/${userID}`)
  },
}

export const rideAPI = {
  async getAll() {
    return api.get('/ride/all')
  },
  async offer(payload: any) {
    return api.post('/ride/offer', payload)
  },
  async join(payload: any) {
    return api.post('/ride/join', payload)
  },
  async sendJoinRequest(payload: any) {
    return api.post('/ride/request', payload)
  },
  async getRideRequests(rideID: number) {
    return api.get(`/ride/${rideID}/requests`)
  },
  async respondToRequest(payload: any) {
    return api.post('/ride/respond', payload)
  },
}

export const requestAPI = {
  async create(payload: any) {
    return api.post('/request/create', payload)
  },
  async pending() {
    return api.get('/request/pending')
  },
}

export const chatAPI = {
  async send(payload: any) {
    return api.post('/chat/send', payload)
  },
  async broadcast(payload: any) {
    return api.post('/chat/broadcast', payload)
  },
  async byRide(rideID: number) {
    return api.get(`/chat/ride/${rideID}`)
  },
  async all() {
    return api.get('/chat/all')
  },
}

export default api
