# UniRide Frontend

This is a minimal React + TypeScript + Vite + Tailwind frontend scaffold for UniRide.

Base API URL: http://localhost:8080

Quick start:

```powershell
cd "c:\Users\MY PC\Desktop\ARSALA\.vscode\DSA\UniRide\UniRide\frontend"
npm install
npm run dev
```

Notes:
- Replace the placeholder Google sign-in in `src/pages/Landing.tsx` with a proper OAuth flow.
- The Axios client is in `src/api/client.ts` and points to the local API.
- Add or adapt components to match backend routes in `UniRide_API_Guide.md`.

Google Sign-In (developer setup)
- Create a Google Cloud OAuth 2.0 Client ID (Web application) and add
	`http://localhost:5173` to Authorized JavaScript origins.
- Add the client id to an env file at the project root (Vite expects VITE_ prefix):

```powershell
# .env
VITE_GOOGLE_CLIENT_ID=your-google-client-id.apps.googleusercontent.com
```

Then restart the dev server. The landing page will render the Google Sign-In button and enforce `@cloud.neduet.edu.pk` emails.
