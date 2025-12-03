
    function updateTime() {
      // Use Philippines timezone (UTC+8)
      const timeOptions = { 
        hour: '2-digit', 
        minute: '2-digit', 
        second: '2-digit', 
        hour12: true,
        timeZone: 'Asia/Manila'
      };
      const dateOptions = { 
        weekday: 'long', 
        year: 'numeric', 
        month: 'long', 
        day: 'numeric',
        timeZone: 'Asia/Manila'
      };
      
      const now = new Date();
      document.getElementById('currentTime').innerText = now.toLocaleTimeString('en-US', timeOptions);
      document.getElementById('currentDate').innerText = now.toLocaleDateString('en-US', dateOptions);
    }
    
    updateTime();
    setInterval(updateTime, 1000);
    
    // Set default date to today
    const today = new Date().toISOString().split('T')[0];
    document.getElementById('fertilizeDate').value = today;
    
    function updateStatus(message, isError = false) {
      const statusEl = document.getElementById('status');
      const dotEl = document.getElementById('statusDot');
      
      statusEl.innerText = message;
      if (isError) {
        dotEl.classList.add('error');
      } else {
        dotEl.classList.remove('error');
      }
    }
    
    function sendCommand(cmd) {
      updateStatus(`Sending ${cmd} command...`);
      fetch(`http://localhost:8000/command?cmd=${cmd}`)
        .then(() => updateStatus(`✓ Command sent: ${cmd.charAt(0).toUpperCase() + cmd.slice(1)}`))
        .catch(() => updateStatus('✗ Error sending command', true));
    }
    
    function setFertilizeSchedule() {
      const date = document.getElementById("fertilizeDate").value;
      const time = document.getElementById("fertilizeTime").value;
      
      if (!date) {
        updateStatus('Please select a date', true);
        return;
      }
      
      updateStatus('Setting schedule...');
      fetch(`http://localhost:8000/setdate?date=${date}&time=${time}`)
        .then(() => {
          const formattedDate = new Date(date + 'T' + time).toLocaleDateString('en-US', {
            weekday: 'short',
            month: 'short',
            day: 'numeric',
            hour: '2-digit',
            minute: '2-digit'
          });
          updateStatus(`✓ Scheduled for: ${formattedDate}`);
        })
        .catch(() => updateStatus('✗ Error setting schedule', true));
    }
