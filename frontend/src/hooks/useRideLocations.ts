import { useEffect, useState } from 'react'

export function useRideLocations() {
  const [locations, setLocations] = useState<string[]>(['NED University'])
  useEffect(() => {
    fetch('/locations.csv')
      .then(r => r.text())
      .then(text => {
        const lines = text.split('\n').slice(1) // skip header
        const locs = lines
          .map(l => l.split(',')[0].trim())
          .filter(l => l && l !== 'NED University')
        setLocations(['NED University', ...locs])
      })
      .catch(() => setLocations(['NED University']))
  }, [])
  return locations
}
