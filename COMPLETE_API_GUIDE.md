# UniRide Complete API Guide

## Table of Contents
1. [Authentication](#authentication)
2. [User Management](#user-management)
3. [Ride Management](#ride-management)
4. [Request Management](#request-management)
5. [Chat System](#chat-system)
6. [User Preferences](#user-preferences)
7. [Error Codes](#error-codes)

---

## Authentication

### Google OAuth Registration/Login
**Endpoint**: `POST /auth/google`

**Description**: Authenticate using Google OAuth with enrollment validation

**Request**:
```bash
curl -X POST http://localhost:8080/auth/google \
  -H "Content-Type: application/json" \
  -d '{
    "idToken": "GOOGLE_ID_TOKEN_HERE",
    "enrollmentId": "NED/1915/2024"
  }'
```

**Valid Enrollment IDs**:
- `NED/0393/2024`
- `NED/0887/2024`
- `NED/1915/2024`

**Response**:
```json
{
  "success": true,
  "user": {
    "id": "116860011496725847632",
    "name": "RIDA RAFIQUE",
    "email": "rafique4735048@cloud.neduet.edu.pk"
  },
  "sessionToken": "94d0b4b3cfab71b7e3761fafdee73973",
  "expiresIn": 86400
}
```

**Error Responses**:
- `400`: Missing idToken or enrollmentId
- `401`: Invalid token or enrollment ID not found

---

## User Management

### Get All Users
**Endpoint**: `GET /users`

```bash
curl http://localhost:8080/users
```

### Set User Preferences
**Endpoint**: `POST /user/preferences`

```bash
curl -X POST http://localhost:8080/user/preferences \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "116860011496725847632",
    "genderPreference": "any",
    "vehicleType": 2
  }'
```

**Vehicle Types**:
- `0`: BIKE
- `1`: RICKSHAW
- `2`: CAR_OWNER
- `3`: CAR_BOOKING

### Get User Preferences
**Endpoint**: `GET /user/preferences/{userID}`

```bash
curl http://localhost:8080/user/preferences/116860011496725847632
```

---

## Ride Management

### Create Ride Offer (For Owners)
**Endpoint**: `POST /ride/offer`

**Description**: Create a ride offer (Bike or Carpool only)

```bash
curl -X POST http://localhost:8080/ride/offer \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "116860011496725847632",
    "from": "NED Campus",
    "to": "Saddar",
    "rideType": "carpool",
    "femalesOnly": false
  }'
```

**Ride Types**: `bike`, `carpool`

### Get All Rides
**Endpoint**: `GET /ride/all`

```bash
curl http://localhost:8080/ride/all
```

**Response Fields**:
- `currentCapacity`: Current participants
- `maxCapacity`: Maximum capacity
- `availableSlots`: Remaining slots
- `femalesOnly`: Gender restriction


### Join Existing Ride
**Endpoint**: `POST /ride/join`

```bash
curl -X POST http://localhost:8080/ride/join \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "116860011496725847632",
    "from": "NED Campus",
    "to": "Saddar",
    "rideType": "carpool"
  }'
```

---

## Request Management

### Create Ride Request
**Endpoint**: `POST /request/create`

**Description**: Create a ride request (all ride types)

```bash
curl -X POST http://localhost:8080/request/create \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "116860011496725847632",
    "from": "NED Campus",
    "to": "Saddar",
    "rideType": "rickshaw"
  }'
```

**Ride Types**: `bike`, `carpool`, `rickshaw`

### Send Join Request
**Endpoint**: `POST /ride/request`

```bash
curl -X POST http://localhost:8080/ride/request \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "116860011496725847632",
    "rideID": 1
  }'
```

### Approve/Reject Join Request
**Endpoint**: `POST /ride/respond`

```bash
curl -X POST http://localhost:8080/ride/respond \
  -H "Content-Type: application/json" \
  -d '{
    "rideID": 1,
    "userID": "116860011496725847632",
    "accept": true
  }'
```

### Get Pending Requests for Ride
**Endpoint**: `GET /ride/{rideID}/requests`

```bash
curl http://localhost:8080/ride/1/requests
```

### View All Pending Requests
**Endpoint**: `GET /request/pending`

```bash
curl http://localhost:8080/request/pending
```

---

## Chat System

### Send Message (Hub-and-Spoke)
**Endpoint**: `POST /chat/send`

```bash
curl -X POST http://localhost:8080/chat/send \
  -H "Content-Type: application/json" \
  -d '{
    "sender": "116860011496725847632",
    "recipient": "OTHER_USER_ID",
    "text": "Are you ready to leave?",
    "rideID": 1
  }'
```

### Broadcast Message
**Endpoint**: `POST /chat/broadcast`

```bash
curl -X POST http://localhost:8080/chat/broadcast \
  -H "Content-Type: application/json" \
  -d '{
    "sender": "116860011496725847632",
    "text": "Looking for carpool to Saddar!"
  }'
```

### Get Chat Messages for Ride
**Endpoint**: `GET /chat/ride/{rideID}`

```bash
curl http://localhost:8080/chat/ride/1
```

### Get All Chat Messages
**Endpoint**: `GET /chat/all`

```bash
curl http://localhost:8080/chat/all
```

---

## Ride Types & Capacity

### Bike Ride
- **Max Capacity**: 2 (Owner + 1 Passenger)
- **Owner Required**: Yes
- **Usage**: Personal bike sharing

### Carpool Ride
- **Max Capacity**: 4-5 people
- **Owner Required**: Yes (car owner)
- **Usage**: Car sharing with driver

### Rickshaw Ride
- **Max Capacity**: 3 Participants
- **Owner Required**: No (shared booking)
- **Usage**: Group booking for rickshaw

---

## Authentication Flow

1. **Get Google ID Token**: Use Google OAuth 2.0
2. **Provide Enrollment ID**: Must be registered student
3. **System Validates**: Both Google token and enrollment
4. **Receive Session Token**: Use for subsequent API calls
5. **Session Expires**: After 24 hours (86400 seconds)

---

## Error Codes

### Authentication Errors
- `400`: Missing required fields (idToken, enrollmentId)
- `401`: Invalid Google token or enrollment ID not found
- `500`: Server error during authentication

### API Errors
- `400`: Invalid JSON or missing required fields
- `404`: Resource not found (ride, user, etc.)
- `500`: Database or server error

### Business Logic Errors
- **Ride Full**: "No available rides found or ride is full"
- **Duplicate Request**: "You already have a pending request"
- **Invalid Ride Type**: "Rickshaw rides cannot have owners"

---

## Example Complete Workflow

### 1. Authenticate
```bash
curl -X POST http://localhost:8080/auth/google \
  -H "Content-Type: application/json" \
  -d '{"idToken":"TOKEN","enrollmentId":"NED/1915/2024"}'
```

### 2. Set Preferences
```bash
curl -X POST http://localhost:8080/user/preferences \
  -H "Content-Type: application/json" \
  -d '{"userID":"YOUR_USER_ID","genderPreference":"any","vehicleType":2}'
```

### 3. Create or Search for Rides
```bash
# Create ride offer
curl -X POST http://localhost:8080/ride/offer \
  -H "Content-Type: application/json" \
  -d '{"userID":"YOUR_USER_ID","from":"NED","to":"Saddar","rideType":"carpool"}'

# OR search for existing rides
curl -X POST http://localhost:8080/rides/search \
  -H "Content-Type: application/json" \
  -d '{"userID":"YOUR_USER_ID","from":"NED","to":"Saddar","rideType":"carpool"}'
```

### 4. Join or Request to Join
```bash
curl -X POST http://localhost:8080/ride/request \
  -H "Content-Type: application/json" \
  -d '{"userID":"YOUR_USER_ID","rideID":1}'
```

### 5. Chat with Ride Members
```bash
curl -X POST http://localhost:8080/chat/send \
  -H "Content-Type: application/json" \
  -d '{"sender":"YOUR_USER_ID","recipient":"OTHER_USER","text":"Hello!","rideID":1}'
```

---

## Notes

- **Session Tokens**: Valid for 24 hours, use for authentication
- **Enrollment Validation**: Only registered students can authenticate
- **Thread Safety**: All operations are thread-safe with mutex protection
- **Database**: SQLite with persistent storage
- **Real-time**: Chat system supports ride-specific messaging

**Base URL**: `http://localhost:8080`
**Content-Type**: `application/json` for all POST requests