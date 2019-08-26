document.addEventListener('DOMContentLoaded', function (event) {
  var baseHost = document.location.origin
  var streamUrl = baseHost + ':81'

  const framesize = document.getElementById('framesize')
  const ledGroup = document.getElementById('led-group')
  const awb = document.getElementById('awb_gain')
  const wb = document.getElementById('wb_mode-group')
  const agc = document.getElementById('agc')
  const agcGain = document.getElementById('agc_gain-group')
  const gainCeiling = document.getElementById('gainceiling-group')
  const aec = document.getElementById('aec')
  const exposure = document.getElementById('aec_value-group')
  const mdns = document.getElementById('mdns')
  const mdns_instance = document.getElementById('mdns-instance-group')
  const dhcp = document.getElementById('dhcp')
  const ip = document.getElementById('ip-group')
  const netmask = document.getElementById('netmask-group')
  const gateway = document.getElementById('gateway-group')
  const dns1 = document.getElementById('dns1-group')
  const dns2 = document.getElementById('dns2-group')
  const restoreButton = document.getElementById('restore-defaults')
  const rebootButton = document.getElementById('reboot-camera')
  const storeButton = document.getElementById('store-settings')
  const refreshButton = document.getElementById('refresh-settings')
  const streamButton = document.getElementById('toggle-stream')
  const stillButton = document.getElementById('get-still')
  const view = document.getElementById('stream')
  const viewContainer = document.getElementById('stream-container')
  const streamWindowLink = document.getElementById('stream-window-link')

  function hide(el) {
    el.classList.add('hidden')
  }
  
  function show(el) {
    el.classList.remove('hidden')
  }

  function disable(el) {
    el.classList.add('disabled')
    el.disabled = true
  }

  function enable(el) {
    el.classList.remove('disabled')
    el.disabled = false
  }

  function stopStream() {
    window.stop();
    streamButton.innerHTML = 'Start Stream'
  }

  function startStream() {
    console.log("Starting Stream")
    view.src = `${streamUrl}/stream`
    show(view)
    show(viewContainer)
    streamButton.innerHTML = 'Stop Stream'
  }

  function rebootCamera() {
    const query = `${baseHost}/reboot`
    fetch(query)
      .then(response => {
         console.log(`request to ${query} finished, status: ${response.status}`)
         if (response.status == 200) 
            //Reload the page and ignore the browser cache.
            window.location.reload(true);
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

  function fetchSettings() {
    fetch(`${baseHost}/status`)
      .then(function (response) {
        return response.json()
      })
      .then(function (state) {
        document
          .querySelectorAll('.default-action')
          .forEach(el => {
            updateValue(el, state[el.id], false)
          })
        document.title = state.hostname
        var pageTitle = document.getElementById("page-title")
        if (pageTitle) {
          pageTitle.innerHTML = state.hostname
        }
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

  function updateValue(el, value, updateRemote) {
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
          show(gainCeiling)
          hide(agcGain)
        } else {
          hide(gainCeiling)
          show(agcGain)
        }
      } else if(el.id === "awb_gain"){
        value ? show(wb) : hide(wb)
      } else if(el.id == "led_intensity"){
        value > -1 ? show(ledGroup) : hide(ledGroup)
      } else if(el.id == "dhcp"){
        if (value) {
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

  // Attach actions to buttons

  restoreButton.onclick = () => {
    if (confirm("Are you sure you want to restore default settings?")) {
      stopStream()
      hide(viewContainer)
      resetDefaults()
      //rebootCamera()
    }
  }

  rebootButton.onclick = () => {
    if (confirm("Are you sure you want to reboot the camera?")) {
      stopStream()
      hide(viewContainer)
      rebootCamera()
    }
  }

  storeButton.onclick = () => {
    storeSettings();
  }

  refreshButton.onclick = () => {
    fetchSettings();
  }

  stillButton.onclick = () => {
    stopStream()
    view.src = `${baseHost}/capture?_cb=${Date.now()}`
    show(viewContainer)
  }

  streamButton.onclick = () => {
    const streamEnabled = streamButton.innerHTML === 'Stop Stream'
    if (streamEnabled) {
      stopStream()
    } else {
      startStream()
    }
  }

  streamWindowLink.onclick = () => {
    const streamEnabled = streamButton.innerHTML === 'Stop Stream'
    if (streamEnabled) {
      stopStream()
    }
  }

  streamButton.onerror = () => {
    window.stop()
    streamButton.innerHTML = 'Start Stream'
    hide(view)
    hide(viewContainer)
  }
  
  // Attach default on change action
  document
  .querySelectorAll('.default-action')
  .forEach(el => {
    el.onchange = () => updateConfig(el)
  })
  
  agc.onchange = () => {
    updateConfig(agc)
    if (agc.checked) {
      show(gainCeiling)
      hide(agcGain)
    } else {
      hide(gainCeiling)
      show(agcGain)
    }
  }

  aec.onchange = () => {
    updateConfig(aec)
    aec.checked ? hide(exposure) : show(exposure)
  }

  awb.onchange = () => {
    updateConfig(awb)
    awb.checked ? show(wb) : hide(wb)
  }
  
  framesize.onchange = () => {
    updateConfig(framesize)
  }

  mdns.onchange = () => {
    if (mdns.checked) {
      show(mdns_instance)
    } else {
      hide(mdns_instance)
    }
  }
  
  dhcp.onchange = () => {
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

  document
    .querySelectorAll('.close')
    .forEach(el => {
      el.onclick = () => {
        hide(el.parentNode)
      }
    })

  streamWindowLink.href = `${streamUrl}/stream`
  fetchSettings()
  startStream()
})
