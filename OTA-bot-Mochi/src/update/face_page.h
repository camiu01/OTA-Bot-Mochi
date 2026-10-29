/**
 * @file face_page.h
 * @brief Offline, accessible face allow-list editor; no external fonts, scripts or images.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
static const char FACE_PAGE[] = R"HTML(<!doctype html>
<html lang="%LANG%"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Mochi · Face library</title>
<style>
:root{color-scheme:dark;font-family:system-ui,sans-serif;background:#101615;color:#eef5f2;accent-color:#a9e4ca}
*{box-sizing:border-box}body{max-width:44rem;margin:auto;padding:1.5rem 1.25rem 14rem;line-height:1.6}
h1{font-size:2rem;line-height:1.2;margin:0 0 1rem}p{margin:.75rem 0 1.25rem}
label[for=search]{display:block;font-weight:600}input[type=search]{width:100%;font:inherit;padding:.8rem;background:#182320;color:inherit;border:1px solid #79978a;border-radius:.5rem;caret-color:#a9e4ca}
input[type=checkbox]{width:1.4rem;height:1.4rem;flex:none}
button{font:inherit;min-height:44px;padding:.6rem 1rem;border:1px solid #79978a;border-radius:.5rem;background:transparent;color:inherit;cursor:pointer}
button:hover{background:#23362d}button:disabled{opacity:.55;cursor:default}
#save{background:#a9e4ca;border-color:#a9e4ca;color:#101615;font-weight:650}#save:hover{background:#c9f3df}
.tools{display:flex;gap:.75rem;flex-wrap:wrap;margin:1rem 0}.face{display:flex;align-items:center;gap:1rem;min-height:60px;padding:.6rem .25rem;border-bottom:1px solid #34483f;cursor:pointer}
.face[hidden]{display:none}.face span{overflow-wrap:anywhere}.face small{display:block;color:#b5cbbf;font-size:.8rem}
#count{font-variant-numeric:tabular-nums;color:#b5cbbf}#empty{padding:1rem 0}
footer{position:fixed;bottom:0;left:0;right:0;background:#101615;border-top:1px solid #34483f;padding:1rem max(1.25rem,calc((100vw - 41.5rem)/2))}
#status{margin:.5rem 0 0;max-width:44rem;font-size:.9rem}
:focus-visible{outline:3px solid #a9e4ca;outline-offset:4px;scroll-margin:14rem}::selection{background:#a9e4ca;color:#101615}
@media(prefers-reduced-motion:no-preference){button{transition:background .15s}}
</style>
<main><h1 id="title">Face library</h1><p><a href="/settings-ui" id="settingsLink" style="color:#a9e4ca;text-underline-offset:.25em"></a></p><p id="intro"></p>
<label id="searchLabel" for="search">Search faces</label>
<input id="search" type="search" autocomplete="off" disabled>
<div class="tools"><button id="all" disabled>Enable all</button><button id="reload">Reload</button></div>
<p id="count" aria-live="polite"></p><div id="faces" aria-busy="true"></div>
<p id="empty" hidden></p></main>
<footer><button id="save" disabled>Save faces</button><p id="status" role="status" aria-live="polite"></p></footer>
<script>
const token='%TOKEN%'||location.hash.slice(1),it=document.documentElement.lang==='it';
const words=it?{
 title:'Le facce di Mochi',intro:'Scegli quali facce mostrare, anche per tocco, movimento e saluto. Tienine almeno una attiva. La sessione Wi-Fi dura 5 minuti.',
 search:'Cerca una faccia',all:'Attiva tutte',reload:'Ricarica',save:'Salva facce',loading:'Caricamento…',
 saved:'Facce salvate su Mochi.',changed:'Modifiche non salvate.',one:'Devi tenere almeno una faccia attiva.',
 failed:'Connessione non riuscita. Controlla il Wi-Fi e riapri la libreria sul display se la sessione è scaduta.',
 conflict:'La lista è cambiata in un altro browser. Ricarica prima di salvare.',
 storage:'Salvataggio fallito. Le modifiche sono attive solo fino al riavvio. Riprova Salva facce.',
 empty:'Nessuna faccia trovata.',count:(a,b)=>`${a} di ${b} attive`,discard:'Scartare le modifiche non salvate?'
}:{
 title:"Mochi's face library",intro:'Choose which faces can appear, including touch, motion and greetings. Keep at least one enabled. This Wi-Fi session lasts 5 minutes.',
 search:'Search faces',all:'Enable all',reload:'Reload',save:'Save faces',loading:'Loading…',
 saved:'Faces saved on Mochi.',changed:'Unsaved changes.',one:'Keep at least one face enabled.',
 failed:'Connection failed. Check Wi-Fi and reopen the library on the display if the session expired.',
 conflict:'Another browser changed this list. Reload before saving.',
 storage:'Save failed. Changes are active only until restart. Try Save faces again.',
 empty:'No matching faces.',count:(a,b)=>`${a} of ${b} enabled`,discard:'Discard unsaved changes?'
};
const el=id=>document.getElementById(id),list=el('faces'),statusNode=el('status');
document.title=words.title;
el('settingsLink').textContent=it?'Impostazioni, Wi-Fi e meteo':'Settings, Wi-Fi and weather';
el('settingsLink').href='/settings-ui#'+token;
let names=[],labels=[],enabled=[],revision=0,dirty=false,busy=false,loaded=false;
for(const [id,key] of [['title','title'],['searchLabel','search'],['all','all'],['reload','reload'],['save','save'],['intro','intro'],['empty','empty']])el(id).textContent=words[key];
function controls(value){
 busy=value;list.setAttribute('aria-busy',String(value));
 for(const input of list.querySelectorAll('input'))input.disabled=value;
 el('search').disabled=value||!loaded;el('all').disabled=value||!loaded;
 el('reload').disabled=value;el('save').disabled=value||!loaded||!dirty;
}
function count(){el('count').textContent=words.count(enabled.filter(Boolean).length,names.length);}
function changed(){dirty=true;statusNode.textContent=words.changed;count();controls(false);}
function render(){
 list.replaceChildren();
 names.forEach((name,id)=>{
  const row=document.createElement('label'),check=document.createElement('input'),text=document.createElement('span'),canonical=document.createElement('small');
  row.className='face';row.dataset.search=(name+' '+labels[id]).toLowerCase();
  check.type='checkbox';check.checked=enabled[id];
  check.onchange=()=>{
   if(!check.checked&&enabled.filter(Boolean).length===1){check.checked=true;statusNode.textContent=words.one;return;}
   enabled[id]=check.checked;changed();
  };
  text.textContent=labels[id];canonical.textContent=name;text.append(canonical);row.append(check,text);list.append(row);
 });
 filter();count();
}
function filter(){
 const query=el('search').value.trim().toLowerCase();let visible=0;
 for(const row of list.children){row.hidden=!row.dataset.search.includes(query);if(!row.hidden)visible++;}
 el('empty').hidden=!loaded||visible!==0;
}
async function request(options={}){
 const controller=new AbortController(),timer=setTimeout(()=>controller.abort(),10000);
 try{return await fetch('/faces',{...options,headers:{'X-Mochi-Token':token,...options.headers},signal:controller.signal,cache:'no-store'});}
 finally{clearTimeout(timer);}
}
async function load(){
 if(busy||(dirty&&!confirm(words.discard)))return;
 controls(true);statusNode.textContent=words.loading;
 try{
  const response=await request();if(!response.ok)throw Error();
  const state=await response.json();names=state.names;labels=state.labels;revision=state.revision;
  enabled=names.map((_,id)=>(parseInt(state.enabled.slice((id>>3)*2,(id>>3)*2+2),16)&(1<<(id%8)))!==0);
  loaded=true;dirty=!state.saved;render();statusNode.textContent=dirty?words.changed:words.saved;
 }catch{statusNode.textContent=words.failed;}finally{controls(false);}
}
el('save').onclick=async()=>{
 if(busy||!loaded)return;controls(true);
 const bytes=new Uint8Array(Math.ceil(names.length/8));enabled.forEach((on,id)=>{if(on)bytes[id>>3]|=1<<(id%8);});
 const body=Array.from(bytes,v=>v.toString(16).padStart(2,'0')).join('');
 try{
  const response=await request({method:'POST',headers:{'Content-Type':'text/plain','X-Mochi-Faces-Revision':String(revision)},body});
  if(response.status===409){statusNode.textContent=words.conflict;return;}
  if(response.status!==200&&response.status!==507)throw Error();
  const state=await response.json();revision=state.revision;dirty=!state.saved;
  statusNode.textContent=response.ok?words.saved:words.storage;
 }catch{statusNode.textContent=words.failed;}finally{controls(false);}
};
el('all').onclick=()=>{enabled.fill(true);render();changed();};
el('reload').onclick=load;el('search').oninput=filter;
window.addEventListener('beforeunload',event=>{if(dirty){event.preventDefault();event.returnValue='';}});
if(/^[a-f0-9]{32}$/.test(token))load();
else statusNode.textContent=it?'Scansiona il QR sul display di Mochi per autorizzare questa pagina.':'Scan the QR on Mochi to authorize this page.';
</script></html>)HTML";
