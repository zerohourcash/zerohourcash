"""macOS integration smoke test. Uses temporary regtest wallets; plays deposit MP3."""
import argparse,json,pathlib,socket,subprocess,tempfile,time,threading
parser=argparse.ArgumentParser()
parser.add_argument('app',type=pathlib.Path,help='Path to full Qt app executable')
parser.add_argument('--keep-open',action='store_true',help='Leave isolated test GUI open for inspection')
options=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[3]
app=options.app.resolve()
data=pathlib.Path(tempfile.mkdtemp(prefix='zhc-fullqt-check-'))
with socket.socket() as s:
 s.bind(('127.0.0.1',0)); port=s.getsockname()[1]
args=[f'-datadir={data}','-regtest',f'-rpcport={port}','-rpcclienttimeout=180']
log=(data/'gui.log').open('w')
p=subprocess.Popen([str(app),*args[:-1],'-server=1','-listen=0','-connect=0','-staking=0','-choosedatadir=0','-wallet=receiver','-wallet=sender','-lang=ru_RU'],stdout=log,stderr=log)
state={'data':str(data),'port':port,'pid':p.pid,'app':str(app)}
(data/'smoke-state.json').write_text(json.dumps(state))
def rpc(wallet,*words):
 r=subprocess.run([str(root/'src/zerohour-cli'),*args,f'-rpcwallet={wallet}',*map(str,words)],capture_output=True,text=True,timeout=180)
 if r.returncode: raise RuntimeError(r.stderr)
 try:return json.loads(r.stdout)
 except ValueError:return r.stdout.strip()
for _ in range(120):
 try:info=rpc('receiver','getblockchaininfo');break
 except Exception:
  if p.poll() is not None: raise RuntimeError((data/'gui.log').read_text())
  time.sleep(.5)
else:raise RuntimeError('Qt RPC startup timeout')
print('PASS: full Qt GUI started on isolated regtest',data,flush=True)
seen=set(); running=True
pattern=r'^/usr/bin/afplay -v 0.72 /.*zhcash-deposit-'
def monitor():
 while running:
  result=subprocess.run(['pgrep','-f',pattern],capture_output=True,text=True)
  seen.update(result.stdout.split());time.sleep(.05)
threading.Thread(target=monitor,daemon=True).start()
try:
 address=rpc('sender','getnewaddress')
 hashes=rpc('sender','generatetoaddress',501,address,1000000000)
 assert len(hashes)==501
 time.sleep(3)
 assert not seen, f'Mining played audio: {seen}'
 print('PASS: 501 mined blocks do not play deposit sound',flush=True)
 assert rpc('receiver','getblockchaininfo')['initialblockdownload'] is False
 receive=rpc('receiver','getnewaddress','audio-check')
 tx=rpc('sender','sendtoaddress',receive,1)
 for _ in range(100):
  if seen: break
  time.sleep(.1)
 assert len(seen)==1, f'Expected one audio process, got {seen}'
 assert rpc('receiver','gettransaction',tx)['amount']==1
 print('PASS: real incoming ZHC transfer launches exact MP3 playback once',flush=True)
 time.sleep(4)
 before=set(seen)
 rpc('sender','generatetoaddress',1,address)
 time.sleep(3)
 assert seen==before, 'Confirmation replayed audio'
 print('PASS: confirmation does not replay sound',flush=True)
 own=rpc('sender','getnewaddress')
 rpc('sender','sendtoaddress',own,1)
 time.sleep(3)
 assert seen==before, 'Self-send played audio'
 print('PASS: self-transfer does not play sound',flush=True)
 sig=rpc('receiver','signmessage',receive,'ZHC visual smoke test')
 assert rpc('receiver','verifymessage',receive,sig,'ZHC visual smoke test') is True
 backup=data/'receiver-backup.dat';rpc('receiver','backupwallet',str(backup));assert backup.stat().st_size>0
 print('PASS: address generation, signing, verification and backup',flush=True)
 state.update(address=receive,tx=tx,sound_pids=list(seen),height=502)
 (data/'smoke-state.json').write_text(json.dumps(state))
 print('PASS: smoke test complete; test data at',data,flush=True)
finally:
 running=False
 if not options.keep_open:
  try: rpc('receiver','stop')
  finally: p.wait(timeout=60)
