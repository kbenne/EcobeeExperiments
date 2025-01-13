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
  const [isStarted, setIsStarted] = useState(false);
  const minDate = dayjs().startOf('year');
  const maxDate = dayjs().endOf('year');

  const handleStart = () => {
    setIsStarted(true);
  };

  const handleBack = () => {
    setIsStarted(false);
  };

  return (
    <ThemeProvider theme={theme}>
      <LocalizationProvider dateAdapter={AdapterDayjs}>
        <CssBaseline />
        <Container maxWidth={false} sx={{ width: '1024px', maxWidth: '100%' }}>
          <Box
            sx={{
              minHeight: '100vh',
              maxHeight: '600px',
              py: 4,
              display: 'flex',
              flexDirection: 'column',
              alignItems: 'center',
              gap: 4,
              overflow: 'auto',
            }}
          >
            <Typography
              variant="h3"
              component="h1"
              sx={{
                color: 'primary.main',
                fontWeight: 'bold',
                textAlign: 'center',
                mb: 2,
              }}
            >
              Virtual Home Environment
            </Typography>

            <Card
              sx={{
                width: '100%',
                maxWidth: 1024,
                boxShadow: 3,
                borderRadius: 2,
              }}
            >
              <CardContent
                sx={{
                  display: 'flex',
                  flexDirection: 'column',
                  alignItems: 'center',
                  gap: 4,
                  p: 4,
                }}
              >
                {!isStarted ? (
                  <SetupView
                    selectedDateTime={selectedDateTime}
                    setSelectedDateTime={setSelectedDateTime}
                    onStart={handleStart}
                    minDate={minDate}
                    maxDate={maxDate}
                  />
                ) : (
                  <SessionView
                    selectedDateTime={selectedDateTime}
                    onBack={handleBack}
                  />
                )}
              </CardContent>
            </Card>
          </Box>
        </Container>
      </LocalizationProvider>
    </ThemeProvider>
  );
}

export default App;
