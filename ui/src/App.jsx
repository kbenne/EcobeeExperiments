import { ThemeProvider, createTheme, responsiveFontSizes } from '@mui/material/styles';
import { LocalizationProvider } from '@mui/x-date-pickers';
import { AdapterDayjs } from '@mui/x-date-pickers/AdapterDayjs';
import CssBaseline from '@mui/material/CssBaseline';
import Container from '@mui/material/Container';
import Box from '@mui/material/Box';
import Typography from '@mui/material/Typography';
import Card from '@mui/material/Card';
import CardContent from '@mui/material/CardContent';
import { useState } from 'react';
import dayjs from 'dayjs';
import SetupView from './components/SetupView';
import SessionView from './components/SessionView';
import KPIView from './components/KPIView';

// Create the theme with responsive typography
let theme = createTheme({
  palette: {
    primary: {
      main: '#2196f3',
    },
    secondary: {
      main: '#19857b',
    },
    background: {
      default: '#f5f5f5',
    },
  },
  typography: {
    fontFamily: 'Roboto, Arial, sans-serif',
    h1: { fontSize: '3rem' },
    h2: { fontSize: '2.5rem' },
    h3: { fontSize: '2rem' },
    h4: { fontSize: '1.75rem' },
    h5: { fontSize: '1.5rem' },
    h6: { fontSize: '1.25rem' },
    body1: { fontSize: '1rem' },
    body2: { fontSize: '0.875rem' },
  },
  breakpoints: {
    values: {
      xs: 0,
      sm: 600,
      md: 1024,
      lg: 1200,
      xl: 1536,
    },
  },
});

theme = responsiveFontSizes(theme);

function App() {
  const [selectedDateTime, setSelectedDateTime] = useState(dayjs());
  const [currentView, setCurrentView] = useState('setup'); // Tracks the current view ('setup', 'session', 'kpi')
  const minDate = dayjs().startOf('year');
  const maxDate = dayjs().endOf('year');

  const handleStart = () => {
    setCurrentView('session');
  };

  const handleBack = () => {
    setCurrentView('setup');
  };

  const handleShowKPI = () => {
    setCurrentView('kpi');
  };

  const handleBackToSession = () => {
    setCurrentView('setup');;
  };

  return (
    <ThemeProvider theme={theme}>
      <LocalizationProvider dateAdapter={AdapterDayjs}>
        <CssBaseline />
          {/* Outer Box (full height) */}
          <Box
            sx={{
              display: 'flex',
              flexDirection: 'column',
              height: '100%',
            }}
          >
            {/* Header */}
            <Box
              sx={{
                flex: '0 0 auto',
                py: 1.5,
                backgroundColor: 'primary.light',
                color: '#fff',
                textAlign: 'center',
              }}
            >
              <Typography variant="h4" sx={{ fontWeight: 'bold', m: 0 }}>
                Home Energy Simulation
              </Typography>
            </Box>

            {/* Main Content */}
            <Box
              sx={{
                flex: '1 1 auto',
                p: 2,
                display: 'flex',
              }}
            >
              <Card
                sx={{
                  width: '100%',
                  borderRadius: 2,
                  boxShadow: 3,
                  display: 'flex',
                  flexDirection: 'column',
		              alignItems: 'center'
                }}
              >
	              <CardContent>
                  {/* Conditionally render views */}
                  {currentView === 'setup' && (
                    <SetupView
                      selectedDateTime={selectedDateTime}
                      setSelectedDateTime={setSelectedDateTime}
                      onStart={handleStart}
                      minDate={minDate}
                      maxDate={maxDate}
                    />
                  )}
                  {currentView === 'session' && (
                    <SessionView
                      selectedDateTime={selectedDateTime}
                      onBack={handleBack}
                      onShowKPI={handleShowKPI}
                    />
                  )}
                  {currentView === 'kpi' && <KPIView onBack={handleBackToSession} />}
                </CardContent>
              </Card>
            </Box>
          </Box>
      </LocalizationProvider>
    </ThemeProvider>
  );
}

export default App;
