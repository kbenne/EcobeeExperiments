import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import Paper from '@mui/material/Paper';
import AcUnitIcon from '@mui/icons-material/AcUnit';
import LocalFireDepartmentIcon from '@mui/icons-material/LocalFireDepartment';
import AirIcon from '@mui/icons-material/Air';
import PowerSettingsNewIcon from '@mui/icons-material/PowerSettingsNew';

function HvacStatus({ status }) {
  const getStatusInfo = () => {
    switch (status.toLowerCase()) {
      case 'heat':
        return {
          icon: <LocalFireDepartmentIcon sx={{ fontSize: 40, color: '#ff4d4d' }} />,
          color: '#ff4d4d'
        };
      case 'cool':
        return {
          icon: <AcUnitIcon sx={{ fontSize: 40, color: '#4dabf5' }} />,
          color: '#4dabf5'
        };
      case 'fan':
        return {
          icon: <AirIcon sx={{ fontSize: 40, color: '#69b076' }} />,
          color: '#69b076'
        };
      default: // 'off'
        return {
          icon: <PowerSettingsNewIcon sx={{ fontSize: 40, color: '#757575' }} />,
          color: '#757575'
        };
    }
  };

  const statusInfo = getStatusInfo();

  return (
    <Paper 
      elevation={2}
      sx={{
        p: 3,
        borderRadius: 2,
        width: '100%',
        bgcolor: 'background.paper'
      }}
    >
      <Box sx={{
        display: 'flex',
        alignItems: 'center',
        gap: 2
      }}>
        {statusInfo.icon}
        <Box>
          <Typography variant="body1" color="text.secondary">
            HVAC Status
          </Typography>
          <Typography 
            variant="h4" 
            sx={{ 
              fontWeight: 'medium',
              color: statusInfo.color
            }}
          >
            {status}
          </Typography>
        </Box>
      </Box>
    </Paper>
  );
}

export default HvacStatus; 