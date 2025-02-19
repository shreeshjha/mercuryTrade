/** @type {import('tailwindcss').Config} */
export default {
  content: [
    "./index.html",
    "./src/**/*.{js,ts,jsx,tsx}",
  ],
  theme: {
    extend: {
      colors: {
        trading: {
          buy: '#22c55e',  // Green
          sell: '#ef4444', // Red
          neutral: '#64748b' // Slate
        }
      }
    },
  },
  plugins: [
    require('@tailwindcss/forms'),
  ],
}