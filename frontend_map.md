# Proposed UI Structure

1. **Landing Page**
   - “Register” or “Login” buttons
   - Should specify that the users can only register with the NED university cloud emails

2. **Dashboard**
   - Tabs: [New Ride] 
   The new ride option should take the user to a page where he enters the preferences for finding a ride
   - gives options for selecting if the user is a car or bike owner and want to share the ride, if the user is not owner and wants to join a ride , if the user wants to book a car/rickshaw with other people
   - take all respective api flows from complete_testing_guide.md for all cases
   - for the ones posting as owners, they shall post their rides and others who are looking to join will see them listed there as options
   - for those that are looking for people to book together with them, shall have the options appear if any exsiting rides match their preferences or their own shall get posted as one and they shall wait for others to join
   [My Rides]: this page shall show current ride details  
3. **Chat Page**
   - Opens when the user has successfully booked a ride and is not continuing to proceed.
   - Displays current conversations for the current ride
   - Text input for sending messages (→ POST /chat/send)
   - Show message history by polling or fetching periodically
