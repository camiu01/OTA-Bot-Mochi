/**
 * @file ota_page.h
 * @brief Self-contained browser upload form; no external resources.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once

static const char OTA_PAGE[] = R"HTML(<!doctype html>
<html lang="en"><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Mochi firmware update</title>
<style>
:root{color-scheme:dark;font-family:system-ui,sans-serif;background:#101615;color:#eef5f2}
body{max-width:38rem;margin:0 auto;padding:2rem 1.25rem;line-height:1.6}
h1{font-size:1.8rem;line-height:1.2}a{color:#a9e4ca;text-underline-offset:.2em}
label{display:block;margin-top:2rem;font-weight:600}input{display:block;margin:1rem 0;max-width:100%}
button{padding:.8rem 1.2rem;font:inherit;background:#a9e4ca;color:#101615;border:0;border-radius:.5rem;cursor:pointer}
button:hover{background:#c9f3df}button:disabled{opacity:.6;cursor:wait}
:focus-visible{outline:3px solid #a9e4ca;outline-offset:4px}::selection{background:#a9e4ca;color:#101615}
progress{width:100%;accent-color:#a9e4ca;margin-top:1.5rem}code{overflow-wrap:anywhere}
</style>
<main><h1>Update Mochi</h1>
<p>Download the release before connecting to Mochi Wi-Fi. Choose the
<strong>app-only</strong> image for your board, never a <code>*-full.bin</code>.</p>
<p>Expected file: <code>%FILE%</code></p>
<p>Keep USB power connected. Saved preferences are retained; unsaved changes are lost.
This session closes after five minutes.</p>
<form id="form"><label for="file">Firmware file (.bin)</label>
<input id="file" type="file" accept=".bin" required>
<button id="send">Upload firmware</button></form>
<progress id="progress" value="0" max="100" aria-label="Upload progress"></progress>
<p id="status" role="status" aria-live="polite">Ready to upload.</p></main>
<script>
const form=document.getElementById('form'),file=document.getElementById('file'),
button=document.getElementById('send'),status=document.getElementById('status'),
progress=document.getElementById('progress');
form.onsubmit=e=>{
 e.preventDefault();const f=file.files[0];if(!f)return;
 if(f.name!=='%FILE%'){status.textContent='Choose %FILE% from the release assets.';return;}
 button.disabled=true;file.disabled=true;status.textContent='Uploading. Do not disconnect power.';
 const body=new FormData();body.append('firmware',f);
 const request=new XMLHttpRequest();request.open('POST','/update');
 request.timeout=180000;request.setRequestHeader('X-Mochi-Token','%TOKEN%');
 request.setRequestHeader('X-Mochi-Size',String(f.size));
 request.upload.onprogress=e=>{if(e.lengthComputable)progress.value=e.loaded/e.total*100;};
 request.onload=()=>{status.textContent=request.responseText;if(request.status!==200){button.disabled=false;file.disabled=false;}};
 request.onerror=request.ontimeout=()=>{
  status.textContent='Connection lost. Check the Mochi display. Reopen update mode before retrying.';
  button.disabled=false;file.disabled=false;
 };
 request.send(body);
};
</script></html>)HTML";
