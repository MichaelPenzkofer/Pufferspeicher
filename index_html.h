const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="de">
<head>
  <meta charset="UTF-8">
  <title>Sensorreihenfolge konfigurieren</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    :root {
      --primary: #0d6efd;
      --bg: #f8f9fa;
    }
    body {
      background-color: var(--bg);
      padding: 2rem;
      font-family: Arial, sans-serif;
    }
    .container { max-width: 800px; margin: auto; }
    .card { background: #fff; border-radius: 0.5rem; box-shadow: 0 2px 6px rgba(0,0,0,0.1); margin-bottom: 2rem; }
    .card-header { background: var(--primary); color: white; padding: 1rem; border-radius: 0.5rem 0.5rem 0 0; }
    .card-body { padding: 1rem; }
    .form-label { display: block; margin-bottom: 0.5rem; }
    .form-control { width: 100%; padding: 0.5rem; border: 1px solid #ccc; border-radius: 0.375rem; margin-bottom: 1rem; }
    .btn { padding: 0.5rem 1rem; border: none; border-radius: 0.375rem; cursor: pointer; font-size: 1rem; }
    .btn-primary { background: var(--primary); color: white; }
    .btn-danger { background: #dc3545; color: white; }
    .btn-warning { background: #ffc107; color: black; }
    .alert {
      padding: 1rem; margin-bottom: 1rem; border-radius: 0.375rem;
    }
    .alert-info { background: #d1ecf1; color: #0c5460; }
    .alert-success { background: #d4edda; color: #155724; }
    .alert-danger { background: #f8d7da; color: #721c24; }
    .alert-warning { background: #fff3cd; color: #856404; }

    .sensor-container { min-height: 50px; }
    .sensor-item {
      padding: 1rem; margin-bottom: 0.5rem; background: white;
      border: 1px solid #ced4da; border-radius: 0.5rem;
      cursor: grab; user-select: none; text-align: center;
    }
    .sensor-temp { font-size: 1.5rem; font-weight: bold; }
    .sensor-address { font-size: 0.9rem; color: #6c757d; }
    .sensor-item-ghost,
    .sortable-swap-highlight {
      background-color: #e2e6ea !important;
      border: 2px dashed #6c757d;
    }
  </style>
</head>
<body>
  <div class="container">
    <div id="wifiConfig" style="display:none">
      <div class="card">
        <div class="card-header">📶 WLAN-Konfiguration</div>
        <div class="card-body">
          <div id="wifiStatusMsg"></div>
          <form id="wifiForm">
            <label class="form-label" for="ssid">📡 WLAN-Name (SSID):</label>
            <input class="form-control" id="ssid" name="ssid" required>
            <label class="form-label" for="password">🔑 Passwort:</label>
            <input class="form-control" type="password" id="password" name="password" required>
            <button class="btn btn-primary" type="submit">💾 Speichern</button>
          </form>
        </div>
      </div>
    </div>

    <div id="sensorConfig" style="display:none">
      <div class="card">
        <div class="card-header">🌡️ Sensorkonfiguration</div>
        <div class="card-body">
          <div class="alert alert-info">ℹ️ Ziehen Sie die Sensoren von oben nach unten in die gewünschte Reihenfolge.</div>
          <div id="statusMsg"></div>
          <div id="sensorList" class="mb-3"></div>
          <button id="saveBtn" class="btn btn-primary">💾 Speichern</button>
          <button id="resetOrderBtn" class="btn btn-warning">🔄 Reihenfolge zurücksetzen</button>
          <button id="resetBtn" class="btn btn-danger">🧹 WLAN zurücksetzen</button>
        </div>
      </div>
    </div>
  </div>

  <!-- SortableJS mit Swap Plugin -->
  <script src="https://cdn.jsdelivr.net/npm/sortablejs@1.15.0/Sortable.min.js"></script>
  <script src="https://cdn.jsdelivr.net/npm/sortablejs@1.15.0/plugins/Swap/Sortable.swap.min.js"></script>
  <script>
    let sensorData = [], updateInterval, sortableInstance, initialOrderLoaded = false;

    async function checkMode() {
      try {
        const res = await fetch('/get_mode');
        const data = await res.json();
        const isInAPMode = data.isAP;
        document.getElementById('wifiConfig').style.display = isInAPMode ? 'block' : 'none';
        document.getElementById('sensorConfig').style.display = isInAPMode ? 'none' : 'block';
        
        if (isInAPMode) {
          if (updateInterval) {
            clearInterval(updateInterval);
            updateInterval = null;
          }
        } else if (!updateInterval) {
          startAutoUpdate();
        }
        
        if (!isInAPMode && !initialOrderLoaded) {
          await loadInitialOrder();
        }
        
        return isInAPMode;
      } catch (e) { 
        console.error('checkMode error:', e);
        return false;
      }
    }

    async function loadInitialOrder() {
      showStatus('⏳ Lade Reihenfolge...', 'info');
      try {
        const res = await fetch('/get_initial_order');
        sensorData = await res.json();
        initialOrderLoaded = true;
        renderSensorList();
        showStatus('✅ Reihenfolge geladen.', 'success');
      } catch (e) {
        showStatus('❌ Fehler beim Laden!', 'danger');
      }
    }

    async function fetchSensors() {
      try {
        const res = await fetch('/get_sensors');
        const newData = await res.json();
        newData.forEach(sensor => {
          const tempEl = document.querySelector(`[data-address="${sensor.address}"] .sensor-temp`);
          if (tempEl) tempEl.textContent = `${sensor.temperature.toFixed(1)} °C`;
        });
      } catch (e) { console.error('fetchSensors error:', e); }
    }

    function renderSensorList() {
      const list = document.getElementById('sensorList');
      list.innerHTML = '';
      const container = document.createElement('div');
      container.className = 'sensor-container';
      sensorData.forEach(s => {
        const div = document.createElement('div');
        div.className = 'sensor-item';
        div.dataset.address = s.address;
        div.innerHTML = `<div class="sensor-temp">${s.temperature.toFixed(1)} °C</div><div class="sensor-address">${s.address}</div>`;
        container.appendChild(div);
      });
      list.appendChild(container);
      sortableInstance = new Sortable(container, {
        animation: 150,
        swap: true,
        swapClass: 'sortable-swap-highlight',
        ghostClass: 'sensor-item-ghost',
        chosenClass: 'sensor-item-chosen',
        forceFallback: true
      });
    }

    function showStatus(msg, type) {
      const map = {
        success: 'alert-success',
        danger: 'alert-danger',
        info: 'alert-info',
        warning: 'alert-warning'
      };
      const el = document.getElementById('statusMsg');
      el.innerHTML = `<div class="alert ${map[type] || 'alert-info'}">${msg}</div>`;
      if (type === 'success' || type === 'info') setTimeout(() => { el.innerHTML = ''; }, 5000);
    }

    document.addEventListener('DOMContentLoaded', async () => {
      const isAP = await checkMode();
      if (!isAP) startAutoUpdate();

      document.getElementById('wifiForm').addEventListener('submit', async (e) => {
        e.preventDefault();
        const data = {
          ssid: document.getElementById('ssid').value,
          password: document.getElementById('password').value
        };
        try {
          await fetch('/set_wifi', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
          });
          showStatus('✅ WLAN gespeichert, Neustart...', 'success');
          setTimeout(() => location.reload(), 4000);
        } catch {
          showStatus('❌ WLAN speichern fehlgeschlagen', 'danger');
        }
      });

      document.getElementById('saveBtn').addEventListener('click', async () => {
        const order = Array.from(document.querySelectorAll('.sensor-item')).map(i => i.dataset.address);
        try {
          await fetch('/set_order', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(order)
          });
          showStatus('✅ Reihenfolge gespeichert', 'success');
        } catch {
          showStatus('❌ Speichern fehlgeschlagen', 'danger');
        }
      });

      document.getElementById('resetBtn').addEventListener('click', async () => {
        if (confirm('WLAN zurücksetzen?')) {
          await fetch('/reset_wifi', { method: 'POST' });
          showStatus('✅ WLAN zurückgesetzt. Neustart...', 'success');
          // Start checking mode more frequently after reset
          let checkCount = 0;
          const modeChecker = setInterval(async () => {
            checkCount++;
            if (await checkMode() || checkCount > 20) {
              clearInterval(modeChecker);
              if (checkCount > 20) location.reload();
            }
          }, 500);
        }
      });

      document.getElementById('resetOrderBtn').addEventListener('click', async () => {
        if (confirm('Reihenfolge zurücksetzen?')) {
          await fetch('/reset_order', { method: 'POST' });
          await loadInitialOrder();
        }
      });
    });

    function startAutoUpdate() {
      fetchSensors();
      updateInterval = setInterval(fetchSensors, 5000);
    }
  </script>
</body>
</html>
)rawliteral";
