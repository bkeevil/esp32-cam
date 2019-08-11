document.addEventListener('DOMContentLoaded', function (event) {
  var baseHost = document.location.origin
  var streamUrl = baseHost + ':81'

  const hide = el => {
    el.classList.add('hidden')
  }
  
  const show = el => {
    el.classList.remove('hidden')
  }

  const disable = el => {
    el.classList.add('disabled')
    el.disabled = true
  }

  const enable = el => {
    el.classList.remove('disabled')
    el.disabled = false
  }

  const updateValue = (el, value, updateRemote) => {
    updateRemote = updateRemote == null ? true : updateRemote
    let initialValue
    if (el.type === 'checkbox') {
      initialValue = el.checked
      value = !!value
      el.checked = value
    } else {
      initialValue = el.value
      el.value = value
    }

    if (updateRemote && initialValue !== value) {
      updateConfig(el);
    } else if(!updateRemote){
      if(el.id === "aec"){
        value ? hide(exposure) : show(exposure)
      } else if(el.id === "agc"){
        if (value) {
          show(gainCeiling) // Not for OV3660
          hide(agcGain)
        } else {
          hide(gainCeiling) // Not for OV3660
          show(agcGain)
        }
      } else if(el.id === "awb_gain"){
        value ? show(wb) : hide(wb)
      } else if(el.id == "led_intensity"){
        value > -1 ? show(ledGroup) : hide(ledGroup)
      } else if(el.id == "dhcp"){
        if (value) {
          console.log("using dhcp")
          hide(ip)
          hide(netmask)
          hide(gateway)
          hide(dns1)
          hide(dns2)
        } else {
          console.log("using static")
          show(ip)
          show(netmask)
          show(gateway)
          show(dns1)
          show(dns2)
        }
      }  
    }
  }

  function updateConfig (el) {
    let value
    switch (el.type) {
      case 'checkbox':
        value = el.checked ? 1 : 0
        break
      case 'range':
      case 'text':
      case 'password':
      case 'select-one':
        value = el.value
        break
      case 'button':
      case 'submit':
        value = '1'
        break
      default:
        return
    }

    const query = `${baseHost}/control?var=${el.id}&val=${value}`

    fetch(query)
      .then(response => {
        console.log(`request to ${query} finished, status: ${response.status}`)
      })
  }

  document
    .querySelectorAll('.close')
    .forEach(el => {
      el.onclick = () => {
        hide(el.parentNode)
      }
    })

  fetchSettings();

  const view = document.getElementById('stream')
  const viewContainer = document.getElementById('stream-container')
  const restoreButton = document.getElementById('restore-defaults')
  const rebootButton = document.getElementById('reboot-camera')
  const stillButton = document.getElementById('get-still')
  const storeButton = document.getElementById('store-settings')
  const refreshButton = document.getElementById('refresh-settings')
  const streamButton = document.getElementById('toggle-stream')
  const closeButton = document.getElementById('close-stream')
  const saveButton = document.getElementById('save-still')
  const ledGroup = document.getElementById('led-group')

  const stopStream = () => {
    window.stop();
    streamButton.innerHTML = 'Start Stream'
  }

  const startStream = () => {
    view.src = `${streamUrl}/stream`
    show(viewContainer)
    streamButton.innerHTML = 'Stop Stream'
  }

  function rebootCamera() {
    const query = `${baseHost}/reboot`
    fetch(query)
      .then(response => {
         console.log(`request to ${query} finished, status: ${response.status}`)
         if (response.status == 200) 
           alert("Camera is restarting")
      })    
  }

  function storeSettings() {
    const query = `${baseHost}/store`
    fetch(query)
      .then(response => {
         console.log(`request to ${query} finished, status: ${response.status}`)
         if (response.status != 200) 
           alert("Failed to store camera settings. Is the camera connected?")
      })
  }

  // read initial values
  function fetchSettings() {
    fetch(`${baseHost}/status`)
      .then(function (response) {
        return response.json()
      })
      .then(function (state) {
        console.log(state);
        document
          .querySelectorAll('.default-action')
          .forEach(el => {
            updateValue(el, state[el.id], false)
          })
      })
  }

  function resetDefaults() {
    const query = `${baseHost}/reset`
    fetch(query)
      .then(response => {
         console.log(`request to ${query} finished, status: ${response.status}`)
         if (response.status != 200) 
           alert("Failed to reset the camera to firmware defaults. Is the camera connected?")
      })
  }

  // Attach actions to buttons
  restoreButton.onclick = () => {
    if (confirm("Are you sure you want to restore default settings?")) {
      stopStream()
      hide(viewContainer)
      resetDefaults()
      rebootCamera()
    }
  }

  rebootButton.onclick = () => {
    if (confirm("Are you sure you want to reboot the camera?")) {
      stopStream()
      hide(viewContainer)
      rebootCamera()
    }
  }

  stillButton.onclick = () => {
    stopStream()
    view.src = `${baseHost}/capture?_cb=${Date.now()}`
    show(viewContainer)
  }

  closeButton.onclick = () => {
    stopStream()
    hide(viewContainer)
  }

  storeButton.onclick = () => {
    storeSettings();
  }

  refreshButton.onclick = () => {
    fetchSettings();
  }

  streamButton.onclick = () => {
    const streamEnabled = streamButton.innerHTML === 'Stop Stream'
    if (streamEnabled) {
      stopStream()
    } else {
      startStream()
    }
  }

  saveButton.onclick = () => {
    var canvas = document.createElement("canvas");
    canvas.width = view.width;
    canvas.height = view.height;
    document.body.appendChild(canvas);
    var context = canvas.getContext('2d');
    context.drawImage(view,0,0);
    try {
      var dataURL = canvas.toDataURL('image/jpeg');
      saveButton.href = dataURL;
      var d = new Date();
      saveButton.download = d.getFullYear() + ("0"+(d.getMonth()+1)).slice(-2) + ("0" + d.getDate()).slice(-2) + ("0" + d.getHours()).slice(-2) + ("0" + d.getMinutes()).slice(-2) + ("0" + d.getSeconds()).slice(-2) + ".jpg";
    } catch (e) {
      console.error(e);
    }
    canvas.parentNode.removeChild(canvas);
  }

  // Attach default on change action
  document
    .querySelectorAll('.default-action')
    .forEach(el => {
      el.onchange = () => updateConfig(el)
    })

  // Custom actions

  // Gain
  const agc = document.getElementById('agc')
  const agcGain = document.getElementById('agc_gain-group')
  const gainCeiling = document.getElementById('gainceiling-group') // Not for 3660
  
  agc.onchange = () => {
    updateConfig(agc)
    if (agc.checked) {
      show(gainCeiling)  // Not for 3660
      hide(agcGain)
    } else {
      hide(gainCeiling)  // Not for 3660
      show(agcGain)
    }
  }

  // Exposure
  const aec = document.getElementById('aec')
  const exposure = document.getElementById('aec_value-group')
  aec.onchange = () => {
    updateConfig(aec)
    aec.checked ? hide(exposure) : show(exposure)
  }

  // AWB
  const awb = document.getElementById('awb_gain')
  const wb = document.getElementById('wb_mode-group')
  awb.onchange = () => {
    updateConfig(awb)
    awb.checked ? show(wb) : hide(wb)
  }

  // framesize
  const framesize = document.getElementById('framesize')

  framesize.onchange = () => {
    updateConfig(framesize)
  }

  // network
  const dhcp = document.getElementById('dhcp')
  const ip = document.getElementById('ip-group')
  const netmask = document.getElementById('netmask-group')
  const gateway = document.getElementById('gateway-group')
  const dns1 = document.getElementById('dns1-group')
  const dns2 = document.getElementById('dns2-group')

  dhcp.onchange = () => {
      console.log("dhcp.onchange")
      if (dhcp.checked) {
          hide(ip)
          hide(netmask)
          hide(gateway)
          hide(dns1)
          hide(dns2)
      } else {
          show(ip)
          show(netmask)
          show(gateway)
          show(dns1)
          show(dns2)
      }
  }
})
