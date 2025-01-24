import { useState, useEffect } from 'react';
import axios from 'axios';
import dayjs from 'dayjs';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Box from '@mui/material/Box';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';
import BarChartIcon from '@mui/icons-material/BarChart';
import Temperature from './Temperature';
import HvacStatus from './HvacStatus';
import Paper from '@mui/material/Paper';
import Backdrop from '@mui/material/Backdrop';
import CircularProgress from '@mui/material/CircularProgress';

function SessionView({ selectedDateTime, onBack, onShowKPI }) {
  const [indoorTemp, setIndoorTemp] = useState(72.5);
  const [outdoorTemp, setOutdoorTemp] = useState(68.0);
  const [currentDateTime, setCurrentDateTime] = useState(dayjs(selectedDateTime));
  const [hvacStatus, setHvacStatus] = useState('Off'); // Default to "Off"
  const [loading, setLoading] = useState(true); // Loading state

  useEffect(() => {
    const checkSimulationReady = async () => {
      try {
        const response = await axios.get('http://127.0.0.1:5000/sim_ready');
        if (response.data.simulationReady) {
          setLoading(false);
        } else {
          setTimeout(checkSimulationReady, 1000); // Retry after 1 second
        }
      } catch (error) {
        console.error('Error checking simulation readiness:', error);
      }
    };

    checkSimulationReady();

    const fetchData = async () => {
      try {
        const response = await fetch('http://127.0.0.1:5000/sim_data');
        const data = await response.json();
        setOutdoorTemp(data.outdoorTemp);
        setIndoorTemp(data.indoorTemp);
        setCurrentDateTime(dayjs(data.currentDateTime));
        setHvacStatus(data.hvacStatus);
      } catch (error) {
        console.error('Error fetching temperature:', error);
      }
    };

    const intervalId = setInterval(fetchData, 1000); // Fetch every second

    return () => clearInterval(intervalId);
  }, []);

  const handleStopSimulation = async () => {
    try {
      // Signal the Flask server to stop
      await axios.post('http://127.0.0.1:5000/api/stop_simulation');
      // Then go back to SetupView
      onBack();
    } catch (error) {
      console.error('Error stopping simulation:', error);
      alert('Failed to stop simulation!');
    }
  };

  const handleViewKPIs = async () => {
    try {
      // Signal the Flask server to pause
      await axios.post('http://127.0.0.1:5000/api/pause_simulation');
      // Then go back to SetupView
      onShowKPI();
    } catch (error) {
      console.error('Error showing KPIs:', error);
      alert('Failed to show KPIs!');
    }
  };

  return (
    <>
      {/* Loading Spinner */}
      <Backdrop open={loading} sx={{ color: '#fff', zIndex: (theme) => theme.zIndex.drawer + 1 }}>
        <CircularProgress color="inherit" />
      </Backdrop>

      {/* Parent Box occupies the entire area (height is 540px due to 60px header). */}
      <Box
        sx={{
          minWidth: '1000px',
          minHeight:'460px',
          display: 'flex',
          flexDirection: 'column',
          p: 3,
        }}
      >
        {/* Header Section */}
        <Box sx={{ textAlign: 'center', mb: 1 }}>
          <Typography
            variant="h4"
            sx={{
              color: 'primary.main',
              fontWeight: 'medium',
            }}
          >
            Current Simulation Time
          </Typography>

          {/* Time & Date */}
          <Typography
            variant="h2"
            sx={{
              fontFamily: 'monospace',
              fontWeight: 'bold',
              color: 'text.primary',
            }}
          >
            {currentDateTime.isValid()
              ? currentDateTime.format('HH:mm:ss')
              : 'Invalid date'}
          </Typography>

          <Typography
            variant="h5"
            sx={{
              fontFamily: 'monospace',
              color: 'text.secondary',
            }}
          >
            {currentDateTime.isValid()
              ? currentDateTime.format('MMMM D, YYYY')
              : ''}
          </Typography>
        </Box>

        {/* Conditions Section (Indoor & Outdoor Side-by-Side) */}
        <Box
          sx={{
            display: 'flex',
            flexDirection: 'row',
            gap: 2,
            flex: 1,
          }}
        >
          {/* Indoor Conditions */}
          <Paper
            sx={{
              flex: 1,
              p: 2,
			        gap: 2,
              borderRadius: 2,
              display: 'flex',
              flexDirection: 'column',
            }}
          >
            <Typography variant="h6" sx={{ mb: 2 }}>
              Indoor Conditions
            </Typography>

            <Box>
              <Temperature value={indoorTemp} />
            </Box>
            <Box>
              <HvacStatus status={hvacStatus} />
            </Box>
          </Paper>

          {/* Outdoor Conditions */}
          <Paper
            sx={{
              flex: 1,
              p: 2,
			        gap: 2,
              borderRadius: 2,
              display: 'flex',
              flexDirection: 'column',
            }}
          >
            <Typography variant="h6" sx={{ mb: 2 }}>
              Outdoor Conditions
            </Typography>

            <Box>
              <Temperature value={outdoorTemp} />
            </Box>
          </Paper>
        </Box>

        {/* Bottom Buttons */}
        <Box
          sx={{
            textAlign: 'center',
            mt: 2,
            display: 'flex',
            justifyContent: 'center',
            gap: 2,
          }}
        >
          <Button
            variant="outlined"
            startIcon={<ArrowBackIcon />}
            onClick={handleStopSimulation}
            size="large"
            sx={{
              borderRadius: 2,
              textTransform: 'none',
              px: 3,
            }}
          >
            Start a new simulation
          </Button>

          <Button
            variant="contained"
            startIcon={<BarChartIcon />}
            onClick={handleViewKPIs}
            size="large"
            sx={{
              borderRadius: 2,
              textTransform: 'none',
              px: 3,
            }}
          >
            Stop and View Results
          </Button>
        </Box>
      </Box>
    </>
  );
}

export default SessionView;
