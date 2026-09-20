"""Prepare the dated 449-case historical regression manifest; requires Pillow.

Uses the same private chronological index and reference PNGs as
generate-cn-profile.py. Output images and manifest stay in the build directory.
"""
import argparse
import json
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--references', type=Path, required=True)
parser.add_argument('--index', type=Path, required=True)
parser.add_argument('--output', type=Path, default=ROOT/'build/night-analysis')
args = parser.parse_args()
idx = json.loads(args.index.read_text(encoding='utf-8'))
if len(idx) != 239:
    parser.error('This dated regression requires the original 239-frame index')
args.output.mkdir(parents=True, exist_ok=True)
rows=[]
def add(page,scope,top,scale,path): rows.append(f'{page}\t{scope}\t{top}\t{scale}\t{path}')
for i,e in enumerate(idx):
    page=1 if 51<=i<=111 else 2 if i in [1,117] else 5 if i==132 or 142<=i<=154 else 6 if i==238 else 0
    if page==0 and (5<=i<=45 or 134<=i<=141 or 155<=i<=237) and i not in (139,161,164,234,235):page='0|8'
    add(page,'game',0,1,e['path'])
    # Derived scale cases: reconstruct native 1024x576 client dimensions from
    # recorder JPEGs to expose grid-alignment problems hidden at thumbnail size.
    if page:
        p=args.output/f'replay-native-{i}.png'
        Image.open(e['path']).resize((1024,576),Image.Resampling.BILINEAR).save(p)
        add(page,'game',0,1,p)
refs=args.references
for name,page in [('Pasted image 20260919232632.png',3),('Pasted image 20260919232706.png',4)]:
    add(page,'game',30,1,refs/name)
add(7,'launcher',0,1,refs/'Pasted image 20260919232255.png')
# A selected wrong mode must not be auto-joined, even with an enabled Join button.
wrong=Image.open(idx[238]['path']).convert('RGB')
wrong.paste(wrong.crop((343,317,538,344)),(44,317))
p=args.output/'wrong-mode.png';wrong.save(p);add(0,'game',0,1,p)
# Loading/auth entry screens are covered by the original replay images; also
# remove the Battle.net Play text while retaining its blue button.
launcher=Image.open(refs/'Pasted image 20260919232255.png').convert('RGB')
from PIL import ImageDraw
ImageDraw.Draw(launcher).rectangle((99,launcher.height-113,191,launcher.height-82),fill=(0,116,224))
p=args.output/'launcher-no-play-text.png';launcher.save(p);add(0,'launcher',0,1,p)
(args.output/'replay.tsv').write_text('\n'.join(rows)+'\n',encoding='utf-8')
print('Prepared',len(rows),'replay cases')
