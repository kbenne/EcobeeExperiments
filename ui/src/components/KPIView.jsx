import { useState, useEffect } from 'react';
import { Chart as ChartJS, CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend } from 'chart.js';
import { Bar } from 'react-chartjs-2';
import axios from 'axios';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Box from '@mui/material/Box';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';

// Register Chart.js components
ChartJS.register(CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend);

function KPIView({ onBack }) {
  const [kpiData, setKpiData] = useState(null);

  useEffect(() => {
    const fetchKpis = async () => {
      try {
        const response = await axios.get('http://127.0.0.1:5000/kpi'); // Replace with correct endpoint if needed
        setKpiData(response.data);
      } catch (error) {
        console.error('Error fetching KPI data:', error);
      }
    };

    fetchKpis();
  }, []);

  const data = {
    labels: kpiData ? Object.keys(kpiData) : [],
    datasets: [
      {
        label: 'KPI Values',
        data: kpiData ? Object.values(kpiData) : [],
        backgroundColor: 'rgba(75, 192, 192, 0.6)',
        borderColor: 'rgba(75, 192, 192, 1)',
        borderWidth: 1,
      },
    ],
  };

  const options = {
    responsive: true,
    plugins: {
      legend: {
        display: false,
      },
    },
    scales: {
      y: {
        beginAtZero: true,
      },
    },
  };

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        justifyContent: 'center',
        textAlign: 'center',
        p: 3,
      }}
    >
      <Typography variant="h4" sx={{ mb: 3, fontWeight: 'bold' }}>
        Simulation KPIs
      </Typography>

      {kpiData ? (
        <Box sx={{ width: '80%', mb: 4 }}>
          <Bar data={data} options={options} />
        </Box>
      ) : (
        <Typography variant="h6" sx={{ color: 'text.secondary' }}>
          Loading KPI data...
        </Typography>
      )}

      <Button
        variant="outlined"
        startIcon={<ArrowBackIcon />}
        onClick={onBack}
        size="large"
        sx={{
          mt: 3,
          borderRadius: 2,
          textTransform: 'none',
          px: 3,
        }}
      >
        Back to Session
      </Button>
    </Box>
  );
}

export default KPIView;
