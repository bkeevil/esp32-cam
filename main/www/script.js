document.addEventListener('DOMContentLoaded', function(event) {
    var baseHost = document.location.origin
    var streamUrlbase = baseHost + ':81/stream'
    var streamUrl = streamUrlbase

    const framesize = document.getElementById('framesize')
    const ledGroup = document.getElementById('led-group')
    const awb = document.getElementById('awb_gain')
    const wb = document.getElementById('wb_mode-group')
    const agc = document.getElementById('agc')
    const agcGain = document.getElementById('agc_gain-group')
    const gainCeiling = document.getElementById('gainceiling-group')
    const aec = document.getElementById('aec')
    const exposure = document.getElementById('aec_value-group')
    const mdns_instance = document.getElementById('mdns-instance-group')
    const ntpServer = document.getElementById('ntp_server-group')
    const timezone = document.getElementById('timezone-group')
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
    const http_auth = document.getElementById('http_auth')
    const http_password = document.getElementById('http_password-group')
    const http_user = document.getElementById('http_user-group')

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
  //      var surl = `${streamUrl}/stream` 
  //      if (state.http_auth == 1) {
  //    //add username and password when auth enabled
  //  	   surl = surl.replace("://", "://" + state.http_user + ":" + state.http_password + "@")
  //      }
        
        console.log("Starting Stream: " + `${streamUrl}`)
        
        view.src = `${streamUrl}`
        
        show(view)
        show(viewContainer)
        streamButton.innerHTML = 'Stop Stream'
    }
     
    function refreshme() {
	    window.location.reload(true);
    }

    function rebootCamera() {
        const query = `${baseHost}/reboot`
        fetch(query)
            .then(response => {
                console.log(`request to ${query} finished, status: ${response.status}`)
                if (response.status == 200)
                    //Reload the page and ignore the browser cache after 5seconds
                    setTimeout(refreshme, 10000)
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
            .then(function(response) {
                return response.json()
            })
            .then(function(state) {
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
                if (typeof state.mdns_instance == 'undefined') {
                    hide(mdns_instance)
                }
                if (typeof state.ntp_server == 'undefined') {
                    hide(ntpServer)
                    hide(timezone)
                }
                

		        if (state.http_auth == 1) {
			        //add username and password when auth enabled
	  		        streamUrl = `${streamUrlbase}` + '-' + state.http_password 
	  		        streamWindowLink.href = `${streamUrlbase}` + '-' + state.http_password 
		        } else {
		            streamUrl = `${streamUrlbase}`
 		            streamWindowLink.href = `${streamUrlbase}`
 		        }
                
                // Update the LED intensity slider max-value together with the related label
                if (state.led_intensity !== -1 && state.led_max_intensity) {
                    let led_intensity_slider = document.getElementById("led_intensity");
                    led_intensity_slider.max = state.led_max_intensity;
                    let led_intensity_range_max = document.querySelector('#led-group > div.range-max');
                    led_intensity_range_max.innerText = state.led_max_intensity;
                }
                
                document.getElementById("fps_out").value = state.fps
                document.getElementById("quality_out").value = state.quality
                document.getElementById("brightness_out").value = state.brightness
                document.getElementById("contrast_out").value = state.contrast
                document.getElementById("saturation_out").value = state.saturation
                document.getElementById("led_intensity_out").value = state.led_intensity
                document.getElementById("ae_level_out").value = state.ae_level
                document.getElementById("aec_value_out").value = state.aec_value
                document.getElementById("agc_gain_out").value = state.agc_gain
                document.getElementById("gainceiling_out").value = state.gainceiling
                
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
            // Prevent undefined or null stringified values
            el.value = (value === undefined || value === null) ? '' : value
        }

        if (updateRemote && initialValue !== value) {
            updateConfig(el);
        } else if (!updateRemote) {
            if (el.id === "aec") {
                value ? hide(exposure) : show(exposure)
            } else if (el.id === "agc") {
                if (value) {
                    show(gainCeiling)
                    hide(agcGain)
                } else {
                    hide(gainCeiling)
                    show(agcGain)
                }
            } else if (el.id === "awb_gain") {
                value ? show(wb) : hide(wb)
            } else if (el.id == "led_intensity") {
                value > -1 ? show(ledGroup) : hide(ledGroup)
            } else if (el.id == "dhcp") {
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
            } else if(el.id == "http_auth"){
	            if (value) {
		            show(http_user)
		            show(http_password)
	            } else {
		            hide(http_user)
		            hide(http_password)
	            }
            } 
        }
    }

    function updateConfig(el) {
        let value
        switch (el.type) {
            case 'checkbox':
                value = el.checked ? 1 : 0
                break
            case 'range':
            case 'text':
            case 'password':
            case 'select-one':
                value = encodeURIComponent(el.value)
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

    dhcp.onchange = () => {
        updateConfig(dhcp)
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


    http_auth.onchange = () => {
        updateConfig(http_auth)
        if (http_auth.checked) {
            show(http_user)
		    show(http_password)
        } else {
		    hide(http_user)
		    hide(http_password)
        }
    }

    document
        .querySelectorAll('.close')
        .forEach(el => {
            el.onclick = () => {
                hide(el.parentNode)
            }
        })

    streamWindowLink.href = `${streamUrlbase}`
    fetchSettings()
    setTimeout(() => { startStream() ; }, 2000)
})
