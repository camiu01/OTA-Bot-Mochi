/**
 * @file settings_page.h
 * @brief Offline settings and explicitly requested network-time UI.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "network_page.h"
static const char SETTINGS_PAGE[] = R"HTML(<!doctype html>
<html lang="%LANG%"><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>Mochi settings</title>
<style>
:root{color-scheme:dark;font-family:system-ui,sans-serif;background:#101615;color:#eef5f2;accent-color:#a9e4ca;scrollbar-color:#79978a #101615}
*{box-sizing:border-box}body{max-width:44rem;margin:auto;padding:1.5rem 1.25rem 14rem;line-height:1.6}
h1{font-size:2rem;line-height:1.2;margin:0 0 1rem}h2{font-size:1.4rem;margin:2rem 0 .75rem}
p{margin:.75rem 0 1.25rem}nav{display:flex;gap:1.5rem;flex-wrap:wrap;margin:1rem 0 2rem}a{color:#a9e4ca;text-underline-offset:.25em}
button,input,select{font:inherit;min-height:44px;border:1px solid #79978a;border-radius:.5rem;color:inherit;background:#182320}
button{padding:.6rem 1rem;cursor:pointer}button:hover{background:#23362d}button:disabled{opacity:.55;cursor:default}
input,select{padding:.7rem;width:100%;caret-color:#a9e4ca}input[type=checkbox]{width:1.4rem;height:1.4rem;min-height:0;flex:none}
label{display:block;font-weight:600;margin:1rem 0 .4rem}.toggle{display:flex;gap:1rem;align-items:center;min-height:52px;font-weight:400;margin:.5rem 0}
details{border-bottom:1px solid #34483f;padding:1rem 0}summary{font-weight:650;cursor:pointer;min-height:44px;padding:.5rem 0}
fieldset{padding:0;border:0;margin:0}.tools{display:flex;gap:.75rem;flex-wrap:wrap;margin:1rem 0}
small,.note{color:#b5cbbf}#save{background:#a9e4ca;color:#101615;font-weight:650}
footer{position:fixed;bottom:0;left:0;right:0;background:#101615;border-top:1px solid #34483f;padding:1rem max(1.25rem,calc((100vw - 41.5rem)/2))}
#status{margin:.5rem 0 0;font-size:.9rem;max-width:44rem}#network{scroll-margin-top:1rem}
:focus-visible{outline:3px solid #a9e4ca;outline-offset:4px;scroll-margin-bottom:14rem}::selection{background:#a9e4ca;color:#101615}
</style>
<main><h1 id="title"></h1><p id="intro" class="note"></p>
<nav><a id="facesLink" href="/"></a><a id="ntpLink" href="#network"></a></nav>
%NETWORK_HTML%
<div class="tools"><button id="reload"></button></div>
<form id="preferences"><fieldset id="fields" disabled aria-busy="true"></fieldset></form>
<h2 id="actionsTitle"></h2><p id="actionNote" class="note"></p>
<div class="tools"><button id="calibrate" disabled></button><button id="test" disabled></button><button id="firmware" disabled></button></div>
<p id="actionStatus" role="status"></p>
</main>
<footer><button id="save" disabled></button><p id="status" tabindex="-1" role="status" aria-live="polite"></p></footer>
<script>
const token='%TOKEN%'||location.hash.slice(1),it=document.documentElement.lang==='it',el=id=>document.getElementById(id);
const w=it?{
 title:'Impostazioni di Mochi',intro:'Le stesse impostazioni del display. Salva per mantenerle al riavvio; l’autorotazione resta solo per la sessione. L’accesso web dura 5 minuti, la rete salvata rimane connessa.',
 facesLink:'Libreria facce',ntpLink:'Wi-Fi, ora e meteo',reload:'Ricarica',save:'Salva impostazioni display',loading:'Caricamento…',
 saved:'Impostazioni salvate.',changed:'Modifiche non salvate.',discard:'Scartare le modifiche non salvate?',
 failed:'Connessione non riuscita. Ricollegati al Wi-Fi di Mochi; se la sessione è scaduta, riaprila dal display.',
 conflict:'Le impostazioni sono cambiate. Ricarica prima di salvare.',
 storage:'Salvataggio fallito. Le modifiche restano in RAM: riprova Salva impostazioni.',
 invalid:'Valori non validi o faccia esclusa. Ricarica la lista e riprova.',
 actionsTitle:'Azioni del dispositivo',actionNote:'Salva prima le modifiche. Calibra solo da fermo. Il test rispetta audio disattivato e silenzioso notturno.',
 calibrate:'Calibra IMU',test:'Prova audio',firmware:'Aggiorna firmware…',
 firmwareConfirm:'Passare alla modalità firmware? La sincronizzazione NTP verrà interrotta e servirà un file app.bin compatibile.',
 saveFirst:'Salva prima le impostazioni.',calibration:['Non calibrato','Calibrazione in corso…','Calibrazione pronta','Calibrazione non riuscita'],
 calibrationSave:'Calibrazione in RAM: salvataggio non riuscito.',audio:'Audio',
}:{
 title:'Mochi settings',intro:'The same settings as the display. Save to keep them after restart; auto-rotation is session-only. Web access lasts 5 minutes; saved Wi-Fi stays connected.',
 facesLink:'Face library',ntpLink:'Wi-Fi, time and weather',reload:'Reload',save:'Save display settings',loading:'Loading…',
 saved:'Settings saved.',changed:'Unsaved changes.',discard:'Discard unsaved changes?',
 failed:'Connection failed. Rejoin Mochi Wi-Fi; reopen the session on the display if it expired.',
 conflict:'Settings changed. Reload before saving.',storage:'Save failed. Changes remain in RAM: try Save settings again.',
 invalid:'Invalid values or excluded face. Reload the list and try again.',
 actionsTitle:'Device actions',actionNote:'Save changes first. Calibrate only while parked. Sound test respects sound-off and night mute.',
 calibrate:'Calibrate IMU',test:'Test sound',firmware:'Update firmware…',
 firmwareConfirm:'Switch to firmware mode? NTP will stop and you will need a matching app.bin file.',
 saveFirst:'Save settings first.',calibration:['Not calibrated','Calibrating…','Calibration ready','Calibration failed'],
 calibrationSave:'Calibration is in RAM: storage failed.',audio:'Sound',
};
for(const id of ['title','intro','facesLink','ntpLink','reload','save','actionsTitle','actionNote','calibrate','test','firmware'])el(id).textContent=w[id];
document.title=w.title;
el('facesLink').href='/faces-ui#'+token;
let schema=[],revision='',dirty=false,busy=false,loaded=false;
function controls(value){
 busy=value;el('fields').disabled=value||!loaded;el('fields').setAttribute('aria-busy',String(value));
 el('reload').disabled=value;el('save').disabled=value||!loaded||!dirty;
 for(const id of ['calibrate','test','firmware'])el(id).disabled=value||!loaded||dirty;
}
async function request(path,options={}){
 const controller=new AbortController(),timer=setTimeout(()=>controller.abort(),10000);
 try{return await fetch(path,{...options,headers:{'X-Mochi-Token':token,...(options.method==='POST'?{'Content-Type':'application/x-www-form-urlencoded'}:{}),...options.headers},signal:controller.signal,cache:'no-store'});}
 finally{clearTimeout(timer);}
}
function actions(state){
 el('actionStatus').textContent=(state.calibration===2&&!state.calibration_saved?w.calibrationSave:w.calibration[state.calibration])+' · '+w.audio+': '+state.sound;
}
function render(state){
 const opened=new Set([...el('fields').querySelectorAll('details[open]')].map(d=>d.dataset.section));
 schema=state.fields;revision=state.revision;dirty=!state.saved;el('fields').replaceChildren();
 let section;
 for(const f of schema){
  if(!section||section.dataset.section!==f.section){
   section=document.createElement('details');section.dataset.section=f.section;section.open=opened.has(f.section)||(!opened.size&&f===schema[0]);
   const title=document.createElement('summary');title.textContent=f.section;section.append(title);el('fields').append(section);
  }
  const label=document.createElement('label');label.htmlFor='setting-'+f.key;label.textContent=f.label;
  let input=document.createElement(f.options.length?'select':'input');input.id=label.htmlFor;input.name=f.key;
  if(f.options.length){f.options.forEach((o,index)=>{const opt=new Option(o.label,String(index));opt.disabled=!o.enabled;input.add(opt);});input.value=f.value;}
  else if(f.toggle){input.type='checkbox';input.checked=Boolean(f.value);label.className='toggle';}
  else{input.type='number';input.min=f.min;input.max=f.max;input.step=1;input.value=f.value;input.required=true;}
  input.oninput=()=>{dirty=true;el('status').textContent=w.changed;controls(false);};
  if(f.toggle){label.prepend(input);section.append(label);}else section.append(label,input);
  if(f.key==='auto_rotate'){const note=document.createElement('small');note.textContent=it?'Solo sessione, come sul display.':'Session only, as on the display.';section.append(note);}
 }
 actions(state);el('status').textContent=dirty?w.changed:w.saved;
}
async function load(){
 if(busy||(dirty&&!confirm(w.discard)))return;controls(true);el('status').textContent=w.loading;
 try{const response=await request('/settings');if(!response.ok)throw Error();render(await response.json());loaded=true;}
 catch{el('status').textContent=w.failed;}finally{controls(false);}
}
el('preferences').onsubmit=event=>{event.preventDefault();save();};
async function save(){
 if(busy||!loaded||!el('preferences').reportValidity())return;controls(true);
 const form=new URLSearchParams();for(const f of schema){const input=el('setting-'+f.key);form.set(f.key,f.toggle?(input.checked?'1':'0'):input.value);}
 try{
  const response=await request('/settings',{method:'POST',headers:{'X-Mochi-Settings-Revision':revision},body:form.toString()});
  if(response.status===409){el('status').textContent=w.conflict;return;}
  if(response.status===400){el('status').textContent=w.invalid;return;}
  if(response.status!==200&&response.status!==507)throw Error();
  const next=await response.json();render(next);el('status').textContent=response.ok?w.saved:w.storage;
  if(response.ok&&Boolean(next.fields.find(f=>f.key==='language').value)!==it){location.reload();return;}
  el('status').focus({preventScroll:true});
 }catch{el('status').textContent=w.failed;}finally{controls(false);}
}
for(const id of ['calibrate','test','firmware'])el(id).onclick=async()=>{
 if(busy||dirty){el('status').textContent=w.saveFirst;return;}
 if(id==='firmware'&&!confirm(w.firmwareConfirm))return;controls(true);
 try{
  const response=await request('/settings/action',{method:'POST',body:new URLSearchParams({action:id}).toString()});
  if(!response.ok)throw Error();
  if(id==='firmware'){location.href='/#'+token;return;}actions(await response.json());
  if(id==='calibrate')setTimeout(async()=>{try{const r=await request('/settings');if(r.ok)actions(await r.json());}catch{}},5500);
 }catch{el('status').textContent=w.failed;}finally{controls(false);}
};
el('save').onclick=save;el('reload').onclick=load;
window.addEventListener('beforeunload',event=>{if(dirty||netDirty){event.preventDefault();event.returnValue='';}});
if(/^[a-f0-9]{32}$/.test(token))load();
%NETWORK_SCRIPT%
</script></html>)HTML";
