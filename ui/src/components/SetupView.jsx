import { DateTimePicker } from '@mui/x-date-pickers/DateTimePicker';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Paper from '@mui/material/Paper';
import Slider from '@mui/material/Slider';
import Box from '@mui/material/Box';
import AccessTimeIcon from '@mui/icons-material/AccessTime';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import axios from 'axios';
import { useState } from 'react';

function SetupView({ selectedDateTime, setSelectedDateTime, onStart, minDate, maxDate }) {
  const [stepSize, setStepSize] = useState(10); // Default step size is 10

  const handleStartSimulation = async () => {
    try {
      await axios.post('http://127.0.0.1:5000/api/start_simulation', {
        selectedDateTime: selectedDateTime.toISOString(),
        stepSize: stepSize, // Include step size in the payload
      });
      // If successful, move to the next view
      onStart();
    } catch (error) {
      console.error('Error starting simulation:', error);
      alert('Failed to start simulation!');
    }
  };

  const handleStepSizeChange = (event, newValue) => {
    setStepSize(newValue);
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
      <AccessTimeIcon sx={{ fontSize: 60, color: 'primary.main', mb: 2 }} />
      <Typography variant="h6" sx={{ mb: 2, color: 'text.secondary', textAlign: 'center' }}>
        Select a date and time to begin
      </Typography>
      <Paper
        elevation={3}
        sx={{
          p: 3,
          width: '100%',
          bgcolor: 'background.paper',
          mb: 3,
        }}
      >
        <DateTimePicker
          label="Select Date and Time"
          value={selectedDateTime}
          onChange={(newValue) => setSelectedDateTime(newValue)}
          minDateTime={minDate}
          maxDateTime={maxDate}
          sx={{ width: '100%' }}
        />
      </Paper>

      <Typography variant="body1" sx={{ mb: 1, color: 'text.secondary' }}>
        Warp Speed: {stepSize}
      </Typography>
      <Slider
        value={stepSize}
        onChange={handleStepSizeChange}
        min={1}
        max={10}
        step={1}
        valueLabelDisplay="auto"
        sx={{ width: '80%', mb: 3 }}
      />

      <Button
        variant="contained"
        startIcon={<PlayArrowIcon />}
        onClick={handleStartSimulation}
        size="large"
        sx={{
          px: 4,
          py: 1.5,
          borderRadius: 2,
          textTransform: 'none',
          fontSize: '1.1rem',
        }}
      >
        Start Session
      </Button>
    </Box>
  );
}

export default SetupView;
