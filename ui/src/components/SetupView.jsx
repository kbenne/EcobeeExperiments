import { DateTimePicker } from '@mui/x-date-pickers/DateTimePicker';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Paper from '@mui/material/Paper';
import Slider from '@mui/material/Slider';
import Box from '@mui/material/Box';
import AccessTimeIcon from '@mui/icons-material/AccessTime';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import { useState } from 'react';
import axios from 'axios';

function SetupView({ selectedDateTime, setSelectedDateTime, onStart, minDate, maxDate }) {
  const [stepSize, setStepSize] = useState(10); // Default step size is 10
  const [selectedOption, setSelectedOption] = useState('custom'); // 'custom' or 'preset'
  const [presetScenario, setPresetScenario] = useState(null); // 'summer' or 'winter'

  const handleStartSimulation = async () => {
    try {
      const payload =
        selectedOption === 'custom'
          ? {
              selectedDateTime: selectedDateTime.toISOString(),
              stepSize: stepSize,
            }
          : {
              presetScenario: presetScenario,
            };

      await axios.post('http://127.0.0.1:5000/api/start_simulation', payload);

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
        flexDirection: 'row',
        alignItems: 'center',
        justifyContent: 'center',
        gap: 2,
        p: 3,
      }}
    >
      {/* Custom Date and Time Option */}
      <Paper
        elevation={selectedOption === 'custom' ? 3 : 1}
        sx={{
          p: 3,
          flex: 1,
          bgcolor: selectedOption === 'custom' ? 'background.paper' : 'grey.300',
          cursor: 'pointer',
        }}
        onClick={() => setSelectedOption('custom')}
      >
        <Box
          sx={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
          }}
        >
          <AccessTimeIcon sx={{ fontSize: 60, color: selectedOption === 'custom' ? 'primary.main' : 'text.disabled', mb: 2 }} />
          <Typography
            variant="h6"
            sx={{ mb: 2, color: selectedOption === 'custom' ? 'text.primary' : 'text.disabled' }}
          >
            Select a date and time to begin
          </Typography>
          <DateTimePicker
            label="Select Date and Time"
            value={selectedDateTime}
            onChange={(newValue) => setSelectedDateTime(newValue)}
            minDateTime={minDate}
            maxDateTime={maxDate}
            sx={{ width: '100%' }}
            disabled={selectedOption !== 'custom'}
          />
          <Typography
            variant="body1"
            sx={{ mt: 2, mb: 1, color: selectedOption === 'custom' ? 'text.primary' : 'text.disabled' }}
          >
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
            disabled={selectedOption !== 'custom'}
          />
        </Box>
      </Paper>

      {/* Preset Scenario Option */}
      <Paper
        elevation={selectedOption === 'preset' ? 3 : 1}
        sx={{
          p: 3,
          flex: 1,
          bgcolor: selectedOption === 'preset' ? 'background.paper' : 'grey.300',
          cursor: 'pointer',
        }}
        onClick={() => setSelectedOption('preset')}
      >
        <Box
          sx={{
            display: 'flex',
            flexDirection: 'column',
            alignItems: 'center',
          }}
        >
          <Typography
            variant="h6"
            sx={{ mb: 2, color: selectedOption === 'preset' ? 'text.primary' : 'text.disabled' }}
          >
            Select a preset scenario
          </Typography>
          <Button
            variant={presetScenario === 'summer' ? 'contained' : 'outlined'}
            onClick={() => setPresetScenario('summer')}
            sx={{
              mb: 2,
              width: '80%',
              color: selectedOption === 'preset' ? 'primary.main' : 'text.disabled',
            }}
            disabled={selectedOption !== 'preset'}
          >
            1 Summer Day
          </Button>
          <Button
            variant={presetScenario === 'winter' ? 'contained' : 'outlined'}
            onClick={() => setPresetScenario('winter')}
            sx={{
              width: '80%',
              color: selectedOption === 'preset' ? 'primary.main' : 'text.disabled',
            }}
            disabled={selectedOption !== 'preset'}
          >
            1 Winter Day
          </Button>
        </Box>
      </Paper>

      {/* Start Button */}
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          mt: 3,
        }}
      >
        <Button
          variant="contained"
          startIcon={<PlayArrowIcon />}
          onClick={handleStartSimulation}
          size="large"
          disabled={selectedOption === 'preset' && !presetScenario}
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
    </Box>
  );
}

export default SetupView;
