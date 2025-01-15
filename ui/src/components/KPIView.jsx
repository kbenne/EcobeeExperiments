import { useState, useEffect } from 'react';
import { Chart as ChartJS, CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend } from 'chart.js';
import { Bar } from 'react-chartjs-2';
import Plot from 'react-plotly.js';
import axios from 'axios';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Box from '@mui/material/Box';
import Radio from '@mui/material/Radio';
import RadioGroup from '@mui/material/RadioGroup';
import FormControlLabel from '@mui/material/FormControlLabel';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';

// Register Chart.js components
ChartJS.register(CategoryScale, LinearScale, BarElement, Title, Tooltip, Legend);

function KPIView({ onBack }) {
  const [kpiData, setKpiData] = useState(null);
  const [viewMode, setViewMode] = useState('kpi'); // Default view mode is 'kpi'
  const [plotData, setPlotData] = useState(null);
  const [selectedVariable, setSelectedVariable] = useState(null); // For dropdown selection in Plotly

  useEffect(() => {
    const fetchKpisAndResults = async () => {
      try {
        const response = await axios.get('http://127.0.0.1:5000/kpi');
        setKpiData(response.data.kpi); // Set KPI data
        setPlotData(response.data.results); // Set simulation results

        // Set the initial selected variable for Plotly
        if (response.data.results) {
          setSelectedVariable(Object.keys(response.data.results)[0]);
        }
      } catch (error) {
        console.error('Error fetching KPI and results data:', error);
      }
    };

    fetchKpisAndResults();
  }, []);

  const kpiChartData = {
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

  const chartOptions = {
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
        width: '100%',
        height: '100%',
        display: 'flex',
        flexDirection: 'column',
        justifyContent: 'space-between',
        boxSizing: 'border-box',
        overflow: 'hidden', // Prevent content overflow
        maxHeight: '100%',
        p: 2,
        gap: 2,
      }}
    >
      {/* Main Content */}
      <Box
        sx={{
          flex: 1,
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
          overflow: 'auto', // Allow internal scrolling if content overflows
          maxHeight: 'calc(100% - 80px)', // Reserve space for footer
        }}
      >
        {viewMode === 'kpi' && kpiData ? (
          <Box sx={{ width: '100%', maxHeight: '100%', overflow: 'auto' }}>
            <Bar data={kpiChartData} options={chartOptions} />
          </Box>
        ) : viewMode === 'plotly' && plotData && selectedVariable ? (
          <Box sx={{ width: '100%', overflow: 'auto' }}>
            {/* Dropdown for selecting variable */}
            <select
              value={selectedVariable}
              onChange={(e) => setSelectedVariable(e.target.value)}
              style={{
                marginBottom: '2px',
                padding: '2px',
                fontSize: '13px',
                width: '200px',
              }}
            >
              {Object.keys(plotData).map((key) => (
                <option key={key} value={key}>
                  {key}
                </option>
              ))}
            </select>

            {/* Plotly Chart */}
            <Plot
              data={[
                {
                  x: Array.from({ length: plotData[selectedVariable].length }, (_, i) => i + 1),
                  y: plotData[selectedVariable],
                  type: 'scatter',
                  mode: 'lines+markers',
                  marker: { color: 'blue' },
                },
              ]}
              layout={{
                title: `Line Plot of ${selectedVariable}`,
                xaxis: { title: 'Step' },
                yaxis: { title: selectedVariable },
                responsive: true,
                autosize: true,
              }}
              style={{
                width: '100%', // Ensure the chart takes the full width of the parent
                maxHeight: '100%', // Prevent it from exceeding parent height
                height: '320px', // Set a fixed height
                overflow: 'auto', // Allow scrolling if needed
              }}
              useResizeHandler
            />
          </Box>
        ) : (
          <Typography variant="h6" sx={{ color: 'text.secondary', flex: 1 }}>
            Loading data...
          </Typography>
        )}
      </Box>

      {/* Footer Section */}
      <Box
        sx={{
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
        }}
      >
        {/* Radio Buttons */}
        <RadioGroup
          row
          value={viewMode}
          onChange={(event) => setViewMode(event.target.value)}
          sx={{
            mx: 'auto',
          }}
        >
          <FormControlLabel value="kpi" control={<Radio />} label="KPI Chart" />
          <FormControlLabel value="plotly" control={<Radio />} label="Line Plot" />
        </RadioGroup>

        {/* Back Button */}
        <Button
          variant="outlined"
          startIcon={<ArrowBackIcon />}
          onClick={onBack}
          size="large"
          sx={{
            borderRadius: 2,
            textTransform: 'none',
            px: 3,
          }}
        >
          Start a new simulation
        </Button>
      </Box>
    </Box>
  );
}

export default KPIView;
