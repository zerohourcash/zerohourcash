#!/usr/bin/env python3
"""Link a focused index-proof regression against the actual configured core."""
import pathlib,shlex,subprocess,tempfile
root=pathlib.Path(__file__).resolve().parents[3]
plan=subprocess.check_output(['make','-n','-W','bitcoind.cpp','zerohourd'],cwd=root/'src',text=True)
compile_line=next(x for x in plan.splitlines() if ' -c -o zerohourd-bitcoind.o' in x)
link_line=next(x for x in plan.splitlines() if ' --mode=link ' in x and ' -o zerohourd ' in x)
with tempfile.TemporaryDirectory(prefix='zhcash-genesis-test-') as tmp:
 obj=str(pathlib.Path(tmp)/'genesis.o');exe=str(pathlib.Path(tmp)/'genesis-test')
 compile_args=shlex.split(compile_line.split(';',1)[1].split(' -MT ',1)[0])
 subprocess.run([*compile_args,'-c',str(root/'contrib/devtools/tests/genesis_index.cpp'),'-o',obj],cwd=root/'src',check=True)
 args=shlex.split(link_line.split(';',1)[1]);args[args.index('-o')+1]=exe;args[args.index('zerohourd-bitcoind.o')]=obj
 subprocess.run(args,cwd=root/'src',check=True)
 subprocess.run([exe],check=True)
