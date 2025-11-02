# UniRide API Documentation

## User Roles
- **Ride Owner (Driver)**: Creates ride offers (Bike or Car only)
- **Passenger**: Joins existing ride offers
- **Participant**: Joins shared booking groups (Rickshaw or Carpool)

## Ride Types

### 1. Bike Ride
- **Capacity**: 2 people (Owner + 1 Passenger)
- **Owner**: Required (bike owner)
- **Passengers**: 1 maximum

### 2. Carpool Ride
- **Capacity**: 5 people (Owner + 4 Passengers) OR 4 Participants
- **Owner**: Optional (can be owner-based or participant-based)
- **Passengers/Participants**: 4 maximum

### 3. Rickshaw Ride
- **Capacity**: 3 Participants
- **Owner**: None (all participants are equal)
- **Participants**: 3 maximum

## API Endpoints

### Create Ride Offer (For Owners)
```
POST /ride/offer
{
    "userID": "user123",
    "from": "Location A",
    "to": "Location B",
    "rideType": "bike" | "carpool"
}
```

### Create Ride Request (For Passengers/Participants)
```
POST /request/create
{
    "userID": "user123",
    "from": "Location A", 
    "to": "Location B",
    "rideType": "bike" | "carpool" | "rickshaw"
}
```

### Join Existing Ride
```
POST /ride/join
{
    "userID": "user123",
    "from": "Location A",
    "to": "Location B", 
    "rideType": "bike" | "carpool" | "rickshaw"
}
```

### Get All Rides
```
GET /ride/all
```
Returns rides with capacity information:
- `currentCapacity`: Current number of participants
- `maxCapacity`: Maximum allowed participants
- `availableSlots`: Remaining slots available

## Matching Rules

1. **Same Ride Type**: Users only match with same ride type
2. **Compatible Locations**: Same source and destination areas
3. **Capacity Limits**: 
   - Bike: 2 total (owner + 1 passenger)
   - Carpool: 5 total (owner + 4) or 4 participants
   - Rickshaw: 3 participants (no owner)
4. **Role Restrictions**:
   - Bike owners can only match with passengers
   - Two passengers cannot match for bike rides
   - Rickshaw has no owners, only participants