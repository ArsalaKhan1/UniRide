import React, { useState } from 'react'
import RideOfferForm from '../components/RideOfferForm'
import RideRequestForm from '../components/RideRequestForm'

export default function RideInteractionScreen() {
  const [choice, setChoice] = useState<null | 'offer' | 'request'>(null)

  if (!choice) return (
    <div className="max-w-2xl mx-auto p-8 bg-white rounded-2xl shadow-lg mt-8">
      <h1 className="text-3xl font-bold mb-2 text-gray-900">Looking for a new ride?</h1>
      <p className="mb-6 text-gray-600">Fill in a few details and preferences, and you’ll be finding one in no time!</p>
      <div className="mb-6 font-medium text-lg text-gray-900">Are you a car/bike owner looking for a carpooling passenger?</div>
      <div className="flex gap-4">
        <button
          className="px-6 py-2 bg-blue-600 text-white rounded-lg hover:bg-blue-700"
          onClick={() => setChoice('offer')}
        >Yes</button>
        <button
          className="px-6 py-2 bg-gray-100 text-gray-900 rounded-lg hover:bg-gray-200"
          onClick={() => setChoice('request')}
        >No</button>
      </div>
    </div>
  )

  return choice === 'offer' ? <RideOfferForm /> : <RideRequestForm />
}
