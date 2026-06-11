<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Course Compiler & VM</title>
<style>
  :root {
    --bg: #0f1117;
    --bg2: #161b27;
    --bg3: #1e2535;
    --border: rgba(255,255,255,0.08);
    --border2: rgba(255,255,255,0.14);
    --text: #e2e8f0;
    --muted: #8892a4;
    --accent: #6c8ef7;
    --accent2: #5de0c5;
    --accent3: #f97066;
    --accent4: #fbbf24;
    --purple: #a78bfa;
    --green: #4ade80;
  }
  * { margin:0; padding:0; box-sizing:border-box; }
  body {
    font-family: 'Segoe UI', system-ui, sans-serif;
    background: var(--bg);
    color: var(--text);
    line-height: 1.7;
    font-size: 15px;
  }

  /* ── hero ── */
  .hero {
    position: relative;
    overflow: hidden;
    padding: 80px 40px 60px;
    text-align: center;
    background: linear-gradient(160deg, #0f1117 0%, #131827 60%, #0f1117 100%);
  }
  .hero-grid {
    position: absolute; inset: 0;
    background-image:
      linear-gradient(rgba(108,142,247,0.06) 1px, transparent 1px),
      linear-gradient(90deg, rgba(108,142,247,0.06) 1px, transparent 1px);
    background-size: 48px 48px;
    mask-image: radial-gradient(ellipse 80% 60% at 50% 50%, black, transparent);
  }
  .hero-glow {
    position: absolute;
    top: -120px; left: 50%; transform: translateX(-50%);
    width: 600px; height: 400px;
    background: radial-gradient(ellipse, rgba(108,142,247,0.18) 0%, transparent 65%);
    pointer-events: none;
  }
  .hero h1 {
    position: relative;
    font-size: 52px;
    font-weight: 700;
    letter-spacing: -1.5px;
    background: linear-gradient(135deg, #fff 30%, #6c8ef7 70%, #5de0c5 100%);
    -webkit-background-clip: text;
    -webkit-text-fill-color: transparent;
    background-clip: text;
    margin-bottom: 16px;
  }
  .hero-sub {
    position: relative;
    font-size: 18px;
    color: var(--muted);
    max-width: 560px;
    margin: 0 auto 32px;
  }
  .badges {
    position: relative;
    display: flex;
    gap: 10px;
    justify-content: center;
    flex-wrap: wrap;
  }
  .badge {
    display: inline-flex;
    align-items: center;
    gap: 6px;
    padding: 6px 14px;
    border-radius: 100px;
    font-size: 13px;
    font-weight: 500;
    border: 1px solid;
  }
  .badge-blue   { background: rgba(108,142,247,0.12); border-color: rgba(108,142,247,0.3); color: #93aeff; }
  .badge-teal   { background: rgba(93,224,197,0.10); border-color: rgba(93,224,197,0.25); color: #5de0c5; }
  .badge-orange { background: rgba(249,112,102,0.10); border-color: rgba(249,112,102,0.25); color: #f97066; }
  .badge-purple { background: rgba(167,139,250,0.10); border-color: rgba(167,139,250,0.25); color: #a78bfa; }
  .badge-green  { background: rgba(74,222,128,0.10); border-color: rgba(74,222,128,0.25); color: #4ade80; }

  /* ── layout ── */
  .container { max-width: 900px; margin: 0 auto; padding: 0 32px; }
  .section { padding: 56px 0 40px; }
  .section-title {
    font-size: 13px;
    font-weight: 600;
    text-transform: uppercase;
    letter-spacing: 1.5px;
    color: var(--accent);
    margin-bottom: 20px;
  }
  h2 {
    font-size: 28px;
    font-weight: 700;
    letter-spacing: -0.5px;
    margin-bottom: 24px;
    color: #fff;
  }
  h3 {
    font-size: 18px;
    font-weight: 600;
    color: #fff;
    margin-bottom: 12px;
  }
  p { color: var(--muted); margin-bottom: 16px; }
  hr { border: none; border-top: 1px solid var(--border); margin: 48px 0; }

  /* ── pipeline 3d ── */
  .pipeline {
    display: flex;
    align-items: stretch;
    gap: 0;
    margin: 32px 0;
    overflow-x: auto;
    padding-bottom: 8px;
  }
  .pipe-step {
    flex: 1;
    min-width: 110px;
    position: relative;
  }
  .pipe-inner {
    margin: 0 1px;
    border-radius: 10px;
    padding: 20px 12px 16px;
    text-align: center;
    position: relative;
    overflow: hidden;
    border: 1px solid var(--border2);
    height: 100%;
    transition: transform .2s, box-shadow .2s;
    cursor: default;
  }
  .pipe-inner:hover {
    transform: translateY(-4px);
    box-shadow: 0 16px 40px rgba(0,0,0,0.4);
  }
  .pipe-inner::before {
    content: '';
    position: absolute;
    inset: 0;
    border-radius: 10px;
    background: linear-gradient(160deg, rgba(255,255,255,0.05) 0%, transparent 60%);
    pointer-events: none;
  }
  .pipe-step:nth-child(1) .pipe-inner { background: linear-gradient(160deg,#1a1f32,#131827); }
  .pipe-step:nth-child(2) .pipe-inner { background: linear-gradient(160deg,#1a2132,#131827); }
  .pipe-step:nth-child(3) .pipe-inner { background: linear-gradient(160deg,#1f1a32,#131827); }
  .pipe-step:nth-child(4) .pipe-inner { background: linear-gradient(160deg,#1a2832,#131827); }
  .pipe-step:nth-child(5) .pipe-inner { background: linear-gradient(160deg,#1f2218,#131827); }
  .pipe-step:nth-child(6) .pipe-inner { background: linear-gradient(160deg,#2a1818,#131827); }
  .pipe-icon {
    width: 40px; height: 40px;
    border-radius: 10px;
    margin: 0 auto 10px;
    display: flex; align-items: center; justify-content: center;
    font-size: 20px;
  }
  .pipe-name { font-size: 12px; font-weight: 600; color: #fff; margin-bottom: 4px; }
  .pipe-sub { font-size: 11px; color: var(--muted); line-height: 1.4; }
  .pipe-arrow {
    position: absolute;
    right: -12px; top: 50%;
    transform: translateY(-50%);
    z-index: 2;
    font-size: 16px;
    color: var(--muted);
  }

  /* ── ISA card ── */
  .isa-card {
    background: var(--bg2);
    border: 1px solid var(--border);
    border-radius: 16px;
    padding: 28px 32px;
    position: relative;
    overflow: hidden;
  }
  .isa-card::before {
    content: '';
    position: absolute;
    top: 0; left: 0; right: 0; height: 1px;
    background: linear-gradient(90deg, transparent, rgba(108,142,247,0.4), transparent);
  }
  .bits-row {
    display: flex;
    margin: 20px 0 8px;
    border-radius: 8px;
    overflow: hidden;
    border: 1px solid var(--border2);
  }
  .bit-seg {
    padding: 10px 0;
    text-align: center;
    font-size: 12px;
    font-weight: 600;
    position: relative;
    overflow: hidden;
  }
  .bit-seg::after {
    content: '';
    position: absolute; inset: 0;
    background: linear-gradient(180deg, rgba(255,255,255,0.06) 0%, transparent 100%);
  }
  .bit-seg.b8 { flex: 8; background: rgba(108,142,247,0.2); color: #93aeff; border-right: 1px solid var(--border2); }
  .bit-seg.b4a { flex: 4; background: rgba(93,224,197,0.15); color: #5de0c5; border-right: 1px solid var(--border2); }
  .bit-seg.b4b { flex: 4; background: rgba(167,139,250,0.15); color: #c4b5fd; border-right: 1px solid var(--border2); }
  .bit-seg.b4c { flex: 4; background: rgba(249,112,102,0.12); color: #f97066; border-right: 1px solid var(--border2); }
  .bit-seg.b12 { flex: 12; background: rgba(251,191,36,0.12); color: #fbbf24; }
  .bits-labels {
    display: flex;
    font-size: 11px;
    color: var(--muted);
  }
  .bits-labels span { flex: 1; text-align: center; }
  .bits-labels .l8 { flex: 8; }
  .bits-labels .l4 { flex: 4; }
  .bits-labels .l12 { flex: 12; }

  /* ── calling convention ── */
  .call-steps {
    display: flex;
    flex-direction: column;
    gap: 2px;
    margin: 20px 0;
  }
  .call-step {
    display: flex;
    align-items: center;
    gap: 16px;
    padding: 14px 20px;
    background: var(--bg3);
    border-radius: 8px;
    border: 1px solid var(--border);
    transition: background .15s;
  }
  .call-step:hover { background: #232b3e; }
  .call-step-num {
    width: 28px; height: 28px;
    border-radius: 50%;
    display: flex; align-items: center; justify-content: center;
    font-size: 12px; font-weight: 700;
    flex-shrink: 0;
  }
  .caller .call-step-num { background: rgba(108,142,247,0.2); color: #93aeff; border: 1px solid rgba(108,142,247,0.3); }
  .callee .call-step-num { background: rgba(93,224,197,0.15); color: #5de0c5; border: 1px solid rgba(93,224,197,0.25); }
  .call-step-label { font-size: 13px; font-weight: 600; color: #fff; min-width: 130px; }
  .call-step-desc { font-size: 13px; color: var(--muted); }
  code {
    font-family: 'Cascadia Code', 'Fira Code', monospace;
    font-size: 12px;
    background: rgba(255,255,255,0.07);
    padding: 2px 7px;
    border-radius: 4px;
    color: var(--accent2);
  }

  /* ── file tree ── */
  .file-tree {
    background: var(--bg2);
    border: 1px solid var(--border);
    border-radius: 16px;
    overflow: hidden;
  }
  .file-tree-header {
    background: var(--bg3);
    padding: 12px 20px;
    font-size: 12px;
    color: var(--muted);
    border-bottom: 1px solid var(--border);
    display: flex;
    align-items: center;
    gap: 8px;
  }
  .dot { width: 10px; height: 10px; border-radius: 50%; }
  .dot.r { background: #ff5f57; }
  .dot.y { background: #febc2e; }
  .dot.g { background: #28c840; }
  .file-tree-body { padding: 16px 20px; font-family: 'Cascadia Code', monospace; font-size: 13px; line-height: 2; }
  .ft-folder { color: var(--accent4); }
  .ft-cpp { color: var(--accent); }
  .ft-h { color: var(--accent2); }
  .ft-bat { color: var(--accent3); }
  .ft-txt { color: var(--muted); }
  .ft-indent { padding-left: 20px; border-left: 1px solid var(--border); margin-left: 8px; }
  .ft-row { display: flex; align-items: center; gap: 8px; }
  .ft-icon { width: 16px; text-align: center; }

  /* ── build steps ── */
  .build-section {
    background: var(--bg2);
    border: 1px solid var(--border);
    border-radius: 16px;
    overflow: hidden;
    margin: 24px 0;
  }
  .build-header {
    padding: 16px 24px;
    background: var(--bg3);
    border-bottom: 1px solid var(--border);
    display: flex;
    align-items: center;
    gap: 10px;
    font-weight: 600;
    font-size: 14px;
  }
  .step-num-big {
    width: 28px; height: 28px;
    border-radius: 8px;
    display: flex; align-items: center; justify-content: center;
    font-size: 13px; font-weight: 700;
    background: rgba(108,142,247,0.2);
    color: #93aeff;
    border: 1px solid rgba(108,142,247,0.3);
    flex-shrink: 0;
  }
  .code-block {
    background: #0a0d14;
    padding: 20px 24px;
    font-family: 'Cascadia Code', 'Fira Code', monospace;
    font-size: 13px;
    line-height: 1.8;
    border-top: 1px solid var(--border);
    overflow-x: auto;
  }
  .cmd-prompt { color: var(--accent2); user-select: none; }
  .cmd-text { color: #c9d7f0; }
  .cmd-comment { color: var(--muted); }
  .cmd-flag { color: var(--accent4); }
  .cmd-output { color: var(--green); }
  .build-note {
    padding: 14px 24px;
    font-size: 13px;
    color: var(--muted);
    background: rgba(251,191,36,0.04);
    border-top: 1px solid rgba(251,191,36,0.1);
    display: flex;
    gap: 8px;
    align-items: flex-start;
  }
  .build-note-icon { color: var(--accent4); margin-top: 1px; flex-shrink: 0; }

  /* ── debug section ── */
  .debug-grid {
    display: grid;
    grid-template-columns: 1fr 1fr;
    gap: 16px;
    margin: 24px 0;
  }
  @media(max-width: 640px) { .debug-grid { grid-template-columns: 1fr; } }
  .debug-card {
    background: var(--bg2);
    border: 1px solid var(--border);
    border-radius: 14px;
    padding: 20px;
    position: relative;
    overflow: hidden;
    transition: transform .2s;
  }
  .debug-card:hover { transform: translateY(-2px); }
  .debug-card::before {
    content: '';
    position: absolute;
    top: 0; left: 0; right: 0; height: 2px;
    border-radius: 14px 14px 0 0;
  }
  .debug-card.blue::before { background: linear-gradient(90deg, #6c8ef7, #5de0c5); }
  .debug-card.purple::before { background: linear-gradient(90deg, #a78bfa, #6c8ef7); }
  .debug-card.green::before { background: linear-gradient(90deg, #4ade80, #5de0c5); }
  .debug-card.orange::before { background: linear-gradient(90deg, #f97066, #fbbf24); }
  .debug-card h3 { font-size: 15px; margin-bottom: 8px; }
  .debug-card p { font-size: 13px; margin-bottom: 10px; }
  .debug-card .code-block { border-radius: 8px; border: 1px solid var(--border); margin-top: 8px; padding: 12px 16px; font-size: 12px; }

  /* ── language features ── */
  .feature-grid {
    display: grid;
    grid-template-columns: repeat(auto-fit, minmax(200px, 1fr));
    gap: 12px;
    margin: 24px 0;
  }
  .feature-card {
    background: var(--bg2);
    border: 1px solid var(--border);
    border-radius: 12px;
    padding: 16px;
    transition: border-color .2s;
  }
  .feature-card:hover { border-color: var(--border2); }
  .feature-icon { font-size: 22px; margin-bottom: 8px; }
  .feature-name { font-size: 13px; font-weight: 600; color: #fff; margin-bottom: 4px; }
  .feature-desc { font-size: 12px; color: var(--muted); line-height: 1.5; }

  /* ── table ── */
  table { width: 100%; border-collapse: collapse; margin: 16px 0; }
  th { text-align: left; padding: 10px 16px; font-size: 12px; font-weight: 600; text-transform: uppercase; letter-spacing: 0.5px; color: var(--muted); border-bottom: 1px solid var(--border); }
  td { padding: 10px 16px; font-size: 13px; border-bottom: 1px solid var(--border); color: var(--text); }
  tr:last-child td { border-bottom: none; }
  tr:hover td { background: rgba(255,255,255,0.02); }

  /* ── opcode list ── */
  .opcode-grid {
    display: grid;
    grid-template-columns: repeat(auto-fill, minmax(160px, 1fr));
    gap: 8px;
    margin: 16px 0;
  }
  .opcode-item {
    background: var(--bg3);
    border: 1px solid var(--border);
    border-radius: 8px;
    padding: 10px 12px;
    font-size: 12px;
  }
  .opcode-name { font-family: monospace; color: var(--accent2); font-weight: 600; }
  .opcode-desc { color: var(--muted); margin-top: 2px; font-size: 11px; }

  /* ── examples table ── */
  .example-row td:first-child { font-family: monospace; color: var(--accent); }
  .example-row td:last-child { font-family: monospace; color: var(--green); }

  /* ── limitations ── */
  .limit-list { list-style: none; padding: 0; }
  .limit-list li {
    display: flex;
    gap: 10px;
    padding: 10px 0;
    border-bottom: 1px solid var(--border);
    font-size: 13px;
    color: var(--muted);
  }
  .limit-list li:last-child { border-bottom: none; }
  .limit-tag {
    background: rgba(249,112,102,0.12);
    border: 1px solid rgba(249,112,102,0.2);
    color: #f97066;
    border-radius: 4px;
    padding: 1px 8px;
    font-size: 11px;
    font-weight: 600;
    flex-shrink: 0;
    height: fit-content;
    margin-top: 2px;
  }

  /* ── footer ── */
  footer {
    text-align: center;
    padding: 40px;
    font-size: 13px;
    color: var(--muted);
    border-top: 1px solid var(--border);
    margin-top: 40px;
  }

  /* ── scrollbar ── */
  ::-webkit-scrollbar { width: 6px; height: 6px; }
  ::-webkit-scrollbar-track { background: transparent; }
  ::-webkit-scrollbar-thumb { background: var(--border2); border-radius: 3px; }
</style>
</head>
<body>

<!-- ═══════════════════ HERO ═══════════════════ -->
<div class="hero">
  <div class="hero-grid"></div>
  <div class="hero-glow"></div>
  <h1>Course Compiler &amp; VM</h1>
  <p class="hero-sub">A complete compiler toolchain — lexer, recursive-descent parser, code generator, binary format, and stack-based virtual machine — for a custom imperative language.</p>
  <div class="badges">
    <span class="badge badge-blue">⚙️ C++17</span>
    <span class="badge badge-teal">🧠 Custom ISA</span>
    <span class="badge badge-purple">🔢 32-bit bytecode</span>
    <span class="badge badge-orange">🖥️ MSVC / CMake</span>
    <span class="badge badge-green">✅ Windows</span>
  </div>
</div>

<div class="container">

<!-- ═══════════════════ PIPELINE ═══════════════════ -->
<div class="section">
  <p class="section-title">Architecture</p>
  <h2>Compilation Pipeline</h2>
  <p>Source code flows through six distinct phases before execution.</p>

  <div class="pipeline">
    <div class="pipe-step">
      <div class="pipe-inner">
        <div class="pipe-icon" style="background:rgba(108,142,247,0.15);">📄</div>
        <div class="pipe-name">Lexer</div>
        <div class="pipe-sub">Characters → typed token stream</div>
      </div>
      <span class="pipe-arrow">›</span>
    </div>
    <div class="pipe-step">
      <div class="pipe-inner">
        <div class="pipe-icon" style="background:rgba(93,224,197,0.12);">🌳</div>
        <div class="pipe-name">Parser</div>
        <div class="pipe-sub">Tokens → typed AST nodes</div>
      </div>
      <span class="pipe-arrow">›</span>
    </div>
    <div class="pipe-step">
      <div class="pipe-inner">
        <div class="pipe-icon" style="background:rgba(167,139,250,0.12);">🔍</div>
        <div class="pipe-name">Analysis</div>
        <div class="pipe-sub">Symbols, scopes, stack slots</div>
      </div>
      <span class="pipe-arrow">›</span>
    </div>
    <div class="pipe-step">
      <div class="pipe-inner">
        <div class="pipe-icon" style="background:rgba(251,191,36,0.12);">⚡</div>
        <div class="pipe-name">CodeGen</div>
        <div class="pipe-sub">AST → 32-bit instruction words</div>
      </div>
      <span class="pipe-arrow">›</span>
    </div>
    <div class="pipe-step">
      <div class="pipe-inner">
        <div class="pipe-icon" style="background:rgba(74,222,128,0.12);">💾</div>
        <div class="pipe-name">.exec Writer</div>
        <div class="pipe-sub">Binary format — code + data</div>
      </div>
      <span class="pipe-arrow">›</span>
    </div>
    <div class="pipe-step">
      <div class="pipe-inner">
        <div class="pipe-icon" style="background:rgba(249,112,102,0.12);">🚀</div>
        <div class="pipe-name">VM</div>
        <div class="pipe-sub">Fetch–decode–execute loop</div>
      </div>
    </div>
  </div>
</div>

<hr>

<!-- ═══════════════════ ISA ═══════════════════ -->
<div class="section">
  <p class="section-title">Instruction Set</p>
  <h2>32-bit Custom ISA</h2>
  <p>Every instruction is one 32-bit word, split into five packed fields.</p>

  <div class="isa-card">
    <div style="font-size:13px;color:var(--muted);">Instruction word layout</div>
    <div class="bits-row">
      <div class="bit-seg b8">opcode<br><span style="font-weight:400;font-size:10px;">[31:24]</span></div>
      <div class="bit-seg b4a">rd<br><span style="font-weight:400;font-size:10px;">[23:20]</span></div>
      <div class="bit-seg b4b">rs1<br><span style="font-weight:400;font-size:10px;">[19:16]</span></div>
      <div class="bit-seg b4c">rs2<br><span style="font-weight:400;font-size:10px;">[15:12]</span></div>
      <div class="bit-seg b12">imm12<br><span style="font-weight:400;font-size:10px;">[11:0]</span></div>
    </div>
    <div class="bits-labels">
      <span class="l8" style="color:#93aeff;">8 bits — opcode</span>
      <span class="l4" style="color:#5de0c5;">4 — dst reg</span>
      <span class="l4" style="color:#c4b5fd;">4 — src1</span>
      <span class="l4" style="color:#f97066;">4 — src2</span>
      <span class="l12" style="color:#fbbf24;">12 bits — signed immediate (sign-extended)</span>
    </div>
    <div style="margin-top:20px;font-size:13px;color:var(--muted);">Constants: 16 registers · 64 KB memory · 16 KB stack</div>
  </div>

  <div style="margin-top:24px;">
    <h3>Opcode reference</h3>
    <div class="opcode-grid">
      <div class="opcode-item"><div class="opcode-name">NOP</div><div class="opcode-desc">No operation</div></div>
      <div class="opcode-item"><div class="opcode-name">MOV_REG</div><div class="opcode-desc">Register copy</div></div>
      <div class="opcode-item"><div class="opcode-name">MOV_IMM</div><div class="opcode-desc">Load immediate</div></div>
      <div class="opcode-item"><div class="opcode-name">LOAD_DATA</div><div class="opcode-desc">Load from data segment</div></div>
      <div class="opcode-item"><div class="opcode-name">STORE_DATA</div><div class="opcode-desc">Store to data segment</div></div>
      <div class="opcode-item"><div class="opcode-name">LOAD_LOCAL</div><div class="opcode-desc">Load from stack frame</div></div>
      <div class="opcode-item"><div class="opcode-name">STORE_LOCAL</div><div class="opcode-desc">Store to stack frame</div></div>
      <div class="opcode-item"><div class="opcode-name">ADD / SUB</div><div class="opcode-desc">Integer arithmetic</div></div>
      <div class="opcode-item"><div class="opcode-name">MUL / DIV</div><div class="opcode-desc">Multiply / divide (÷0→0)</div></div>
      <div class="opcode-item"><div class="opcode-name">CMP</div><div class="opcode-desc">Compare two registers</div></div>
      <div class="opcode-item"><div class="opcode-name">BR_EQ…BR_GTE</div><div class="opcode-desc">Conditional branches</div></div>
      <div class="opcode-item"><div class="opcode-name">JMP</div><div class="opcode-desc">Unconditional jump</div></div>
      <div class="opcode-item"><div class="opcode-name">CALL / RET</div><div class="opcode-desc">Function call / return</div></div>
      <div class="opcode-item"><div class="opcode-name">PUSH_BP</div><div class="opcode-desc">Save base pointer</div></div>
      <div class="opcode-item"><div class="opcode-name">PUSH_NEXT_PC</div><div class="opcode-desc">Save return address</div></div>
      <div class="opcode-item"><div class="opcode-name">ALLOC_STACK</div><div class="opcode-desc">Reserve local space</div></div>
      <div class="opcode-item"><div class="opcode-name">PRINT / INPUT</div><div class="opcode-desc">Built-in I/O</div></div>
      <div class="opcode-item"><div class="opcode-name">EXIT</div><div class="opcode-desc">Halt the VM</div></div>
    </div>
  </div>

  <div style="margin-top:32px;">
    <h3>Calling convention</h3>
    <p>Frame-based: caller sets up arguments in registers, callee owns the frame.</p>
    <div class="call-steps">
      <div style="font-size:12px;font-weight:600;text-transform:uppercase;letter-spacing:1px;color:var(--accent);padding:6px 0;">Caller side</div>
      <div class="call-step caller">
        <div class="call-step-num">1</div>
        <div class="call-step-label">Load args</div>
        <div class="call-step-desc">Arguments placed in <code>R2</code>, <code>R3</code>, … (right-to-left)</div>
      </div>
      <div class="call-step caller">
        <div class="call-step-num">2</div>
        <div class="call-step-label">PUSH_BP</div>
        <div class="call-step-desc">Current base pointer pushed onto stack</div>
      </div>
      <div class="call-step caller">
        <div class="call-step-num">3</div>
        <div class="call-step-label">PUSH_NEXT_PC</div>
        <div class="call-step-desc">Return address (IP+1) pushed onto stack</div>
      </div>
      <div class="call-step caller">
        <div class="call-step-num">4</div>
        <div class="call-step-label">CALL offset</div>
        <div class="call-step-desc">IP jumps to callee; new BP set to SP+4</div>
      </div>
      <div style="font-size:12px;font-weight:600;text-transform:uppercase;letter-spacing:1px;color:var(--accent2);padding:10px 0 6px;">Callee side</div>
      <div class="call-step callee">
        <div class="call-step-num">5</div>
        <div class="call-step-label">ALLOC_STACK N</div>
        <div class="call-step-desc">SP decremented by N bytes to reserve locals</div>
      </div>
      <div class="call-step callee">
        <div class="call-step-num">6</div>
        <div class="call-step-label">… body …</div>
        <div class="call-step-desc">Function executes; locals at negative BP offsets</div>
      </div>
      <div class="call-step callee">
        <div class="call-step-num">7</div>
        <div class="call-step-label">RET rs1</div>
        <div class="call-step-desc">Result in <code>R1</code>; BP, SP, IP restored from frame</div>
      </div>
    </div>
  </div>
</div>

<hr>

<!-- ═══════════════════ FILE STRUCTURE ═══════════════════ -->
<div class="section">
  <p class="section-title">Project</p>
  <h2>File Structure</h2>

  <div class="file-tree">
    <div class="file-tree-header">
      <div class="dot r"></div><div class="dot y"></div><div class="dot g"></div>
      <span style="margin-left:4px;">course project/</span>
    </div>
    <div class="file-tree-body">
      <div class="ft-row"><span class="ft-folder">📁 src/</span></div>
      <div class="ft-indent">
        <div class="ft-row"><span class="ft-cpp">⚡ main.cpp</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— CLI: compile / run commands</span></div>
        <div class="ft-row"><span class="ft-cpp">⚡ lexer.cpp</span> <span class="ft-h">/ lexer.h</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— tokenisation</span></div>
        <div class="ft-row"><span class="ft-cpp">⚡ parser.cpp</span> <span class="ft-h">/ parser.h</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— recursive-descent AST builder</span></div>
        <div class="ft-row"><span class="ft-h">🔷 ast.h</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— all AST node types</span></div>
        <div class="ft-row"><span class="ft-h">🔷 token.h</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— TokenType enum + Token struct</span></div>
        <div class="ft-row"><span class="ft-cpp">⚡ symbols.cpp</span> <span class="ft-h">/ symbols.h</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— scoped symbol table</span></div>
        <div class="ft-row"><span class="ft-cpp">⚡ codegen.cpp</span> <span class="ft-h">/ codegen.h</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— AST → bytecode</span></div>
        <div class="ft-row"><span class="ft-h">🔷 isa.h</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— opcodes, encoding helpers</span></div>
        <div class="ft-row"><span class="ft-cpp">⚡ exec_format.cpp</span> <span class="ft-h">/ exec_format.h</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— .exec read/write</span></div>
        <div class="ft-row"><span class="ft-cpp">⚡ vm.cpp</span> <span class="ft-h">/ vm.h</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— virtual machine</span></div>
      </div>
      <div class="ft-row" style="margin-top:6px;"><span class="ft-folder">📁 examples/</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— sample .lang programs</span></div>
      <div class="ft-row" style="margin-top:6px;"><span class="ft-bat">🔧 build.bat</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— MSVC one-click build</span></div>
      <div class="ft-row"><span class="ft-txt">📋 CMakeLists.txt</span><span class="ft-txt" style="margin-left:8px;font-size:11px;">— CMake build</span></div>
    </div>
  </div>
</div>

<hr>

<!-- ═══════════════════ REQUIREMENTS ═══════════════════ -->
<div class="section">
  <p class="section-title">Setup</p>
  <h2>Requirements</h2>
  <table>
    <thead><tr><th>Requirement</th><th>Version / Notes</th></tr></thead>
    <tbody>
      <tr><td>OS</td><td>Windows (MSVC path) · any OS with CMake</td></tr>
      <tr><td>C++ Standard</td><td>C++17</td></tr>
      <tr><td>Visual Studio <span style="color:var(--muted);font-size:12px;">(Option A)</span></td><td>2022 · <em>Desktop development with C++</em> workload required</td></tr>
      <tr><td>CMake <span style="color:var(--muted);font-size:12px;">(Option B)</span></td><td>3.16 or newer</td></tr>
    </tbody>
  </table>
</div>

<hr>

<!-- ═══════════════════ BUILD ═══════════════════ -->
<div class="section">
  <p class="section-title">Build</p>
  <h2>Building from Source</h2>

  <!-- Step 1: Create build.bat -->
  <div class="build-section">
    <div class="build-header">
      <div class="step-num-big">1</div>
      Create <code>build.bat</code> in the project root
    </div>
    <div class="code-block">
<span class="cmd-comment">:: Save this file as build.bat in the course project/ folder</span>
<span class="cmd-comment">:: Adjust the path if VS is not Community edition</span>

<span class="cmd-text">@echo off</span>
<span class="cmd-text">call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"</span>

<span class="cmd-text">if not exist build mkdir build</span>

<span class="cmd-text">cl </span><span class="cmd-flag">/std:c++17 /EHsc /Zi /Od</span><span class="cmd-text"> /Fe:build\compiler.exe /Fd:build\compiler.pdb ^</span>
<span class="cmd-text">    src\main.cpp src\lexer.cpp src\parser.cpp ^</span>
<span class="cmd-text">    src\symbols.cpp src\codegen.cpp ^</span>
<span class="cmd-text">    src\exec_format.cpp src\vm.cpp</span>

<span class="cmd-text">echo Done!</span>
    </div>
    <div class="build-note">
      <span class="build-note-icon">⚠</span>
      <div>
        <strong style="color:var(--accent4);">Debug flags included:</strong>
        <code>/Zi</code> generates a <code>.pdb</code> symbol file,
        <code>/Od</code> disables optimisations so the debugger shows exact source lines.
        For a release build replace these with <code>/O2</code> and remove <code>/Zi /Fd</code>.
        <br><br>
        If your VS edition is <strong>Professional</strong> or <strong>Enterprise</strong>, change <code>Community</code> in the path accordingly.
      </div>
    </div>
  </div>

  <!-- Step 2: Run build.bat -->
  <div class="build-section">
    <div class="build-header">
      <div class="step-num-big">2</div>
      Open <strong>Command Prompt</strong> and build
    </div>
    <div class="code-block">
<span class="cmd-comment">:: Use cmd.exe — NOT PowerShell</span>
<span class="cmd-prompt">C:\&gt; </span><span class="cmd-text">cd "course project"</span>
<span class="cmd-prompt">C:\course project&gt; </span><span class="cmd-text">build.bat</span>

<span class="cmd-output">...
compiler.exe : file created</span>
    </div>
  </div>

  <!-- Step 3: Compile a program -->
  <div class="build-section">
    <div class="build-header">
      <div class="step-num-big">3</div>
      Compile a <code>.lang</code> program
    </div>
    <div class="code-block">
<span class="cmd-prompt">C:\course project&gt; </span><span class="cmd-text">build\compiler.exe compile examples\simple.lang </span><span class="cmd-flag">-o</span><span class="cmd-text"> build\simple.exec</span>
<span class="cmd-output">Compiled -> build\simple.exec  (code words=12, data words=0)</span>

<span class="cmd-comment">;; Optional: dump instruction words as hex</span>
<span class="cmd-prompt">C:\course project&gt; </span><span class="cmd-text">build\compiler.exe compile examples\simple.lang </span><span class="cmd-flag">-o</span><span class="cmd-text"> build\simple.exec </span><span class="cmd-flag">--dump</span>
    </div>
  </div>

  <!-- Step 4: Run -->
  <div class="build-section">
    <div class="build-header">
      <div class="step-num-big">4</div>
      Run the compiled program
    </div>
    <div class="code-block">
<span class="cmd-prompt">C:\course project&gt; </span><span class="cmd-text">build\compiler.exe run build\simple.exec</span>
<span class="cmd-output">17</span>

<span class="cmd-comment">;; Enable VM trace output (reserved for future use)</span>
<span class="cmd-prompt">C:\course project&gt; </span><span class="cmd-text">build\compiler.exe run build\simple.exec </span><span class="cmd-flag">--trace</span>
    </div>
  </div>

  <!-- Alternative: CMake -->
  <div style="margin-top:24px;">
    <h3>Alternative — CMake</h3>
    <div class="build-section">
      <div class="build-header" style="background:rgba(93,224,197,0.06);border-bottom-color:rgba(93,224,197,0.15);">
        <div class="step-num-big" style="background:rgba(93,224,197,0.15);color:#5de0c5;border-color:rgba(93,224,197,0.3);">B</div>
        CMake build (any compiler)
      </div>
      <div class="code-block">
<span class="cmd-text">cmake -B build -G "Visual Studio 17 2022"</span>
<span class="cmd-text">cmake --build build --config Debug</span>

<span class="cmd-comment">;; Executable is at build\Debug\course_compiler.exe</span>
<span class="cmd-text">build\Debug\course_compiler.exe compile examples\simple.lang -o build\simple.exec</span>
<span class="cmd-text">build\Debug\course_compiler.exe run build\simple.exec</span>
      </div>
    </div>
  </div>
</div>

<hr>

<!-- ═══════════════════ DEBUGGING ═══════════════════ -->
<div class="section">
  <p class="section-title">Debugging</p>
  <h2>Debugger Setup</h2>
  <p>The <code>/Zi /Od</code> flags in <code>build.bat</code> produce a full <code>.pdb</code> symbol file. Any debugger that reads PDB files works — VS, VS Code, WinDbg, RemedyBG.</p>

  <div class="debug-grid">

    <div class="debug-card blue">
      <h3>Visual Studio 2022</h3>
      <p>Open the folder, set launch args, press F5.</p>
      <div class="code-block">
1. File → Open → Folder
   → pick "course project/"

2. Right-click compiler.exe in
   Solution Explorer → Properties
   → Debugging → Command Arguments:
   compile examples\simple.lang
   -o build\simple.exec

3. Set breakpoints in any .cpp
4. Press F5
      </div>
    </div>

    <div class="debug-card purple">
      <h3>VS Code + MSVC</h3>
      <p>Add a <code>launch.json</code> in <code>.vscode/</code>.</p>
      <div class="code-block">
{
  "type": "cppvsdbg",
  "request": "launch",
  "name": "Debug compile",
  "program": "${workspaceFolder}/build/compiler.exe",
  "args": [
    "compile",
    "examples/simple.lang",
    "-o", "build/simple.exec"
  ],
  "cwd": "${workspaceFolder}",
  "stopAtEntry": false
}
      </div>
    </div>

    <div class="debug-card green">
      <h3>Debug the VM</h3>
      <p>Use a separate launch config to step through execution.</p>
      <div class="code-block">
{
  "type": "cppvsdbg",
  "request": "launch",
  "name": "Debug run",
  "program": "${workspaceFolder}/build/compiler.exe",
  "args": [
    "run",
    "build/simple.exec",
    "--trace"
  ],
  "cwd": "${workspaceFolder}"
}

// Useful breakpoints:
// vm.cpp : VirtualMachine::step()
// vm.cpp : case isa::CALL
// vm.cpp : case isa::RET
      </div>
    </div>

    <div class="debug-card orange">
      <h3>Useful Breakpoints</h3>
      <p>Key locations when tracing a bug through the pipeline.</p>
      <div class="code-block">
// Lexer output
lexer.cpp → tokenize()
  — inspect `out` vector

// Parser entry
parser.cpp → parseProgram()

// Symbol resolution
symbols.cpp → lookup()

// Instruction emit
codegen.cpp → emit(uint32_t w)

// VM step-by-step
vm.cpp → step()
  — watch ip_, sp_, bp_
  — watch regs_[1..4]
      </div>
    </div>

  </div>

  <div class="build-note" style="border-radius:12px;border:1px solid rgba(251,191,36,0.15);margin-top:8px;">
    <span class="build-note-icon">💡</span>
    <div>
      <strong style="color:var(--accent4);">Tip:</strong>
      Add <code>--dump</code> to the <code>compile</code> command to print every emitted instruction word as hex — useful for verifying codegen output before running the VM.
      Add <code>--trace</code> to <code>run</code> for future VM-level tracing support.
    </div>
  </div>
</div>

<hr>

<!-- ═══════════════════ LANGUAGE ═══════════════════ -->
<div class="section">
  <p class="section-title">Language</p>
  <h2>Language Reference</h2>

  <div style="margin-bottom:8px;">
    <p>Statements end with <code>:</code> · Blocks use <code>{ }</code> · Comments: <code>// line comment</code></p>
  </div>

  <div class="feature-grid">
    <div class="feature-card">
      <div class="feature-icon">🌐</div>
      <div class="feature-name">global</div>
      <div class="feature-desc">Whole-program scope. Lives in data segment. Declare at top level or inside a function.</div>
    </div>
    <div class="feature-card">
      <div class="feature-icon">🔁</div>
      <div class="feature-name">static</div>
      <div class="feature-desc">Persists across calls. Function-statics are mangled as <code>func::var</code>.</div>
    </div>
    <div class="feature-card">
      <div class="feature-icon">📦</div>
      <div class="feature-name">local</div>
      <div class="feature-desc">Stack-allocated. Lives only for the current function call.</div>
    </div>
    <div class="feature-card">
      <div class="feature-icon">🔀</div>
      <div class="feature-name">if / else</div>
      <div class="feature-desc">Standard conditional. <code>elif</code> reserved but not yet implemented.</div>
    </div>
    <div class="feature-card">
      <div class="feature-icon">🔄</div>
      <div class="feature-name">while / do…while</div>
      <div class="feature-desc">Pre-test and post-test loops.</div>
    </div>
    <div class="feature-card">
      <div class="feature-icon">🔢</div>
      <div class="feature-name">for</div>
      <div class="feature-desc"><code>for (init : cond : update) { }</code> — colon-separated, not semicolons.</div>
    </div>
    <div class="feature-card">
      <div class="feature-icon">🖨️</div>
      <div class="feature-name">print / input</div>
      <div class="feature-desc">Built-in I/O. <code>print(expr):</code> and <code>input(name):</code></div>
    </div>
    <div class="feature-card">
      <div class="feature-icon">↩️</div>
      <div class="feature-name">return</div>
      <div class="feature-desc">Returns a value from a function. Result lands in <code>R1</code>.</div>
    </div>
  </div>

  <div style="margin-top:24px;">
    <h3>Operator precedence</h3>
    <table>
      <thead><tr><th>Precedence</th><th>Operators</th><th>Notes</th></tr></thead>
      <tbody>
        <tr><td>High</td><td><code>* / %</code></td><td>% tokenised but not fully implemented in codegen</td></tr>
        <tr><td>Mid</td><td><code>+ -</code></td><td></td></tr>
        <tr><td>Low</td><td><code>== != &lt; &gt; &lt;= &gt;=</code></td><td>Condition contexts only — not valid in expressions</td></tr>
      </tbody>
    </table>
  </div>

  <div style="margin-top:24px;">
    <h3>Variable scope summary</h3>
    <table>
      <thead><tr><th>Keyword</th><th>Scope</th><th>Lifetime</th><th>Storage</th></tr></thead>
      <tbody>
        <tr><td><code>global</code></td><td>Whole program</td><td>Entire run</td><td>Data segment</td></tr>
        <tr><td><code>static</code> (top-level)</td><td>File</td><td>Entire run</td><td>Data segment</td></tr>
        <tr><td><code>static</code> (in function)</td><td>Function</td><td>Persists across calls</td><td>Data segment, mangled name</td></tr>
        <tr><td><code>local</code></td><td>Function</td><td>Stack frame</td><td>Stack (BP-relative)</td></tr>
        <tr><td>parameter</td><td>Function</td><td>Stack frame</td><td>Register (R2, R3, …)</td></tr>
      </tbody>
    </table>
  </div>
</div>

<hr>

<!-- ═══════════════════ EXAMPLES ═══════════════════ -->
<div class="section">
  <p class="section-title">Examples</p>
  <h2>Example Programs</h2>
  <table>
    <thead><tr><th>File</th><th>Tests</th><th>Expected output</th></tr></thead>
    <tbody class="example-row">
      <tr><td>simple.lang</td><td>Basic arithmetic, function call</td><td>17</td></tr>
      <tr><td>local_test.lang</td><td>Local variable allocation</td><td>2</td></tr>
      <tr><td>call_test.lang</td><td>Function calls with parameters</td><td>3</td></tr>
      <tr><td>while_test.lang</td><td>While loop, do-while</td><td>0 then 1</td></tr>
      <tr><td>functions_for.lang</td><td>For loop, nested function calls</td><td>14</td></tr>
      <tr><td>globals_static.lang</td><td>Globals, file-static, function-static</td><td>6, 3, 3</td></tr>
    </tbody>
  </table>

  <div class="build-section" style="margin-top:20px;">
    <div class="build-header">
      <div class="step-num-big">▶</div>
      Run all examples
    </div>
    <div class="code-block">
<span class="cmd-text">build.bat</span>

<span class="cmd-text">build\compiler.exe compile examples\simple.lang         -o build\simple.exec</span>
<span class="cmd-text">build\compiler.exe compile examples\local_test.lang     -o build\local_test.exec</span>
<span class="cmd-text">build\compiler.exe compile examples\call_test.lang      -o build\call_test.exec</span>
<span class="cmd-text">build\compiler.exe compile examples\while_test.lang     -o build\while_test.exec</span>
<span class="cmd-text">build\compiler.exe compile examples\functions_for.lang  -o build\functions_for.exec</span>
<span class="cmd-text">build\compiler.exe compile examples\globals_static.lang -o build\globals_static.exec</span>

<span class="cmd-text">for %f in (build\*.exec) do build\compiler.exe run %f</span>
    </div>
  </div>
</div>

<hr>

<!-- ═══════════════════ .exec FORMAT ═══════════════════ -->
<div class="section">
  <p class="section-title">Binary Format</p>
  <h2>.exec File Layout</h2>
  <table>
    <thead><tr><th>Section</th><th>Type ID</th><th>Content</th></tr></thead>
    <tbody>
      <tr><td>File header</td><td>—</td><td>Magic <code>0x45584B21</code> · header size · section count</td></tr>
      <tr><td>Code</td><td>1</td><td><code>uint32_t[]</code> instruction words</td></tr>
      <tr><td>Data</td><td>2</td><td><code>int32_t[]</code> initial values of globals / statics</td></tr>
      <tr><td>Symbol table</td><td>3</td><td><code>(nameHash:u32, pc:u32)</code> pairs for functions</td></tr>
    </tbody>
  </table>
</div>

<hr>

<!-- ═══════════════════ LIMITATIONS ═══════════════════ -->
<div class="section">
  <p class="section-title">Known Issues</p>
  <h2>Current Limitations</h2>
  <ul class="limit-list">
    <li><span class="limit-tag">TODO</span><span><code>elif</code> is lexed but not parsed — only <code>if / else</code> chains work.</span></li>
    <li><span class="limit-tag">TODO</span><span><code>%</code> (modulo) is tokenised but not emitted in codegen — using it will throw at runtime.</span></li>
    <li><span class="limit-tag">LIMIT</span><span><code>imm12</code> is a signed 12-bit field — very large immediates or long-range jumps will overflow silently.</span></li>
    <li><span class="limit-tag">LIMIT</span><span>Integer-only — no floats, strings, arrays, pointers, or structs.</span></li>
    <li><span class="limit-tag">LIMIT</span><span>Single-file compilation — no imports, no modules, no multi-file programs.</span></li>
    <li><span class="limit-tag">LIMIT</span><span>VM step guard is hardcoded at 500 000 steps — infinite loops will terminate with an error message.</span></li>
    <li><span class="limit-tag">LIMIT</span><span><code>--trace</code> flag is accepted but not yet implemented in the VM.</span></li>
  </ul>
</div>

</div><!-- /container -->

<footer>
  Course Compiler &amp; VM · C++17 · Custom ISA · Stack-based VM
</footer>

</body>
</html>
