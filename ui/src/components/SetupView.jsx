import { DateTimePicker } from '@mui/x-date-pickers/DateTimePicker';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Paper from '@mui/material/Paper';
import AccessTimeIcon from '@mui/icons-material/AccessTime';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';

function SetupView({ selectedDateTime, setSelectedDateTime, onStart, minDate, maxDate }) {
  return (
    <>
      <AccessTimeIcon sx={{ fontSize: 60, color: 'primary.main', mb: 2 }} />
      <Typography variant="h6" sx={{ mb: 3, color: 'text.secondary' }}>
        Select a date and time to begin
      </Typography>
      <Paper elevation={3} sx={{ p: 3, width: '100%', bgcolor: 'background.paper' }}>
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
        onClick={onStart}
        size="large"
        sx={{ 
          mt: 2,
          px: 4,
          py: 1.5,
          borderRadius: 2,
          textTransform: 'none',
          fontSize: '1.1rem'
        }}
      >
        Start Session
      </Button>
    </>
  );
}

export default SetupView; 