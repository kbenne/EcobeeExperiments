import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import ThermostatIcon from '@mui/icons-material/Thermostat';
import Paper from '@mui/material/Paper';

function Temperature({ title, value }) {
  // Determine what to display
  const displayValue = typeof value === 'number' ? value.toFixed(1) : 'N/A';

  return (
      <Box sx={{ display: 'flex', alignItems: 'center', gap: 2 }}>
        <ThermostatIcon sx={{ fontSize: 40, color: 'primary.main' }} />
        <Box>
          <Typography
            variant="body1"
            color="text.secondary"
          >
            Temperature
          </Typography>
          <Box sx={{ minWidth: '150px' }}>
            <Typography
              variant="h4"
              sx={{
                fontWeight: 'medium',
                fontFamily: 'monospace',
                width: '100%'
              }}
            >
              {displayValue !== 'N/A' ? `${displayValue}°F` : displayValue}
            </Typography>
          </Box>
        </Box>
      </Box>
  );
}

export default Temperature;
