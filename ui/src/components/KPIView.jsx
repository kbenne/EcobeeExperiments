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

// 1. KPI map (if needed)
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

// 2. Variable name map for friendly labels in Plotly
const variable_name_map = {
  read_TRadTemp1_y: "Radiant Temperature [K]",
  read_TRoomTemp_y: "Room Temperature [K]",
  time: "Time [s]",
  read_ACPower_y: "AC Power [W]",
  read_TAmb_y: "Outdoor Temperature [K]",
  read_FurnaceHeat_y: "Furnace Heat Energy [W]",
  read_FanPower_y: "Fan Power [W]"
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
        // response.data.results is presumably an object containing arrays:
        // {
        //    time: [...],
        //    read_TRadTemp1_y: [...],
        //    read_TRoomTemp_y: [...],
        //    ...
        // }
        setKpiData(response.data.kpi);
        setPlotData(response.data.results);

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
        {/* KPI Table */}
        {viewMode === 'kpi' && kpiData ? (
          <Box sx={{ width: '100%', maxHeight: '100%', overflow: 'auto' }}>
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
                    const displayName = kpiMap[kpiKey]?.name || kpiKey;
                    const displayUnit = kpiMap[kpiKey]?.unit || "";
                    return (
                      <TableRow key={kpiKey}>
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
          /* Plotly Chart with time as the x-axis */
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
                  {variable_name_map[key] ?? key}
                </option>
              ))}
            </select>

            {/* Make sure 'time' exists and has the same length as the selected variable */}
            <Plot
              data={[
                {
                  // Instead of using index i, directly use plotData.time
                  x: plotData.time.map((t) => new Date(2025, 0, 1).getTime() + t * 1000),
                  y: plotData[selectedVariable] || [],
                  type: 'scatter',
                  mode: 'lines+markers',
                  marker: { color: 'blue' },
                },
              ]}
              layout={{
                title: `Line Plot of ${
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
        {/* Radio Buttons */}
        <RadioGroup
          row
          value={viewMode}
          onChange={(event) => setViewMode(event.target.value)}
          sx={{ mx: 'auto' }}
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
          sx={{ borderRadius: 2, textTransform: 'none', px: 3 }}
        >
          Start a new simulation
        </Button>
      </Box>
    </Box>
  );
}

export default KPIView;
