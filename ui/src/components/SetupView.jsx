import { DateTimePicker } from '@mui/x-date-pickers/DateTimePicker';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Paper from '@mui/material/Paper';
import AccessTimeIcon from '@mui/icons-material/AccessTime';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import axios from 'axios';

function SetupView({ 
  selectedDateTime, 
  setSelectedDateTime, 
  onStart, 
  minDate, 
  maxDate 
}) {
  // Handle the button click
  const handleStartSimulation = async () => {
    try {
      await axios.post('http://127.0.0.1:5000/api/start_simulation', {
        selectedDateTime: selectedDateTime.toISOString(),
      });
      // If successful, move to the next view
      onStart();
    } catch (error) {
      console.error('Error starting simulation:', error);
      alert('Failed to start simulation!');
    }
  };

  return (
    <>
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
          mb: 2,
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
    </>
  );
}

export default SetupView;
