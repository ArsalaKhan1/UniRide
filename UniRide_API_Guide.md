# UniRide API Complete Guide

## Overview
UniRide is a ride-sharing API that supports bike, carpool, and rickshaw rides with features including user authentication, ride matching, chat functionality, and request management.

**Base URL:** `http://localhost:8080`

## Table of Contents
1. [System Health Check](#system-health-check)
2. [Authentication](#authentication)
3. [User Preferences](#user-preferences)
4. [Ride Management](#ride-management)
5. [Request System](#request-system)
6. [Chat System](#chat-system)
7. [Complete Workflow Examples](#complete-workflow-examples)

---

## System Health Check

### Check API Status
```bash
curl -X GET http://localhost:8080/
```
**Response:** `"UniRide API is running successfully!"`

---

## Authentication

### Google OAuth Login
Authenticate users using Google OAuth tokens.

```bash
curl -X POST http://localhost:8080/auth/google \
  -H "Content-Type: application/json" \
  -d '{
    "idToken": "your_google_id_token_here",
    "enrollmentId": "student123"
  }'
```

**Response:**
```json
{
  "success": true,
  "user": {
    "id": "user123",
    "name": "John Doe",
    "email": "john@university.edu"
  },
  "sessionToken": "session_token_here",
  "expiresIn": 86400
}
```

---

## User Preferences

### Set User Preferences
Configure user's ride preferences including gender preference and vehicle type.

```bash
curl -X POST http://localhost:8080/user/preferences \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "user123",
    "genderPreference": "any",
    "vehicleType": 1
  }'
```

**Vehicle Types:**
- `0` = BIKE
- `1` = RICKSHAW  
- `2` = CAR_OWNER
- `3` = CAR_BOOKING

**Response:**
```json
{
  "success": true,
  "message": "Preferences updated successfully"
}
```

### Get User Preferences
```bash
curl -X GET http://localhost:8080/user/preferences/user123
```

**Response:**
```json
{
  "genderPreference": "any",
  "vehicleType": 1
}
```

---

## Ride Management

### View All Available Rides
```bash
curl -X GET http://localhost:8080/ride/all
```

**Response:**
```json
{
  "rides": [
    {
      "rideID": 1,
      "leadUserID": "user123",
      "from": "Campus Gate",
      "to": "City Center",
      "time": "now",
      "rideType": "carpool",
      "currentCapacity": 2,
      "maxCapacity": 4,
      "availableSlots": 2,
      "peopleJoined": 2,
      "spotsRemaining": 2,
      "femalesOnly": false
    }
  ]
}
```

### Create Ride Offer (For Vehicle Owners)
Create a ride offer when you own a vehicle and want to share it.

```bash
curl -X POST http://localhost:8080/ride/offer \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "owner123",
    "from": "Campus Gate",
    "to": "City Center",
    "rideType": "carpool",
    "femalesOnly": false
  }'
```

**Ride Types:** `bike`, `carpool`, `rickshaw`

**Response:**
```json
{
  "message": "Ride offer created successfully",
  "rideType": "carpool",
  "maxCapacity": 4,
  "rideID": 1
}
```

### Join Existing Ride
Directly join an available ride.

```bash
curl -X POST http://localhost:8080/ride/join \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "passenger123",
    "from": "Campus Gate",
    "to": "City Center",
    "rideType": "carpool"
  }'
```

**Response:**
```json
{
  "message": "Successfully joined ride",
  "status": "success"
}
```

---

## Request System

### Create Travel Request
Create a request to find or start a ride. The system will either find matches or make you the lead.

```bash
curl -X POST http://localhost:8080/request/create \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "user123",
    "from": "Campus Gate",
    "to": "City Center",
    "rideType": "carpool"
  }'
```

**Response (Matches Found):**
```json
{
  "rideType": "carpool",
  "message": "Found existing travel requests matching your preferences",
  "matches": [
    {
      "rideID": 1,
      "leadUserID": "leader123",
      "from": "Campus Gate",
      "to": "City Center",
      "time": "now",
      "rideType": "carpool",
      "currentCapacity": 1,
      "maxCapacity": 4,
      "availableSlots": 3,
      "peopleJoined": 1,
      "spotsRemaining": 3
    }
  ]
}
```

**Response (No Matches - You Become Lead):**
```json
{
  "rideType": "carpool",
  "message": "No matching requests found. You are now the lead for a new travel request.",
  "rideID": 2,
  "leadUserID": "user123",
  "matches": []
}
```

### Send Join Request
Send a request to join a specific ride.

```bash
curl -X POST http://localhost:8080/ride/request \
  -H "Content-Type: application/json" \
  -d '{
    "userID": "passenger123",
    "rideID": 1
  }'
```

**Response:**
```json
{
  "success": true,
  "message": "Join request sent successfully"
}
```

### View Pending Requests for Your Ride
As a ride owner/lead, view who wants to join your ride.

```bash
curl -X GET http://localhost:8080/ride/1/requests
```

**Response:**
```json
{
  "requests": [
    {
      "userID": "passenger123",
      "timestamp": "2024-01-15 10:30:00"
    }
  ]
}
```

### Approve/Reject Join Requests
As a ride owner/lead, respond to join requests.

```bash
# Approve request
curl -X POST http://localhost:8080/ride/respond \
  -H "Content-Type: application/json" \
  -d '{
    "rideID": 1,
    "userID": "passenger123",
    "accept": true
  }'

# Reject request
curl -X POST http://localhost:8080/ride/respond \
  -H "Content-Type: application/json" \
  -d '{
    "rideID": 1,
    "userID": "passenger123",
    "accept": false
  }'
```

**Response:**
```json
{
  "success": true,
  "message": "Request approved"
}
```

### View All Pending Requests (Legacy)
```bash
curl -X GET http://localhost:8080/request/pending
```

---

## Chat System

### Send Direct Message (Hub-and-Spoke Model)
Send messages between ride participants. Uses hub-and-spoke model where messages go through the ride lead.

```bash
curl -X POST http://localhost:8080/chat/send \
  -H "Content-Type: application/json" \
  -d '{
    "sender": "user123",
    "recipient": "user456",
    "text": "What time should we meet?",
    "rideID": 1
  }'
```

**Response:**
```json
{
  "success": true,
  "error": ""
}
```

### Broadcast Message (Legacy)
Send a message to all users.

```bash
curl -X POST http://localhost:8080/chat/broadcast \
  -H "Content-Type: application/json" \
  -d '{
    "sender": "user123",
    "text": "Looking for a ride to downtown!"
  }'
```

**Response:**
```json
{
  "success": true,
  "message": "Message broadcasted"
}
```

### Get Chat Messages for Specific Ride
```bash
curl -X GET http://localhost:8080/chat/ride/1
```

### Get All Chat Messages (Legacy)
```bash
curl -X GET http://localhost:8080/chat/all
```

**Response:**
```json
{
  "messages": [
    {
      "sender": "user123",
      "text": "Looking for a ride to downtown!"
    }
  ]
}
```

---

## Complete Workflow Examples

### Workflow 1: Vehicle Owner Offering a Ride

1. **Authenticate**
```bash
curl -X POST http://localhost:8080/auth/google \
  -H "Content-Type: application/json" \
  -d '{"idToken": "token123", "enrollmentId": "owner123"}'
```

2. **Set Preferences**
```bash
curl -X POST http://localhost:8080/user/preferences \
  -H "Content-Type: application/json" \
  -d '{"userID": "owner123", "genderPreference": "any", "vehicleType": 2}'
```

3. **Create Ride Offer**
```bash
curl -X POST http://localhost:8080/ride/offer \
  -H "Content-Type: application/json" \
  -d '{"userID": "owner123", "from": "Campus", "to": "Mall", "rideType": "carpool", "femalesOnly": false}'
```

4. **Check for Join Requests**
```bash
curl -X GET http://localhost:8080/ride/1/requests
```

5. **Approve/Reject Requests**
```bash
curl -X POST http://localhost:8080/ride/respond \
  -H "Content-Type: application/json" \
  -d '{"rideID": 1, "userID": "passenger123", "accept": true}'
```

6. **Chat with Passengers**
```bash
curl -X POST http://localhost:8080/chat/send \
  -H "Content-Type: application/json" \
  -d '{"sender": "owner123", "recipient": "passenger123", "text": "Meet at main gate", "rideID": 1}'
```

### Workflow 2: Passenger Looking for a Ride

1. **Authenticate**
```bash
curl -X POST http://localhost:8080/auth/google \
  -H "Content-Type: application/json" \
  -d '{"idToken": "token456", "enrollmentId": "passenger123"}'
```

2. **View Available Rides**
```bash
curl -X GET http://localhost:8080/ride/all
```

3. **Create Travel Request (Find Matches)**
```bash
curl -X POST http://localhost:8080/request/create \
  -H "Content-Type: application/json" \
  -d '{"userID": "passenger123", "from": "Campus", "to": "Mall", "rideType": "carpool"}'
```

4. **Send Join Request to Specific Ride**
```bash
curl -X POST http://localhost:8080/ride/request \
  -H "Content-Type: application/json" \
  -d '{"userID": "passenger123", "rideID": 1}'
```

5. **Chat with Ride Owner**
```bash
curl -X POST http://localhost:8080/chat/send \
  -H "Content-Type: application/json" \
  -d '{"sender": "passenger123", "recipient": "owner123", "text": "What time departure?", "rideID": 1}'
```

### Workflow 3: Group Travel Request (No Vehicle Owner)

1. **Create Request (Becomes Group Lead)**
```bash
curl -X POST http://localhost:8080/request/create \
  -H "Content-Type: application/json" \
  -d '{"userID": "leader123", "from": "Dorm", "to": "Airport", "rideType": "rickshaw"}'
```

2. **Others Join the Request**
```bash
curl -X POST http://localhost:8080/ride/request \
  -H "Content-Type: application/json" \
  -d '{"userID": "member123", "rideID": 2}'
```

3. **Approve Group Members**
```bash
curl -X POST http://localhost:8080/ride/respond \
  -H "Content-Type: application/json" \
  -d '{"rideID": 2, "userID": "member123", "accept": true}'
```

4. **Group Chat Coordination**
```bash
curl -X POST http://localhost:8080/chat/send \
  -H "Content-Type: application/json" \
  -d '{"sender": "leader123", "recipient": "member123", "text": "Rickshaw booked for 3 PM", "rideID": 2}'
```

---

## Key Features

### Ride Types & Capacities
- **Bike**: Max 2 people
- **Carpool**: Max 4 people  
- **Rickshaw**: Max 3 people

### Gender Preferences
- Support for female-only rides
- Gender preference matching

### Smart Matching
- Proximity-based location matching
- Automatic ride suggestions
- Hub-and-spoke chat model

### Request Management
- Pending request tracking
- Approval/rejection system
- Capacity management

---

## Error Responses

### Common Error Codes
- `400`: Invalid JSON or missing required fields
- `401`: Invalid authentication token
- `500`: Database or server error

### Example Error Response
```json
{
  "success": false,
  "message": "Failed to save ride to database"
}
```

---

## Notes

1. **Authentication**: All requests require valid user authentication via Google OAuth
2. **Ride Ownership**: Only bike and carpool rides can have owners; rickshaw rides are group requests
3. **Chat System**: Uses hub-and-spoke model where ride leads facilitate communication
4. **Capacity Management**: System automatically updates ride capacity and status
5. **Gender Preferences**: System respects gender preferences for ride matching
6. **Location Matching**: Uses proximity-based matching for better ride suggestions

This API provides a complete ride-sharing solution with authentication, matching, communication, and management features suitable for university campus transportation needs.