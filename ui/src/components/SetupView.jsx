import { DateTimePicker } from '@mui/x-date-pickers/DateTimePicker';
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Paper from '@mui/material/Paper';
import Slider from '@mui/material/Slider';
import Box from '@mui/material/Box';
import AccessTimeIcon from '@mui/icons-material/AccessTime';
import PlayArrowIcon from '@mui/icons-material/PlayArrow';
import Backdrop from '@mui/material/Backdrop';
import CircularProgress from '@mui/material/CircularProgress';
import { useState } from 'react';
import axios from 'axios';

function SetupView({ selectedDateTime, setSelectedDateTime, onStart, minDate, maxDate }) {
  const [timeMultiplier, setTimeMultiplier] = useState(15); // Default step size is 10
  const [selectedOption, setSelectedOption] = useState('custom'); // 'custom' or 'preset'
  const [presetScenario, setPresetScenario] = useState(null); // 'summer' or 'winter'
  const [loading, setLoading] = useState(false); // Loading state

  const checkSimulationReady = async () => {
    const response = await axios.get('http://127.0.0.1:5000/sim_ready');
    if (response.data.simulationReady) {
      setLoading(false);
      // onStart will move to the SessionView
      onStart();
    } else {
      setTimeout(checkSimulationReady, 1000); // Retry after 1 second
    }
  };

  const handleStartSimulation = async () => {
    try {
      setLoading(true);

      // We don't expect a running simultion here, but just in case...
      await axios.post('http://127.0.0.1:5000/api/stop_simulation');

      const simulation_options =
        selectedOption === 'custom'
          ? {
              selectedDateTime: selectedDateTime.toISOString(),
              timeMultiplier: timeMultiplier,
            }
          : {
              presetScenario: presetScenario,
            };

      await axios.post('http://127.0.0.1:5000/api/start_simulation', simulation_options);

      checkSimulationReady();
    } catch (error) {
      console.error('Error starting simulation:', error);
      alert('Failed to start simulation!');
      setLoading(false);
    }
  };

  const handleTimeMultiplierChange = (event, newValue) => {
    setTimeMultiplier(newValue);
  };

  return (
    <Box
      sx={{
        display: 'flex',
        flexDirection: 'column',
        minWidth: '1000px',
        minHeight:'460px',
        gap: 3,
        p: 3,
      }}
    >
      {/* Loading Spinner */}
      <Backdrop open={loading} sx={{ color: '#fff', zIndex: (theme) => theme.zIndex.drawer + 1 }}>
        <CircularProgress color="inherit" />
      </Backdrop>

      {/* Instructional text */}
      <Box
        sx={{
          textAlign: 'center',
        }}
      >
        <Typography variant="subtitle1" sx={{ color: 'text.secondary' }}>
          Please select either a custom date/time or a preset scenario.
        </Typography>
      </Box>

      {/* Main row with the two options */}
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'row',
          gap: 2,
          height: '250px',
        }}
      >
        {/* Custom Date and Time Option */}
        <Paper
          elevation={selectedOption === 'custom' ? 3 : 1}
          sx={{
            p: 3,
            flex: 2, // Set width ratio 2
            height: '100%', // Fill the parent height
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
              justifyContent: 'space-between', // Distribute content evenly
            }}
          >
            <AccessTimeIcon
              sx={{
                fontSize: 60,
                color: selectedOption === 'custom' ? 'primary.main' : 'text.disabled',
              }}
            />
            <Typography
              variant="h6"
              sx={{
                color: selectedOption === 'custom' ? 'text.primary' : 'text.disabled',
              }}
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
              slotProps={{
                // Prevent the popup from going off screen
                popper: {
                  modifiers: [
                    {
                      name: 'preventOverflow',
                      options: {
                        boundary: 'viewport',
                      },
                    },
                    {
                      name: 'offset',
                      options: {
                        offset: [0, -200], // Adjust popup offset
                      },
                    },
                  ],
                },
              }}
            />
            <Typography
              variant="body1"
              sx={{
                color: selectedOption === 'custom' ? 'text.primary' : 'text.disabled',
              }}
            >
              Time Warp Factor: {timeMultiplier}
            </Typography>
            <Slider
              value={timeMultiplier}
              onChange={handleTimeMultiplierChange}
              min={1}
              max={15}
              step={1}
              valueLabelDisplay="auto"
              sx={{ width: '80%' }}
              disabled={selectedOption !== 'custom'}
            />
          </Box>
        </Paper>

        {/* Preset Scenario Option */}
        <Paper
          elevation={selectedOption === 'preset' ? 3 : 1}
          sx={{
            p: 3,
            flex: 1, // Set width ratio 1
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
	            gap: 2,
            }}
          >
            <Typography
              variant="h6"
              sx={{
                color: selectedOption === 'preset' ? 'text.primary' : 'text.disabled',
              }}
            >
              Select a preset scenario
            </Typography>
            <Button
              variant={presetScenario === 'summer' ? 'contained' : 'outlined'}
              onClick={() => setPresetScenario('summer')}
              sx={{
                width: '80%',
              }}
              disabled={selectedOption !== 'preset'}
            >
              Summer Day
            </Button>
            <Button
              variant={presetScenario === 'winter' ? 'contained' : 'outlined'}
              onClick={() => setPresetScenario('winter')}
              sx={{
                width: '80%',
              }}
              disabled={selectedOption !== 'preset'}
            >
              Winter Day
            </Button>
          </Box>
        </Paper>
      </Box>

      {/* Start Button */}
      <Box
        sx={{
          display: 'flex',
          flexDirection: 'row',
          justifyContent: 'center',
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
