# CrowdDecision - Deployment Guide

## Quick Deploy Options

### Option 1: Railway (Recommended - Free Tier)

1. **Create Railway Account**: https://railway.app
2. **Install Railway CLI**:
   ```bash
   npm install -g @railway/cli
   railway login
   ```
3. **Deploy Backend**:
   ```bash
   cd server
   railway init
   railway up
   ```
4. **Deploy Frontend**:
   ```bash
   cd web-client
   npm run build
   railway init
   railway up
   ```

### Option 2: Render (Free Tier)

1. Push code to GitHub
2. Go to https://render.com
3. Create new Web Service → Connect GitHub repo
4. Set build command: `npm install && npm run build`
5. Set start command: `npm run start`

### Option 3: Vercel + Railway

- **Frontend**: Deploy to Vercel (free, fast)
  ```bash
  cd web-client
  npx vercel
  ```
- **Backend**: Deploy to Railway (free tier)

## Environment Variables

Set these in your deployment platform:

```env
JWT_SECRET=your_super_secure_random_string_here
NODE_ENV=production
PORT=3000
```

## Production Checklist

- [ ] Set strong JWT_SECRET
- [ ] Enable HTTPS
- [ ] Update CORS origins in server
- [ ] Set up monitoring/logging
