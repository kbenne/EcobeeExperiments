import { useState, useEffect } from 'react';
import Plot from 'react-plotly.js';
import axios from 'axios';

// Material UI components
import Typography from '@mui/material/Typography';
import Button from '@mui/material/Button';
import Box from '@mui/material/Box';
import Radio from '@mui/material/Radio';
import RadioGroup from '@mui/material/RadioGroup';
import FormControlLabel from '@mui/material/FormControlLabel';
import ArrowBackIcon from '@mui/icons-material/ArrowBack';
import Table from '@mui/material/Table';
import TableBody from '@mui/material/TableBody';
import TableCell from '@mui/material/TableCell';
import TableContainer from '@mui/material/TableContainer';
import TableHead from '@mui/material/TableHead';
import TableRow from '@mui/material/TableRow';
import Paper from '@mui/material/Paper';

const kpiMap = {
  cost_tot: { name: "HVAC energy cost", unit: "$/m2 or Euro/m2" },
  emis_tot: { name: "HVAC energy emissions", unit: "kgCO2e/m2" },
  ener_tot: { name: "HVAC energy total", unit: "kWh/m2" },
  pele_tot: { name: "HVAC peak electrical demand", unit: "kW/m2" },
  pgas_tot: { name: "HVAC peak gas demand", unit: "kW/m2" },
  //pdih_tot: { name: "HVAC peak district heating demand", unit: "kW/m2" },
  //idis_tot: { name: "Indoor air quality discomfort", unit: "ppmh/zone" },
  tdis_tot: { name: "Thermal discomfort", unit: "Kh/zone" },
  //time_rat: { name: "Computational time ratio", unit: "s/ss" },
};

const variable_name_map = {
  read_TRadTemp1_y: "Radiant Temperature [K]",
  read_TRoomTemp_y: "Room Temperature [K]",
  read_ACPower_y: "AC Power [kW]",
  read_TAmb_y: "Outdoor Temperature [K]",
  read_FurnaceHeat_y: "Furnace Gas Power [kW]",
  read_FanPower_y: "Fan Power [kW]",
};

function toScientificWithSuperscript(num, sigFigs = 3) {
  if (num === 0) return '0';
  const exponent = Math.floor(Math.log10(Math.abs(num)));
  const mantissa = (num / Math.pow(10, exponent)).toPrecision(sigFigs);

  // Convert the exponent to a superscript string
  const superscriptMap = {
    '-': '⁻',
    '0': '⁰',
    '1': '¹',
    '2': '²',
    '3': '³',
    '4': '⁴',
    '5': '⁵',
    '6': '⁶',
    '7': '⁷',
    '8': '⁸',
    '9': '⁹',
  };

  const superscriptExponent = String(exponent)
    .split('')
    .map((char) => superscriptMap[char])
    .join('');

  return `${mantissa} × 10${superscriptExponent}`;
}

function KPIView({ onBack }) {
  const [kpiData, setKpiData] = useState(null);
  const [viewMode, setViewMode] = useState('kpi');
  const [plotData, setPlotData] = useState(null);
  const [selectedVariable, setSelectedVariable] = useState(null);

  // Fetch KPI data and results on mount
  useEffect(() => {
    const fetchKpisAndResults = async () => {
      try {
        const response = await axios.get('http://127.0.0.1:5000/kpi');
        setKpiData(response.data.kpi);
        setPlotData(response.data.results);

        // Pick the first available non-time key by default
        if (response.data.results) {
          const nonTimeKeys = Object.keys(response.data.results).filter(
            (key) => key !== 'time'
          );
          if (nonTimeKeys.length) {
            setSelectedVariable(nonTimeKeys[0]);
          }
        }
      } catch (error) {
        console.error('Error fetching KPI and results data:', error);
      }
    };

    fetchKpisAndResults();
  }, []);

  // Stop simulation, then navigate back to SetupView
  const handleStopAndBack = async () => {
    try {
      await axios.post('http://127.0.0.1:5000/api/stop_simulation');
    } catch (error) {
      console.error('Error stopping simulation:', error);
      alert('Failed to stop simulation!');
    }
    onBack();
  };

  return (
    <Box
      sx={{
        minWidth: '1000px',
        minHeight:'460px',
        display: 'flex',
        flexDirection: 'column',
        p: 2,
        gap: 1,
      }}
    >
      {/* Main Content */}
      <Box
        sx={{
          flex: 1,
          display: 'flex',
          flexDirection: 'column',
        }}
      >
        {/* KPI Table */}
        {viewMode === 'kpi' && kpiData ? (
          <Box sx={{ width: '100%'}}>
            <TableContainer component={Paper}>
              <Table>
                <TableHead>
                  <TableRow>
                    <TableCell>KPI</TableCell>
                    <TableCell align="right">Value</TableCell>
                    <TableCell align="right">Unit</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {Object.entries(kpiData).map(([kpiKey, value]) => {
                    // Omit KPIs that are not mapped to display names
                    if (! kpiMap[kpiKey]) {
                      return;
                    }
                    const displayName = kpiMap[kpiKey]?.name || kpiKey;
                    const displayUnit = kpiMap[kpiKey]?.unit || "";

                    // If "value" is a number, format in scientific notation w/ 3 sig figs
                    let displayValue = value;
                    if (typeof value === 'number') {
                      displayValue = toScientificWithSuperscript(value, 3);
                    }

                    return (
                      <TableRow key={kpiKey}>
                        <TableCell component="th" scope="row">
                          {displayName}
                        </TableCell>
                        <TableCell align="right">{displayValue}</TableCell>
                        <TableCell align="right">{displayUnit}</TableCell>
                      </TableRow>
                    );
                  })}
                </TableBody>
              </Table>
            </TableContainer>
          </Box>
        ) : viewMode === 'plotly' && plotData && selectedVariable ? (
          // Timeseries Plot
          <Box sx={{ width: '100%'}}>
            {/* Dropdown for selecting variable */}
            <select
              value={selectedVariable}
              onChange={(e) => setSelectedVariable(e.target.value)}
              style={{
                marginBottom: '2px',
                padding: '2px',
                fontSize: '13px',
                width: '200px',
              }}
            >
              {Object.keys(plotData)
                .filter((key) => key !== 'time')
                .map((key) => (
                  <option key={key} value={key}>
                    {variable_name_map[key] ?? key}
                  </option>
                ))}
            </select>

            <Plot
              data={[
                {
                  // Convert numeric time to a date-based axis
                  x: plotData.time.map(
                    (t) => new Date(2025, 0, 1).getTime() + t * 1000
                  ),
                  y: plotData[selectedVariable] || [],
                  type: 'scatter',
                  mode: 'lines',
                  marker: { color: 'blue' },
                },
              ]}
              layout={{
                title: `Timeseries: ${
                  variable_name_map[selectedVariable] ?? selectedVariable
                }`,
                xaxis: { title: 'Date/Time', type: 'date' },
                yaxis: {
                  title: variable_name_map[selectedVariable] ?? selectedVariable,
                },
                responsive: true,
                autosize: true,
              }}
              style={{
                width: '100%',
                maxHeight: '100%',
                height: '320px',
                overflow: 'auto',
              }}
              useResizeHandler
            />
          </Box>
        ) : (
          <Typography variant="h6" sx={{ color: 'text.secondary', flex: 1 }}>
            Loading data...
          </Typography>
        )}
      </Box>

      {/* Footer Section */}
      <Box
        sx={{
          display: 'flex',
          justifyContent: 'space-between',
          alignItems: 'center',
        }}
      >
        {/* View Mode Switch */}
        <RadioGroup
          row
          value={viewMode}
          onChange={(event) => setViewMode(event.target.value)}
          sx={{ mx: 'auto' }}
        >
          <FormControlLabel value="kpi" control={<Radio />} label="KPI Table" />
          <FormControlLabel value="plotly" control={<Radio />} label="Timeseries Results" />
        </RadioGroup>

        <Button
          variant="outlined"
          startIcon={<ArrowBackIcon />}
          onClick={handleStopAndBack}
          size="large"
          sx={{
            borderRadius: 2,
            textTransform: 'none',
            px: 3,
          }}
        >
          Start a new simulation
        </Button>
      </Box>
    </Box>
  );
}

export default KPIView;
