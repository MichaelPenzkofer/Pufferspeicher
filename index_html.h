const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <title>Sensorreihenfolge konfigurieren</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root {
      --bs-blue: #0d6efd;
      --bs-primary: #0d6efd;
      --bs-body-bg: #f8f9fa;
    }
    .container { width: 100%; max-width: 1200px; margin: 0 auto; padding: 1rem; }
    .card { background: #fff; border-radius: 0.5rem; box-shadow: 0 0.125rem 0.25rem rgba(0, 0, 0, 0.075); }
    .card-header { background: var(--bs-primary); color: white; padding: 1rem; border-radius: 0.5rem 0.5rem 0 0; }
    .card-body { padding: 1rem; }
    .alert { padding: 1rem; border-radius: 0.375rem; margin-bottom: 1rem; }
    .alert-info { background-color: #cff4fc; border: 1px solid #b6effb; }
    .form-label { display: block; margin-bottom: 0.5rem; }
    .form-control { display: block; width: 100%; padding: 0.375rem 0.75rem; font-size: 1rem; border: 1px solid #dee2e6; border-radius: 0.375rem; margin-bottom: 1rem; }
    .btn { display: inline-block; padding: 0.375rem 0.75rem; border: none; border-radius: 0.375rem; cursor: pointer; text-align: center; text-decoration: none; }
    .btn-primary { background: var(--bs-primary); color: white; }
    .btn-lg { padding: 0.5rem 1rem; font-size: 1.25rem; }
    .d-grid { display: grid; }
    .mb-0 { margin-bottom: 0; }
    .mb-3 { margin-bottom: 1rem; }
    .mb-5 { margin-bottom: 3rem; }
    .mt-3 { margin-top: 1rem; }
    @media (min-width: 768px) {
      .col-md-8 { width: 66.666667%; margin: 0 auto; }
    }
  </style>
  <style>
    body { 
      background-color: #f8f9fa; 
      padding: 2rem; 
      font-family: Arial, Helvetica, sans-serif;
    }
    .sensor-container {
      min-height: 50px;
    }
    .sensor-item {
      padding: 1rem;
      margin-bottom: 0.5rem;
      background-color: white;
      border: 1px solid #ced4da;
      border-radius: 0.5rem;
      cursor: grab;
      user-select: none;
      transition: background-color 0.15s ease;
      touch-action: none;
      text-align: center;
    }
    .sensor-item:last-child {
      margin-bottom: 0;
    }
    .sensor-temp {
      font-size: 2em;
      font-weight: bold;
      margin-bottom: 0.25rem;
    }
    .sensor-address {
      font-size: 1em;
      color: #6c757d;
    }
    .sensor-item-ghost {
      opacity: 0.5;
      background: #f8f9fa;
    }
    .sensor-item-chosen {
      background-color: #e9ecef;
      box-shadow: 0 0 0 0.2rem rgba(13, 110, 253, 0.25);
    }
    .sensor-item-drag {
      opacity: 1;
    }
    .sortable-fallback {
      position: absolute;
      pointer-events: none;
      z-index: 1000;
      opacity: 0.8;
      background: white;
      box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
    }
  </style>
</head>
<body>
  <div class="container">
    <div id="wifiConfig" class="mb-5 card" style="display: none;">
      <div class="card-header">
        <h4 class="mb-0">WLAN-Konfiguration</h4>
      </div>
      <div class="card-body">
        <div class="alert alert-info" role="alert">
          Das Gerät befindet sich im Einrichtungsmodus. Bitte geben Sie Ihre WLAN-Zugangsdaten ein.
        </div>
        <div class="col-md-8">
          <form class="needs-validation" novalidate>
            <div class="mb-3">
              <label for="ssid" class="form-label">WLAN-Name (SSID)</label>
              <input type="text" class="form-control" id="ssid" name="ssid" required>
            </div>
            <div class="mb-3">
              <label for="password" class="form-label">WLAN-Passwort</label>
              <input type="password" class="form-control" id="password" name="password" required>
            </div>
            <div class="d-grid">
              <button id="saveWifiBtn" class="btn btn-primary btn-lg">WLAN speichern</button>
            </div>
            <div id="wifiStatusMsg" class="mt-3"></div>
          </form>
        </div>
      </div>
    </div>

    <div id="sensorConfig" style="display: none;">
      <h2>Sensorkonfiguration</h2>
      <p>Ziehen Sie die Sensoren in die gewünschte Reihenfolge von oben (Fühler oben im Puffer) nach unten.</p>
      <div id="statusMsg" class="mb-3"></div>
      <div id="sensorList"></div>
      <div class="d-flex gap-2 align-items-center mt-3">
        <button id="saveBtn" class="btn btn-primary">Speichern</button>
        <button id="resetBtn" class="btn btn-danger">WLAN zurücksetzen</button>
        <button id="resetOrderBtn" class="btn btn-warning">Fühlerreihenfolge zurücksetzen</button>
      </div>
    </div>
  </div>

  <script src="https://cdn.jsdelivr.net/npm/sortablejs@1.15.0/Sortable.min.js"></script>
  <script>
    let sensorData = [];
    let updateInterval;
    let initialOrderLoaded = false;
    
    async function checkMode() {
      try {
        const res = await fetch('/get_mode');
        if (!res.ok) throw new Error('Network response was not ok');
        const data = await res.json();
        
        document.getElementById('wifiConfig').style.display = data.isAP ? 'block' : 'none';
        document.getElementById('sensorConfig').style.display = data.isAP ? 'none' : 'block';
        
        if (!data.isAP && !updateInterval) {
          if (!initialOrderLoaded) {
            await loadInitialOrder();
          }
          startAutoUpdate();
        } else if (data.isAP && updateInterval) {
          clearInterval(updateInterval);
          updateInterval = null;
        }
      } catch (error) {
        console.error('Error checking mode:', error);
      }
    }

    async function loadInitialOrder() {
      showStatus('Lade gespeicherte Reihenfolge...', 'info');
      try {
        const res = await fetch('/get_initial_order');
        if (!res.ok) throw new Error('Network response was not ok');
        sensorData = await res.json();
        initialOrderLoaded = true;
        renderSensorList();
        showStatus('Reihenfolge geladen', 'info');
      } catch (error) {
        console.error('Error loading initial order:', error);
        showStatus('Fehler beim Laden der Reihenfolge!', 'error');
      }
    }

    async function fetchSensors() {
      if (!initialOrderLoaded) return;
      
      try {
        const res = await fetch('/get_sensors');
        if (!res.ok) throw new Error('Network response was not ok');
        const newData = await res.json();
        
        // Update temperatures while maintaining current order
        const container = document.querySelector('.sensor-container');
        if (container) {
          const currentOrder = Array.from(container.children)
            .map(item => item.dataset.address);
          
          // Update temperatures while keeping current order
          sensorData = currentOrder.map(address => {
            const newSensor = newData.find(s => s.address === address);
            return newSensor || sensorData.find(s => s.address === address);
          });
        }
        
        renderSensorList();
      } catch (error) {
        console.error('Error fetching sensors:', error);
      }
    }

    let sortableInstance = null;

    function cleanupSortable() {
      try {
        if (sortableInstance) {
          const el = sortableInstance.el;
          if (el) {
            // Remove all sortable-specific event listeners
            const clone = el.cloneNode(true);
            el.parentNode.replaceChild(clone, el);
          }
          sortableInstance.destroy();
          sortableInstance = null;
        }
      } catch (error) {
        console.error('Cleanup error:', error);
      }
    }

    function showStatus(message, type = 'info') {
      const statusDiv = document.getElementById('statusMsg');
      if (statusDiv) {
        let alertClass = 'alert ';
        switch(type) {
          case 'success':
            alertClass += 'alert-success';
            break;
          case 'error':
            alertClass += 'alert-danger';
            break;
          case 'warning':
            alertClass += 'alert-warning';
            break;
          default:
            alertClass += 'alert-info';
        }
        statusDiv.innerHTML = `<div class="${alertClass}" role="alert">${message}</div>`;
        
        // Auto-hide success and info messages after 5 seconds
        if (type === 'success' || type === 'info') {
          setTimeout(() => {
            statusDiv.innerHTML = '';
          }, 5000);
        }
      }
    }

    function renderSensorList() {
      const list = document.getElementById('sensorList');
      if (!list) return;

      // Safely cleanup existing Sortable instance
      cleanupSortable();

      list.innerHTML = '';

      // Only create sortable if we have sensors
      if (sensorData && sensorData.length > 0) {
        const container = document.createElement('div');
        container.className = 'sensor-container';

        sensorData.forEach(sensor => {
          const div = document.createElement('div');
          div.className = 'sensor-item';
          div.dataset.address = sensor.address;
          div.innerHTML = 
            '<div class="sensor-temp">' + sensor.temperature.toFixed(1) + ' °C</div>' +
            '<div class="sensor-address">' + sensor.address + '</div>';
          container.appendChild(div);
        });

        list.appendChild(container);

        // Initialize Sortable after adding items
        sortableInstance = new Sortable(container, {
          animation: 150,
          ghostClass: 'sensor-item-ghost',
          chosenClass: 'sensor-item-chosen',
          dragClass: 'sensor-item-drag',
          forceFallback: true,
          fallbackClass: 'sortable-fallback'
        });
      } else {
        list.innerHTML = '<div class="alert alert-info">Keine Sensoren gefunden</div>';
      }
    }

    document.getElementById('saveBtn').addEventListener('click', async () => {
      const container = document.querySelector('.sensor-container');
      if (!container) {
        showStatus('Keine Sensoren gefunden!', 'error');
        return;
      }
      
      showStatus('Speichere Reihenfolge...', 'info');
      
      const order = Array.from(container.children)
        .map(item => item.dataset.address);
      
      try {
        const res = await fetch('/set_order', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify(order)
        });
        
        if (!res.ok) throw new Error('Network response was not ok');
        showStatus('Reihenfolge wurde gespeichert!', 'info');
      } catch (error) {
        console.error('Save error:', error);
        showStatus('Fehler beim Speichern der Reihenfolge!', 'error');
      }
    });

    document.getElementById('saveWifiBtn').addEventListener('click', async () => {
      const ssid = document.getElementById('ssid').value;
      const password = document.getElementById('password').value;
      
      if (!ssid) {
        document.getElementById('wifiStatusMsg').innerHTML = 
          '<div class="alert alert-danger">' +
          'Bitte geben Sie einen WLAN-Namen ein!' +
          '</div>';
        return;
      }
      
      try {
        const res = await fetch('/set_wifi', {
          method: 'POST',
          headers: {
            'Content-Type': 'application/json',
          },
          body: JSON.stringify({ ssid, password }),
        });
        
        if (!res.ok) throw new Error('Network response was not ok');
        
        document.getElementById('wifiStatusMsg').innerHTML = 
          '<div class="alert alert-success">' +
          'WLAN-Daten gespeichert! Das Gerät wird neu gestartet...' +
          '</div>';
      } catch (error) {
        console.error('Error saving WiFi:', error);
        document.getElementById('wifiStatusMsg').innerHTML = 
          '<div class="alert alert-danger">' +
          'Fehler beim Speichern der WLAN-Daten!' +
          '</div>';
      }
    });

    document.getElementById('resetBtn').addEventListener('click', async () => {
      if (confirm('Möchten Sie wirklich die WLAN-Konfiguration zurücksetzen?')) {
        try {
          const res = await fetch('/reset_wifi', { method: 'POST' });
          if (!res.ok) throw new Error('Network response was not ok');
          alert('WLAN-Konfiguration wurde zurückgesetzt. Das Gerät startet neu...');
        } catch (error) {
          console.error('Reset error:', error);
          alert('Fehler beim Zurücksetzen der WLAN-Konfiguration');
        }
      }
    });

    document.getElementById('resetOrderBtn').addEventListener('click', async () => {
      if (confirm('Möchten Sie wirklich die Fühlerreihenfolge zurücksetzen?')) {
        showStatus('Setze Fühlerreihenfolge zurück...', 'info');
        
        try {
          const res = await fetch('/reset_order', { method: 'POST' });
          if (!res.ok) throw new Error('Network response was not ok');
          
          // Reload initial order after reset
          await loadInitialOrder();
          
          showStatus('Fühlerreihenfolge wurde zurückgesetzt!', 'info');
        } catch (error) {
          console.error('Reset order error:', error);
          showStatus('Fehler beim Zurücksetzen der Fühlerreihenfolge!', 'error');
        }
      }
    });

    function startAutoUpdate() {
      checkMode();
      fetchSensors();
      updateInterval = setInterval(() => {
        checkMode();
        fetchSensors();
      }, 5000);
    }

    window.addEventListener('beforeunload', () => {
      if (updateInterval) clearInterval(updateInterval);
      cleanupSortable();
    });

    checkMode();
  </script>
</body>
</html>
)rawliteral";
