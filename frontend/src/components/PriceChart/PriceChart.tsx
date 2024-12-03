import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer } from 'recharts';

export default function PriceChart() {
  const data = [
    { time: '12:00', price: 49500 },
    { time: '12:05', price: 49700 },
    { time: '12:10', price: 49600 },
    { time: '12:15', price: 49800 },
    { time: '12:20', price: 49900 },
    { time: '12:25', price: 49850 },
  ];

  return (
    <div className="h-[400px]">
      <ResponsiveContainer width="100%" height="100%">
        <LineChart data={data}>
          <CartesianGrid strokeDasharray="3 3" />
          <XAxis dataKey="time" />
          <YAxis domain={['auto', 'auto']} />
          <Tooltip />
          <Line type="monotone" dataKey="price" stroke="#2563eb" />
        </LineChart>
      </ResponsiveContainer>
    </div>
  );
}