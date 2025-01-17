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

// 1. Create a KPI map, matching your backend KPI keys to a name & unit
const kpiMap = {
  cost_tot: { name: "HVAC energy cost", unit: "$/m2 or Euro/m2" },
  emis_tot: { name: "HVAC energy emissions", unit: "kgCO2e/m2" },
  ener_tot: { name: "HVAC energy total", unit: "kWh/m2" },
  pele_tot: { name: "HVAC peak electrical demand", unit: "kW/m2" },
  pgas_tot: { name: "HVAC peak gas demand", unit: "kW/m2" },
  pdih_tot: { name: "HVAC peak district heating demand", unit: "kW/m2" },
  idis_tot: { name: "Indoor air quality discomfort", unit: "ppmh/zone" },
  tdis_tot: { name: "Thermal discomfort", unit: "Kh/zone" },
  time_rat: { name: "Computational time ratio", unit: "s/ss" },
};

function KPIView({ onBack }) {
  const [kpiData, setKpiData] = useState(null);
  const [viewMode, setViewMode] = useState('kpi'); 
  const [plotData, setPlotData] = useState(null);
  const [selectedVariable, setSelectedVariable] = useState(null);

  useEffect(() => {
    const fetchKpisAndResults = async () => {
      try {
        const response = await axios.get('http://127.0.0.1:5000/kpi');
        setKpiData(response.data.kpi);       // e.g. {'tdis_tot': 0.0083, 'idis_tot': 0, etc.}
        setPlotData(response.data.results);  // simulation results

        // Set the initial selected variable for Plotly
        if (response.data.results) {
          setSelectedVariable(Object.keys(response.data.results)[0]);
        }
      } catch (error) {
        console.error('Error fetching KPI and results data:', error);
      }
    };

    fetchKpisAndResults();
  }, []);

  return (
    <Box
      sx={{
        width: '100%',
        height: '100%',
        display: 'flex',
        flexDirection: 'column',
        justifyContent: 'space-between',
        boxSizing: 'border-box',
        overflow: 'hidden',
        maxHeight: '100%',
        p: 2,
        gap: 2,
      }}
    >
      {/* Main Content */}
      <Box
        sx={{
          flex: 1,
          display: 'flex',
          flexDirection: 'column',
          alignItems: 'center',
          justifyContent: 'center',
          overflow: 'auto',
          maxHeight: 'calc(100% - 80px)',
        }}
      >
        {viewMode === 'kpi' && kpiData ? (
          <Box sx={{ width: '100%', maxHeight: '100%', overflow: 'auto' }}>
            <TableContainer component={Paper}>
              <Table>
                <TableHead>
                  <TableRow>
                    <TableCell>KPI</TableCell>
                    <TableCell align="right">Value</TableCell>
                    {/* 2. Add another column for Unit */}
                    <TableCell align="right">Unit</TableCell>
                  </TableRow>
                </TableHead>
                <TableBody>
                  {Object.entries(kpiData).map(([kpiKey, value]) => {
                    // 3. Extract new name and unit from kpiMap
                    const displayName = kpiMap[kpiKey]?.name || kpiKey;
                    const displayUnit = kpiMap[kpiKey]?.unit || "";

                    return (
                      <TableRow key={kpiKey}>
                        {/* Show the mapped KPI name, or fallback to the raw kpiKey */}
                        <TableCell component="th" scope="row">
                          {displayName}
                        </TableCell>
                        <TableCell align="right">{value}</TableCell>
                        <TableCell align="right">{displayUnit}</TableCell>
                      </TableRow>
                    );
                  })}
                </TableBody>
              </Table>
            </TableContainer>
          </Box>
        ) : viewMode === 'plotly' && plotData && selectedVariable ? (
          <Box sx={{ width: '100%', overflow: 'auto' }}>
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
              {Object.keys(plotData).map((key) => (
                <option key={key} value={key}>
                  {key}
                </option>
              ))}
            </select>

            {/* Plotly Chart */}
            <Plot
              data={[
                {
                  x: Array.from({ length: plotData[selectedVariable].length }, (_, i) => i + 1),
                  y: plotData[selectedVariable],
                  type: 'scatter',
                  mode: 'lines+markers',
                  marker: { color: 'blue' },
                },
              ]}
              layout={{
                title: `Line Plot of ${selectedVariable}`,
                xaxis: { title: 'Step' },
                yaxis: { title: selectedVariable },
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
        {/* Radio Buttons */}
        <RadioGroup
          row
          value={viewMode}
          onChange={(event) => setViewMode(event.target.value)}
          sx={{
            mx: 'auto',
          }}
        >
          <FormControlLabel value="kpi" control={<Radio />} label="KPI Table" />
          <FormControlLabel value="plotly" control={<Radio />} label="Line Plot" />
        </RadioGroup>

        {/* Back Button */}
        <Button
          variant="outlined"
          startIcon={<ArrowBackIcon />}
          onClick={onBack}
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
