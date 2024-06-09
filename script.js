async function fetchData() {
    try {
        const response = await fetch('http://192.168.43.36'); // Replace with your NodeMCU IP address
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        const data = await response.json();
        
        // Update temperature-related elements
        document.getElementById('temperature').innerText = `${data.temperature}°C`;

        let temperatureAlert = '';
        let waterAlert = '';

        if (data.alert) {
            if (data.alert.includes('TEMPERATURE')) {
                temperatureAlert = data.alert.match(/TEMPERATURE IS \w+/)[0];
            }
            if (data.alert.includes('WATER LEVEL')) {
                waterAlert = 'Water level is low';
            }
        }

        document.getElementById('alert').innerText = temperatureAlert;
        document.getElementById('led').innerText = data.led ? data.led : '';
        
        // Update water level-related elements
        document.getElementById('distance').innerText = `Distance: ${data.distance} cm`;
        document.getElementById('message').innerText = waterAlert ? waterAlert : 'Water level indicating';
    } catch (error) {
        console.error('Error fetching data:', error);
        document.getElementById('temperature').innerText = 'Error';
        document.getElementById('alert').innerText = 'Error fetching temperature data';
        document.getElementById('led').innerText = '';
        document.getElementById('distance').innerText = 'Error loading data';
        document.getElementById('message').innerText = 'Error fetching water level data';
    }
}

setInterval(fetchData, 1000); // Update every 1 second
fetchData(); // Initial fetch
