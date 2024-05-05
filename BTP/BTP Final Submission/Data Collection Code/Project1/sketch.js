const serviceUuid = "19b10010-e8f2-537e-4f6c-d104768a1214";
const  characteristicsUUID = {
  spo2:"19b10011-e8f2-537e-4f6c-d104768a1214",
  heart:"19b10012-e8f2-537e-4f6c-d104768a1214",
  temperature: "19b10013-e8f2-537e-4f6c-d104768a1214"
}

let spo2Characteristic;
let heartRateCharacteristic;
let temperatureCharacteristic;
let myBLE;
let ecgCharacteristic;

let ecgData = [];
let cnt = 0;

let spo2ValueElement;
let heartRateValueElement;
let temperatureValueElement;

let spo2Value  = 0;
let heartValue = 0;
let temperatureValue = 0;

function setup() {
  spo2ValueElement = document.getElementById("spo2Value");
  heartRateValueElement = document.getElementById("heartRateValue");
  temperatureValueElement = document.getElementById("temperatureValue");

  createCanvas(400, 400);
    // Create a p5ble  class
  myBLE = new p5ble();

  createCanvas(600, 400);
  background("#FFF");

  // Create a 'Connect and Start Notifications' button
  const connectButton  = createButton('Connect and Start Notifications')
  connectButton.mousePressed(connectAndStartNotify);

}

function connectAndStartNotify() {
  // Connect to a  device by passing the service UUID
  myBLE.connect(serviceUuid, gotCharacteristics);
}

//  A function that will be called once got characteristics
function gotCharacteristics(error,  characteristics) {
  if (error) console.log('error: ', error);
  console.log(characteristics[1].uuid);
  
   for (let i = 0; i < characteristics.length; i++) {
        if (characteristics[i].uuid == characteristicsUUID.spo2) { // SPO2 characteristic UUID
          spo2Characteristic = characteristics[i];
          myBLE.startNotifications(spo2Characteristic, handleSPO2);
          
        } else if (characteristics[i].uuid == characteristicsUUID.heart) { // Heart Rate characteristic UUID
          heartRateCharacteristic = characteristics[i];
          ecgCharacteristic = characteristics[i];
          myBLE.startNotifications(heartRateCharacteristic, handleHeartRate);
          myBLE.startNotifications(ecgCharacteristic, handleECG);
          
        } else if (characteristics[i].uuid == characteristicsUUID.temperature) { // Temperature characteristic UUID
          temperatureCharacteristic = characteristics[i];
          myBLE.startNotifications(temperatureCharacteristic, handleTemperature);
          
        }
    }
 
  // Start notifications on the first characteristic by passing the characteristic
  // And a callback function to handle notifications
  
}


// A function  that will be called once got characteristics
function handleSPO2(data) {
  console.log('Spo2: ', data);
  spo2Value = Number(data);
  spo2ValueElement.innerText = spo2Value + "%";
}

function handleHeartRate(data) {
  console.log('Heart Rate: ', data);
  heartValue = Number(data);
  heartRateValueElement.innerText = heartValue + " bpm";
}

function handleTemperature(data) {
  console.log('Temperature: ', data);
  // temperatureValue = Number(data);
  temperatureValue  = float(data);
  temperatureValueElement.innerText = temperatureValue.toFixed(2) + " °C";
}

function handleECG(data) {
  ecgData.push(Number(data));

  // Update the ECG chart
  updateECGChart();

  // Adjust the data array to limit the number of points displayed
  if (ecgData.length > 200) {
    ecgData.shift();
  }
}

let ecgChart; // Declare the chart variable outside the function

function updateECGChart() {
  const ctx = document.getElementById('ecgChart').getContext('2d');

  // Destroy the existing chart if it exists
  if (ecgChart) {
    ecgChart.destroy();
  }

  ecgChart = new Chart(ctx, {
    type: 'line',
    data: {
      labels: Array.from({ length: ecgData.length }, (_, i) => i),
      datasets: [{
        label: 'ECG',
        borderColor: 'rgb(75, 192, 192)',
        data: ecgData,
        fill: false
      }]
    },
    options: {
      scales: {
        x: {
          type: 'linear',
          position: 'bottom'
        }
      }
    }
  });
}