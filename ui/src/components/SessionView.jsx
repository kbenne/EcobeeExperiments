import { useState, useEffect } from 'react';
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
  const hvacStatus = 'Cool'; // This would typically come from your HVAC system

  const handleOffsetChange = (event, newValue) => {
    setTempOffset(newValue);
  };

  useEffect(() => {
    const fetchTemperature = async () => {
      try {
        const response = await fetch('https://api.example.com/temperature');
        const data = await response.json();
        setOutdoorTemp(data.outdoorTemp);
        setIndoorTemp(data.indoorTemp);
      } catch (error) {
        console.error('Error fetching temperature:', error);
      }
    };

    fetchTemperature(); // Initial fetch

    const intervalId = setInterval(fetchTemperature, 1000); // Fetch every 1 seconds

    return () => clearInterval(intervalId); // Cleanup on unmount
  }, []);

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
      <Box sx={{ 
        display: 'flex',
        flexDirection: 'column',
        alignItems: 'center',
        mb: 4
      }}>
        <Typography 
          variant="h2" 
          sx={{ 
            fontFamily: 'monospace',
            fontWeight: 'bold',
            color: 'text.primary'
          }}
        >
          {selectedDateTime.format('HH:mm:ss')}
        </Typography>
        
        <Typography 
          variant="h5" 
          sx={{ 
            fontFamily: 'monospace',
            color: 'text.secondary'
          }}
        >
          {selectedDateTime.format('MMMM D, YYYY')}
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
          onClick={onBack}
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