import React from 'react';
import ReactDOM from 'react-dom/client';
import './index.css';
import App from './App.jsx';
import reportWebVitals from './reportWebVitals';

// Add Roboto font
const robotoFont = document.createElement('link');
robotoFont.rel = 'stylesheet';
robotoFont.href = 'https://fonts.googleapis.com/css?family=Roboto:300,400,500,700&display=swap';
document.head.appendChild(robotoFont);

const root = ReactDOM.createRoot(document.getElementById('root'));
root.render(
  <React.StrictMode>
    <App />
  </React.StrictMode>
);

// If you want to start measuring performance in your app, pass a function
// to log results (for example: reportWebVitals(console.log))
// or send to an analytics endpoint. Learn more: https://bit.ly/CRA-vitals
reportWebVitals();
