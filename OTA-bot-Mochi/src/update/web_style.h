/**
 * @file web_style.h
 * @brief Shared offline refinement of the existing dark mint maintenance UI.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
static const char WEB_STYLE[] = R"CSS(
html{scroll-padding-bottom:14rem}
body{padding-top:clamp(1.5rem,5vw,3.5rem)}
h1{letter-spacing:-.025em;text-wrap:balance;font-size:clamp(1.8rem,5vw,2.4rem)}
h2{letter-spacing:-.015em}p{max-width:70ch}main{min-width:0}
nav{padding:1rem 0;border-bottom:1px solid #34483f}
nav a{min-height:44px;display:inline-flex;align-items:center}
input,select,button{max-width:100%}input::placeholder{color:#b5cbbf;opacity:1}
input:focus,select:focus{border-color:#a9e4ca}button{font-weight:600}
button:disabled{opacity:.65}button:active:not(:disabled){background:#34483f}
summary:hover{color:#a9e4ca}summary::marker{color:#a9e4ca}
#network{padding-bottom:2rem;border-bottom:1px solid #34483f}
#netState{font-variant-numeric:tabular-nums;color:#b5cbbf}
#netSave{background:#a9e4ca;color:#101615;border-color:#a9e4ca}
#netForget{color:#ffc5bc;border-color:#b98279}
#cityOptions{max-height:18rem;overflow:auto;padding:.25rem 0;border-bottom:1px solid #79978a}
#cityOptions button{display:block;text-align:left;width:100%;border:0;border-radius:0;background:#182320}
#cityOptions button:hover,#cityOptions button:focus{background:#23362d}
footer{padding-bottom:max(1rem,env(safe-area-inset-bottom))}
::selection{background:#a9e4ca;color:#101615}
:root{scrollbar-color:#79978a #101615}
@media(max-width:420px){body{padding-left:1rem;padding-right:1rem}.tools{gap:.5rem}.tools button{flex:1 1 auto}nav{gap:1rem}}
@media(prefers-reduced-motion:no-preference){button,a,summary{transition:background-color .15s ease-out,color .15s ease-out}}
)CSS";
