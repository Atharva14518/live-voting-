const API_BASE_URL = import.meta.env.VITE_API_URL || 'http://localhost:3000';

export const api = {
    baseUrl: API_BASE_URL,

    async fetch(endpoint: string, options?: RequestInit) {
        const response = await fetch(`${API_BASE_URL}${endpoint}`, {
            ...options,
            headers: {
                'Content-Type': 'application/json',
                ...options?.headers,
            },
        });
        return response;
    },

    async get(endpoint: string) {
        return this.fetch(endpoint);
    },

    async post(endpoint: string, data: any) {
        return this.fetch(endpoint, {
            method: 'POST',
            body: JSON.stringify(data),
        });
    },
};

export default api;
