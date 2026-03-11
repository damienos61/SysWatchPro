#ifndef DASHBOARD_H
#define DASHBOARD_H

static const char* DASHBOARD_HTML =
"<!DOCTYPE html><html lang='fr'>"
"<head><meta charset='UTF-8'>"
"<meta http-equiv='refresh' content='2'>"
"<title>SysWatch Pro - Dashboard</title>"
"<style>"
"*{margin:0;padding:0;box-sizing:border-box}"
"body{background:#0a0e1a;color:#e0e0e0;font-family:'Segoe UI',monospace;padding:20px}"
"h1{color:#00d4ff;font-size:1.6em;margin-bottom:4px}"
".subtitle{color:#555;font-size:.85em;margin-bottom:20px}"
".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:16px}"
".card{background:#111827;border:1px solid #1e2a3a;border-radius:10px;padding:16px}"
".card h2{color:#00d4ff;font-size:.95em;margin-bottom:12px;text-transform:uppercase;letter-spacing:1px}"
".bar-wrap{background:#1a2030;border-radius:4px;height:18px;overflow:hidden;margin:6px 0}"
".bar{height:100%;border-radius:4px;transition:width .4s}"
".green{background:linear-gradient(90deg,#00c853,#69f0ae)}"
".yellow{background:linear-gradient(90deg,#ff8f00,#ffd54f)}"
".red{background:linear-gradient(90deg,#c62828,#ef5350)}"
".val{font-size:1.3em;font-weight:bold;color:#fff}"
".sub{color:#7a8a9a;font-size:.8em}"
"table{width:100%;border-collapse:collapse;font-size:.82em}"
"th{color:#00d4ff;text-align:left;padding:4px 8px;border-bottom:1px solid #1e2a3a}"
"td{padding:3px 8px;border-bottom:1px solid #111}"
"tr:hover td{background:#1a2030}"
".badge{display:inline-block;padding:1px 7px;border-radius:10px;font-size:.75em}"
".ok{background:#1b5e20;color:#a5d6a7}"
".warn{background:#4a3000;color:#ffcc02}"
".alert{background:#5d0000;color:#ff6060}"
".spark{display:inline-block;width:6px;height:30px;background:#1e2a3a;vertical-align:bottom;margin:1px}"
"#timestamp{color:#556;font-size:.78em;text-align:right;margin-top:16px}"
"</style></head>"
"<body>"
"<h1>SysWatch Pro <span style='color:#556;font-size:.6em'>v2.0</span></h1>"
"<div class='subtitle' id='ts'>Chargement...</div>"
"<div class='grid'>"

/* CPU card */
"<div class='card'><h2>Processeur (CPU)</h2>"
"<div class='val' id='cpu-val'>--%</div>"
"<div class='sub' id='cpu-cores'></div>"
"<div class='bar-wrap'><div class='bar' id='cpu-bar' style='width:0'></div></div>"
"<div id='cpu-spark' style='margin-top:8px'></div>"
"</div>"

/* RAM card */
"<div class='card'><h2>Memoire (RAM)</h2>"
"<div class='val' id='ram-val'>--%</div>"
"<div class='sub' id='ram-detail'></div>"
"<div class='bar-wrap'><div class='bar' id='ram-bar' style='width:0'></div></div>"
"<div id='ram-spark' style='margin-top:8px'></div>"
"</div>"

/* Anomaly card */
"<div class='card'><h2>IA Anomalies</h2>"
"<div id='anomaly-summary'></div>"
"<table><thead><tr><th>PID</th><th>Nom</th><th>Score</th><th>Niveau</th></tr></thead>"
"<tbody id='anomaly-table'></tbody></table>"
"</div>"

/* Network card */
"<div class='card'><h2>Reseau</h2>"
"<div id='net-summary'></div>"
"<table><thead><tr><th>Process</th><th>Local</th><th>Remote</th><th>Etat</th></tr></thead>"
"<tbody id='net-table'></tbody></table>"
"</div>"

/* Process card */
"<div class='card' style='grid-column:1/-1'><h2>Top Processus</h2>"
"<table><thead><tr><th>PID</th><th>Nom</th><th>CPU%</th><th>RAM(KB)</th><th>Threads</th><th>Statut</th></tr></thead>"
"<tbody id='proc-table'></tbody></table>"
"</div>"

"</div>"
"<div id='timestamp'></div>"

"<script>"
"function colorClass(p){return p<50?'green':p<80?'yellow':'red'}"
"function badge(level){"
"  const m={0:'ok',1:'warn',2:'alert',3:'alert'};"
"  const l={0:'OK',1:'WARN',2:'ALERTE',3:'CRITIQUE'};"
"  return `<span class='badge ${m[level]}'>${l[level]}</span>`"
"}"
"async function refresh(){"
"  try{"
"    const d=await fetch('/api/data').then(r=>r.json());"
"    const now=new Date().toLocaleString('fr-FR');"
"    document.getElementById('ts').textContent='Derniere mise a jour: '+now;"
"    document.getElementById('timestamp').textContent='Auto-refresh toutes les 2s';"

/* CPU */
"    const c=d.cpu;"
"    document.getElementById('cpu-val').textContent=c.usage.toFixed(1)+'%';"
"    document.getElementById('cpu-cores').textContent=c.cores+' coeurs logiques';"
"    const cb=document.getElementById('cpu-bar');"
"    cb.style.width=c.usage+'%'; cb.className='bar '+colorClass(c.usage);"
"    document.getElementById('cpu-spark').innerHTML=c.history.map(v=>{"
"      const h=Math.max(4,Math.round(v*30/100));"
"      const cl=colorClass(v);"
"      return `<span class='spark' style='height:${h}px;background:${cl==='green'?'#00c853':cl==='yellow'?'#ff8f00':'#c62828'}'></span>`"
"    }).join('');"

/* RAM */
"    const m=d.mem;"
"    document.getElementById('ram-val').textContent=m.usage.toFixed(1)+'%';"
"    document.getElementById('ram-detail').textContent=`${(m.used/1048576).toFixed(2)} GB / ${(m.total/1048576).toFixed(2)} GB`;"
"    const rb=document.getElementById('ram-bar');"
"    rb.style.width=m.usage+'%'; rb.className='bar '+colorClass(m.usage);"
"    document.getElementById('ram-spark').innerHTML=m.history.map(v=>{"
"      const h=Math.max(4,Math.round(v*30/100));"
"      const cl=colorClass(v);"
"      return `<span class='spark' style='height:${h}px;background:${cl==='green'?'#00c853':cl==='yellow'?'#ff8f00':'#c62828'}'></span>`"
"    }).join('');"

/* Anomalies */
"    const as=d.anomalies;"
"    document.getElementById('anomaly-summary').innerHTML="
"      `<span style='color:#7a8a9a;font-size:.85em'>Critique: <b style='color:#ef5350'>${as.crit}</b>"
"       &nbsp;Alerte: <b style='color:#ffd54f'>${as.alert}</b>"
"       &nbsp;Warn: <b style='color:#00d4ff'>${as.warn}</b></span><br><br>`;"
"    document.getElementById('anomaly-table').innerHTML=as.list.map(a=>"
"      `<tr><td>${a.pid}</td><td>${a.name}</td>"
"       <td><b>${a.score}</b></td><td>${badge(a.level)}</td></tr>`"
"    ).join('');"

/* Network */
"    const ns=d.network;"
"    document.getElementById('net-summary').innerHTML="
"      `<span style='color:#7a8a9a;font-size:.85em'>TCP: ${ns.tcp} &nbsp;UDP: ${ns.udp}"
"       &nbsp;Suspects: <b style='color:#ef5350'>${ns.suspects}</b></span><br><br>`;"
"    document.getElementById('net-table').innerHTML=ns.list.slice(0,12).map(c=>"
"      `<tr style='${c.suspicious?\"color:#ef5350\":\"\"}'>"
"       <td>${c.proc}</td><td>${c.local}:${c.lport}</td>"
"       <td>${c.remote}:${c.rport}</td><td>${c.state}</td></tr>`"
"    ).join('');"

/* Processes */
"    document.getElementById('proc-table').innerHTML=d.procs.slice(0,20).map(p=>"
"      `<tr style='${p.suspicious?\"color:#ef5350\":\"\"}'>"
"       <td>${p.pid}</td><td>${p.name}</td>"
"       <td>${p.cpu.toFixed(1)}%</td><td>${p.mem}</td>"
"       <td>${p.threads}</td>"
"       <td>${p.suspicious?badge(3):badge(0)}</td></tr>`"
"    ).join('');"

"  } catch(e){ console.error('refresh error',e); }"
"}"
"refresh(); setInterval(refresh,2000);"
"</script></body></html>";

#endif
