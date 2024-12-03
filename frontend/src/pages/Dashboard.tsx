export default function Dashboard() {
    return (
      <div>
        <h1 className="text-2xl font-semibold text-gray-900">Dashboard</h1>
        <div className="mt-6 grid grid-cols-1 gap-6 md:grid-cols-2 lg:grid-cols-3">
          {/* Add dashboard widgets here */}
          <div className="rounded-lg bg-white shadow p-6">
            <h2 className="text-lg font-medium text-gray-900">Market Overview</h2>
            {/* Add content */}
          </div>
        </div>
      </div>
    )
  }