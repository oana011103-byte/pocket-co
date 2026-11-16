// Build-time export of the existing Cola composer Rive animations; no device I/O.
import {createServer} from 'node:http';
import {readFile,writeFile,mkdir} from 'node:fs/promises';
import {createRequire} from 'node:module';
import {fileURLToPath} from 'node:url';
import {resolve,dirname} from 'node:path';
import {createHash} from 'node:crypto';
const base=resolve(dirname(fileURLToPath(import.meta.url)),'..');
const option=name=>{const i=process.argv.indexOf(name);return i<0?undefined:process.argv[i+1];};
const require=createRequire(import.meta.url);
const {chromium}=require(option('--playwright') || resolve(base,'.local-tools/capture/node_modules/playwright'));
const runtime=resolve(option('--runtime-dir') || resolve(base,'.local-tools/rive-capture'));
const paths={'/':resolve(base,'tools/pocket-co-rive-capture.html'),'/rive.js':resolve(runtime,'rive.js'),
 '/rive.wasm':resolve(runtime,'rive.wasm'),'/perching-co.riv':resolve(base,'assets/images/composer-co/perching-co.riv')};
const server=createServer(async(req,res)=>{
 try{
  if(!paths[req.url]){res.writeHead(404);res.end();return;}
  res.setHeader('Content-Type',req.url.endsWith('.wasm')?'application/wasm':req.url.endsWith('.js')?'application/javascript':req.url==='/'?'text/html':'application/octet-stream');
  res.end(await readFile(paths[req.url]));
 }catch(e){res.writeHead(500);res.end(String(e));}
});
await new Promise(r=>server.listen(0,'127.0.0.1',r));
let browser;
try {
 browser=await chromium.launch({headless:true,...(option('--browser')?{executablePath:option('--browser')}:{})});
 const page=await browser.newPage({viewport:{width:240,height:320},deviceScaleFactor:1});
 await page.goto('http://127.0.0.1:'+server.address().port);
 const poses=[['Idle','idle',5600,80],['Thinking','thinking',1120,80],['Complete','complete',1440,80]];
 const sha=async path=>createHash('sha256').update(await readFile(path)).digest('hex');
 const manifest={source:'Cola composer-co/perching-rive/build/perching-co.riv',
  source_sha256:await sha(paths['/perching-co.riv']),runtime_js_sha256:await sha(paths['/rive.js']),
  runtime_wasm_sha256:await sha(paths['/rive.wasm']),width:128,height:110,frame_ms:80,poses:{}};
 const destination=resolve(base,'assets/images/composer-co/frames');
 await mkdir(destination,{recursive:true});
 for(const [animation,name,duration,step] of poses){
  await page.evaluate(n=>window.make(n),animation);
  const files=[];
  for(let t=0;t<duration;t+=step){
   const data=await page.evaluate(t=>capture(t/1000),t);
   const file=name+'-'+String(t/step).padStart(3,'0')+'.png';
   await writeFile(resolve(destination,file),Buffer.from(data.split(',')[1],'base64'));
   files.push(file);
  }
  manifest.poses[name]={duration_ms:duration,frames:files};
 }
 await writeFile(resolve(destination,'manifest.json'),JSON.stringify(manifest,null,2)+'\n');
 console.log('Captured original Rive animation frames.');
}finally{if(browser)await browser.close();await new Promise(r=>server.close(r));}
