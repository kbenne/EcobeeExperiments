import { useState, useEffect } from 'react';
import axios from 'axios';
import dayjs from 'dayjs';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Box from '@mui/material/Box';
import Slider from '@mui/material/Slider';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';
import Temperature from './Temperature';
import HvacStatus from './HvacStatus';
import Grid from '@mui/material/Grid2';
import Paper from '@mui/material/Paper';

function SessionView({ selectedDateTime, onBack }) {
  const [tempOffset, setTempOffset] = useState(0);
  const [indoorTemp, setIndoorTemp] = useState(72.5);
  const [outdoorTemp, setOutdoorTemp] = useState(68.0);
  const [currentDateTime, setCurrentDateTime] = useState(dayjs(selectedDateTime));
  const [hvacStatus, setHvacStatus] = useState('Off'); // Default to "Off"

  const handleOffsetChange = (event, newValue) => {
    setTempOffset(newValue);
  };

  useEffect(() => {
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

    fetchData(); // Initial fetch

    const intervalId = setInterval(fetchData, 1000); // Fetch every 1 seconds

    return () => clearInterval(intervalId); // Cleanup on unmount
  }, []);

  const handleStopSimulation = async () => {
    try {
      // 1) Signal the Flask server to stop
      await axios.post('http://127.0.0.1:5000/api/stop_simulation');

      // 2) Then go back to SetupView
      onBack();
    } catch (error) {
      console.error('Error stopping simulation:', error);
      alert('Failed to stop simulation!');
    }
  };

  return (
    <Box sx={{ width: '100%' }}>
      {/* Header */}
      <Typography 
        variant="h4" 
        sx={{ 
          color: 'primary.main', 
          fontWeight: 'medium',
          mb: 3 
        }}
      >
        Current Simulation Time
      </Typography>
      
      {/* DateTime Display */}
      <Box 
        sx={{ 
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          mb: 4
        }}
      >
        <Typography 
          variant="h2" 
          sx={{ 
            fontFamily: 'monospace',
            fontWeight: 'bold',
            color: 'text.primary'
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
            color: 'text.secondary'
          }}
        >
          {currentDateTime.isValid()
            ? currentDateTime.format('MMMM D, YYYY')
            : ''}
        </Typography>
      </Box>

      {/* Main Content Grid */}
      <Grid container spacing={3} sx={{ mb: 4 }}>
        {/* Indoor Conditions Section */}
        <Grid item xs={12}>
          <Paper sx={{ p: 3, borderRadius: 2 }}>
            <Typography variant="h6" sx={{ mb: 3 }}>
              Indoor Conditions
            </Typography>
            
            <Grid container spacing={3}>
              <Grid item xs={12} md={6}>
                <Temperature
                  value={indoorTemp} 
                />
              </Grid>

              <Grid item xs={12} md={6}>
                <HvacStatus status={hvacStatus} /> 
              </Grid>
            </Grid>
          </Paper>
        </Grid>

        <Grid item xs={12}>
          <Paper sx={{ p: 3, borderRadius: 2 }}>
            <Typography variant="h6" sx={{ mb: 3 }}>
              Outdoor Conditions
            </Typography>
            
            <Grid container spacing={3}>
              <Grid item xs={12} md={6}>
                  <Temperature 
                    value={outdoorTemp + tempOffset} 
                  />
              </Grid>

              <Grid item xs={12} md={6}>
                <Typography gutterBottom>
                  Temperature Offset: 
                  <Box component="span" sx={{ display: 'inline-block', width: 48, textAlign: 'right' }}>
                    {tempOffset >= 0 ? '+' : '-'}
                    {Math.abs(tempOffset).toFixed(1)}
                  </Box>
                  °F
                </Typography>
                <Slider
                  value={tempOffset}
                  onChange={handleOffsetChange}
                  min={-10}
                  max={10}
                  step={0.5}
                  marks={[
                    { value: -10, label: '-10°F' },
                    { value: 0, label: '0°F' },
                    { value: 10, label: '+10°F' },
                  ]}
                  valueLabelDisplay="auto"
                  sx={{ color: 'primary.main' }}
                />
              </Grid>
            </Grid>
          </Paper>
        </Grid>
      </Grid>

      {/* Back Button */}
      <Box sx={{ display: 'flex', justifyContent: 'center' }}>
        <Button 
          variant="outlined" 
          startIcon={<ArrowBackIcon />}
          onClick={handleStopSimulation}  // Use the new handler
          size="large"
          sx={{ 
            borderRadius: 2,
            textTransform: 'none'
          }}
        >
          Start a new simulation
        </Button>
      </Box>
    </Box>
  );
}

export default SessionView; 