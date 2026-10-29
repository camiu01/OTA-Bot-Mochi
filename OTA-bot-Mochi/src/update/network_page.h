/**
 * @file network_page.h
 * @brief Saved-network and city selection controls in the incumbent offline settings page.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
static const char NETWORK_FORM[] = R"FORM(
<section id="network"><h2 id="netTitle"></h2><p id="netNote" class="note"></p>
<p id="netState" role="status" aria-live="polite"></p>
<form id="netForm"><fieldset id="netFields" disabled>
<label class="toggle"><input id="netEnabled" type="checkbox"><span id="netEnabledLabel"></span></label>
<label id="netSsidLabel" for="netSsid"></label><input id="netSsid" maxlength="32" autocomplete="off">
<label id="netModeLabel" for="netMode"></label><select id="netMode"></select>
<label id="netPasswordLabel" for="netPassword"></label><input id="netPassword" type="password" maxlength="64" autocomplete="new-password">
<p id="netPrivacy" class="note"></p>
<label id="netServerLabel" for="netServer"></label><input id="netServer" maxlength="63" value="pool.ntp.org" required autocomplete="off">
<label id="netZoneLabel" for="netZone"></label><select id="netZone"><option value="0">UTC</option><option value="1">Europe/Rome</option></select>
<label class="toggle"><input id="netWeather" type="checkbox"><span id="netWeatherLabel"></span></label>
<p><span id="netCityLabel"></span> <strong id="netCity"></strong></p>
<div class="tools"><button id="netSave" type="submit"></button><button id="netReload" type="button"></button><button id="netForget" type="button"></button></div>
</fieldset></form>
<p id="netResult" role="status" aria-live="polite"></p>
<form id="citySearch"><label id="cityQueryLabel" for="cityQuery"></label>
<input id="cityQuery" minlength="2" maxlength="64" required autocomplete="off" aria-describedby="cityNote">
<p id="cityNote" class="note"></p><button id="cityFind" type="submit" disabled></button></form>
<label id="cityChoiceLabel" for="cityChoice"></label><select id="cityChoice" disabled></select>
<p id="cityResult" role="status" aria-live="polite"></p>
<div class="tools"><button id="sync" disabled></button><button id="weatherRefresh" disabled></button></div>
<p class="note"><a href="https://open-meteo.com/" target="_blank" rel="noreferrer">Open-Meteo</a> · <a href="https://www.geonames.org/" target="_blank" rel="noreferrer">GeoNames</a></p>
</section>
)FORM";
static const char NETWORK_SCRIPT[] = R"JS(
const nw=it?{
 netTitle:'Wi-Fi, ora e meteo',netNote:'Salva la rete per riconnettere Mochi all’avvio. La configurazione Wi-Fi si apre solo dalle impostazioni. La pagina impostazioni resta aperta solo per questa sessione.',
 netEnabledLabel:'Connessione automatica al Wi-Fi',netSsidLabel:'Nome della rete (2,4 GHz)',netModeLabel:'Password della rete',
 modes:['Mantieni la password salvata','Imposta una nuova password','Rete aperta, senza password'],netPasswordLabel:'Nuova password Wi-Fi',
 netPrivacy:'SSID e password vengono salvati sulla scheda, in memoria non cifrata. La password non viene mostrata nel browser né inviata al servizio meteo.',
 netServerLabel:'Server NTP automatico',netZoneLabel:'Fuso orario dell’orologio',netWeatherLabel:'Aggiorna il meteo via Internet ogni 15 minuti',
 netCityLabel:'Città salvata:',none:'Nessuna città selezionata',netSave:'Salva rete, ora e meteo',netReload:'Ricarica rete',netForget:'Dimentica rete',
 cityQueryLabel:'Cerca la città per il meteo',cityNote:'Prima salva il Wi-Fi e attendi la connessione e l’ora NTP. Cerca per nome (anche “Roma, Italia”), scegli un risultato e salva di nuovo.',
 cityFind:'Cerca città',cityChoiceLabel:'Risultati: scegli la città, poi salva',choose:'Scegli una città…',empty:'Nessuna città trovata. Prova un nome più preciso.',
 sync:'Sincronizza ora',weatherRefresh:'Aggiorna meteo',saved:'Rete salvata. Mochi aggiorna la connessione; se il browser perde il collegamento, scansiona di nuovo il QR sul display.',
 invalid:'Controlla nome rete, modalità password, server NTP e città selezionata.',storage:'Salvataggio non riuscito. La rete precedente è rimasta intatta; reinserisci la nuova password e riprova.',
 failed:'Collegamento perso o sessione scaduta. Apri Impostazioni Wi-Fi sul display e scansiona il nuovo QR.',
 conflict:'La configurazione è cambiata. Premi Ricarica rete prima di salvare.',changed:'Modifiche alla rete non salvate.',discard:'Scartare le modifiche alla rete?',
 forget:'Dimenticare la rete e disattivare ora e meteo automatici? Le altre impostazioni resteranno intatte.',
 forgotten:'Rete dimenticata. La riconnessione automatica è disattivata.',busy:'Attendi che Mochi sia connesso e che l’operazione in corso termini, poi riprova.',
 queued:'Aggiornamento richiesto.',limited:'Attendi almeno 30 secondi tra gli aggiornamenti.',searching:'Ricerca della città…',selected:'Città selezionata. Premi Salva rete, ora e meteo.',
 scan:'Scansiona il QR sul display di Mochi per autorizzare questa pagina.',time:'Ora',weather:'Meteo',
 states:{unconfigured:'Rete non configurata',disabled:'Wi-Fi automatico disattivato',updating:'Applicazione della nuova rete…',connecting:'Connessione al Wi-Fi…',connected:'Connesso',offline:'Rete non disponibile; nuovo tentativo automatico',wifi_failed:'Wi-Fi non disponibile',idle:'In attesa',querying:'Richiesta NTP…',synced:'Sincronizzata',timeout:'Tempo scaduto; riproverà',invalid_time:'Ora non valida; riproverà',cancelled:'Annullata',ready:'Aggiornato',loading:'Aggiornamento…',failed:'Aggiornamento fallito; dati precedenti mantenuti',unavailable:'Servizio non disponibile'}
}:{
 netTitle:'Wi-Fi, time and weather',netNote:'Save a network to reconnect at startup. Open Wi-Fi configuration manually from Settings. This settings page is available only during this session.',
 netEnabledLabel:'Connect to Wi-Fi automatically',netSsidLabel:'Network name (2.4 GHz)',netModeLabel:'Network password',
 modes:['Keep the saved password','Set a new password','Open network, no password'],netPasswordLabel:'New Wi-Fi password',
 netPrivacy:'SSID and password are saved on this board in unencrypted storage. The password is never shown in the browser or sent to the weather provider.',
 netServerLabel:'Automatic NTP server',netZoneLabel:'Clock time zone',netWeatherLabel:'Update Internet weather every 15 minutes',
 netCityLabel:'Saved city:',none:'No city selected',netSave:'Save network, time and weather',netReload:'Reload network',netForget:'Forget network',
 cityQueryLabel:'Search for the weather city',cityNote:'First save Wi-Fi and wait for connection and NTP time. Search by name (for example “Rome, Italy”), choose a result and save again.',
 cityFind:'Search cities',cityChoiceLabel:'Results: choose a city, then save',choose:'Choose a city…',empty:'No city found. Try a more specific name.',
 sync:'Sync time now',weatherRefresh:'Refresh weather',saved:'Network saved. Mochi is updating the connection; if the browser disconnects, scan the QR on its display again.',
 invalid:'Check the network name, password mode, NTP server and selected city.',storage:'Storage failed. The previous network is intact; re-enter the new password and retry.',
 failed:'Connection lost or session expired. Open Wi-Fi settings on Mochi and scan its new QR.',
 conflict:'Configuration changed. Reload network before saving.',changed:'Unsaved network changes.',discard:'Discard unsaved network changes?',
 forget:'Forget this network and disable automatic time and weather? Other preferences will stay intact.',
 forgotten:'Network forgotten. Automatic reconnection is disabled.',busy:'Wait for Mochi to connect and finish the current operation, then retry.',
 queued:'Refresh requested.',limited:'Wait at least 30 seconds between refreshes.',searching:'Searching for cities…',selected:'City selected. Press Save network, time and weather.',
 scan:'Scan the QR on Mochi to authorize this page.',time:'Time',weather:'Weather',
 states:{unconfigured:'Network not configured',disabled:'Automatic Wi-Fi disabled',updating:'Applying new network…',connecting:'Connecting to Wi-Fi…',connected:'Connected',offline:'Network unavailable; retrying automatically',wifi_failed:'Wi-Fi unavailable',idle:'Waiting',querying:'Requesting NTP…',synced:'Synced',timeout:'Timed out; will retry',invalid_time:'Invalid time; will retry',cancelled:'Cancelled',ready:'Updated',loading:'Updating…',failed:'Refresh failed; previous data retained',unavailable:'Service unavailable'}
};
for(const id of ['netTitle','netNote','netEnabledLabel','netSsidLabel','netModeLabel','netPasswordLabel','netPrivacy','netServerLabel','netZoneLabel','netWeatherLabel','netCityLabel','netSave','netReload','netForget','cityQueryLabel','cityNote','cityFind','cityChoiceLabel','sync','weatherRefresh'])el(id).textContent=nw[id];
for(const [index,value] of ['keep','replace','open'].entries())el('netMode').add(new Option(nw.modes[index],value));
let netRevision=0,netDirty=false,netBusy=false,netLoaded=false,netOnline=false,netAlive=true,cityBusy=false,choices=[],chosen=null;
function netControls(){
 el('netFields').disabled=netBusy||!netLoaded;el('netPassword').disabled=el('netMode').value!=='replace';
 el('netWeather').disabled=!el('netEnabled').checked;
 el('cityFind').disabled=netBusy||cityBusy||!netOnline;el('cityQuery').disabled=cityBusy;
 el('cityChoice').disabled=netBusy||cityBusy||!choices.length;
 el('sync').disabled=netBusy||netDirty||!netOnline;
 el('weatherRefresh').disabled=netBusy||netDirty||!netOnline||!el('netWeather').checked;
}
function netState(state,fields=false){
 netOnline=state.status==='connected';
 el('netState').textContent=[nw.states[state.status]||state.status,state.ip,nw.time+': '+(nw.states[state.ntp]||state.ntp),nw.weather+': '+(nw.states[state.weather_status]||state.weather_status)].filter(Boolean).join(' · ');
 if(fields){
  netRevision=state.revision;netLoaded=true;netDirty=false;
  el('netEnabled').checked=state.enabled||state.status==='unconfigured';el('netSsid').value=state.ssid;el('netServer').value=state.server;
  el('netZone').value=state.zone;el('netWeather').checked=state.weather;
  el('netMode').value=state.ssid?'keep':'replace';el('netMode').options[0].disabled=!state.ssid;
  chosen=state.city?{name:state.city,latitude:state.latitude,longitude:state.longitude}:null;
  el('netCity').textContent=state.city||nw.none;el('netPassword').value='';
 }
 netControls();
}
async function netLoad(force=false){
 if(force&&netDirty&&!confirm(nw.discard))return;
 try{const response=await request('/network');if(!response.ok)throw Error();netState(await response.json(),force||!netLoaded);}
 catch{netOnline=false;netControls();el('netState').textContent=nw.failed;}
}
async function netPoll(){
 if(!netAlive)return;
 try{
  const response=await request('/network');if(response.status===403){netAlive=false;throw Error();}
  if(!response.ok)throw Error();netState(await response.json());
 }catch{netOnline=false;netControls();el('netState').textContent=nw.failed;}
 if(netAlive)setTimeout(netPoll,3000);
}
for(const node of el('netForm').querySelectorAll('input,select'))node.oninput=()=>{
 if(node.id==='netMode'&&node.value!=='replace')el('netPassword').value='';
 if(node.id==='netSsid'&&el('netMode').value==='keep')el('netMode').value='replace';
 if(node.id==='netEnabled'&&!node.checked)el('netWeather').checked=false;
 netDirty=true;el('netResult').textContent=nw.changed;netControls();
};
el('netForm').onsubmit=async event=>{
 event.preventDefault();if(netBusy)return;
 if(new TextEncoder().encode(el('netSsid').value).length>32||(el('netWeather').checked&&!chosen)){el('netResult').textContent=nw.invalid;return;}
 const form=new URLSearchParams({enabled:el('netEnabled').checked?'1':'0',ssid:el('netSsid').value,password:el('netPassword').value,password_mode:el('netMode').value,server:el('netServer').value,zone:el('netZone').value,weather:el('netWeather').checked?'1':'0',city:chosen?.name||'',latitude:String(chosen?.latitude||0),longitude:String(chosen?.longitude||0)});
 netBusy=true;netControls();el('netPassword').value='';
 try{
  const response=await request('/network',{method:'POST',headers:{'X-Mochi-Network-Revision':String(netRevision)},body:form.toString()});
  if(response.status===409){el('netResult').textContent=nw.conflict;return;}
  if(!response.ok){el('netResult').textContent=response.status===507?nw.storage:nw.invalid;return;}
  netState(await response.json(),true);el('netResult').textContent=nw.saved;
 }catch{el('netResult').textContent=nw.failed;}finally{netBusy=false;netControls();}
};
el('netReload').onclick=()=>netLoad(true);
el('netForget').onclick=async()=>{
 if(netBusy||!confirm(nw.forget))return;netBusy=true;netControls();
 try{
  const response=await request('/network/forget',{method:'POST',body:new URLSearchParams({revision:String(netRevision)}).toString()});
  if(!response.ok){el('netResult').textContent=response.status===409?nw.conflict:nw.storage;return;}
  netState(await response.json(),true);choices=[];el('cityChoice').replaceChildren();el('netResult').textContent=nw.forgotten;
 }catch{el('netResult').textContent=nw.failed;}finally{netBusy=false;netControls();}
};
function cityFinish(){cityBusy=false;netControls();}
async function cityPoll(attempt=0){
 try{
  const response=await request('/locations');if(!response.ok)throw Error();const state=await response.json();
  if(state.status==='loading'&&attempt<30){setTimeout(()=>cityPoll(attempt+1),1000);return;}
  choices=state.locations;el('cityChoice').replaceChildren(new Option(nw.choose,''));
  choices.forEach((v,index)=>el('cityChoice').add(new Option([v.name,v.region,v.country].filter(Boolean).join(' · '),String(index))));
  el('cityResult').textContent=state.status==='ready'?'':state.status==='empty'?nw.empty:nw.busy;
 }catch{el('cityResult').textContent=nw.failed;}finally{if(attempt>=30||choices.length)cityFinish();}
 cityFinish();
}
el('citySearch').onsubmit=async event=>{
 event.preventDefault();if(cityBusy||!netOnline)return;
 if(new TextEncoder().encode(el('cityQuery').value).length>64){el('cityResult').textContent=nw.invalid;return;}
 cityBusy=true;choices=[];netControls();el('cityResult').textContent=nw.searching;
 try{
  const response=await request('/locations',{method:'POST',body:new URLSearchParams({query:el('cityQuery').value}).toString()});
  if(response.status!==202){el('cityResult').textContent=nw.busy;cityFinish();return;}cityPoll();
 }catch{el('cityResult').textContent=nw.failed;cityFinish();}
};
el('cityChoice').onchange=()=>{
 if(el('cityChoice').value==='')return;chosen=choices[Number(el('cityChoice').value)];
 el('netCity').textContent=chosen.name;el('netWeather').checked=true;netDirty=true;el('cityResult').textContent=nw.selected;netControls();
};
for(const [id,path,action] of [['sync','/ntp','sync'],['weatherRefresh','/weather/refresh','refresh']])el(id).onclick=async()=>{
 if(netBusy||netDirty)return;netBusy=true;netControls();
 try{
  const response=await request(path,{method:'POST',body:new URLSearchParams({action}).toString()});
  el('netResult').textContent=response.status===202?nw.queued:response.status===429?nw.limited:nw.busy;netLoad();
 }catch{el('netResult').textContent=nw.failed;}finally{netBusy=false;netControls();}
};
if(/^[a-f0-9]{32}$/.test(token)){netLoad().then(()=>setTimeout(netPoll,3000));}
else{el('status').textContent=nw.scan;el('netState').textContent=nw.scan;netAlive=false;}
)JS";
