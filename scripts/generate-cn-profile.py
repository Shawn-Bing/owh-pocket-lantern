"""Regenerate the 2026-09-20 CN preset from the original local references.

Requires Pillow and the chronological 239-frame index produced during the
overnight investigation. The index is private local evidence, not shipped.
Neither this tool nor cn-replay sends input or starts/stops a game.
"""
import argparse
import json
from pathlib import Path
from PIL import Image
ROOT=Path(__file__).resolve().parents[1]
parser = argparse.ArgumentParser(description=__doc__)
parser.add_argument('--references', type=Path, required=True, help='Folder containing the original workflow PNGs')
parser.add_argument('--index', type=Path, required=True, help='Original chronological image-index.json (239 frames)')
parser.add_argument('--output', type=Path, default=ROOT/'build/night-analysis')
parser.add_argument('--check', action='store_true', help='Also print historical-frame matches (slower)')
args = parser.parse_args()
refs = args.references
idx = json.loads(args.index.read_text(encoding='utf-8'))
if len(idx) != 239:
    parser.error('This dated preset recipe requires the original 239-frame index')
args.output.mkdir(parents=True, exist_ok=True)
def picture(ref):
    if isinstance(ref,tuple):return picture(ref[1]).resize((1024,576),Image.Resampling.BILINEAR)
    if isinstance(ref,int):return Image.open(idx[ref]['path']).convert('RGB')
    im=Image.open(refs/ref).convert('RGB')
    return im.crop((0,30,im.width,im.height)) if '232255' not in ref else im
def sample(im,roi):
    w,h=im.size;r=[roi[0]/w,roi[1]/h,roi[2]/w,roi[3]/h];p=im.load();out=[]
    for y in range(16):
      for x in range(32):
        total=0
        for sy in range(2):
          for sx in range(2):
            px=min(w-1,max(0,int((r[0]+r[2]*(x+(sx+.5)/2)/32)*w)))
            py=min(h-1,max(0,int((r[1]+r[3]*(y+(sy+.5)/2)/16)*h)))
            a,b,c=p[px,py];total+=(a*30+b*59+c*11)//100
        out.append(total//4)
    return out
def sim(a,b):
    ma=sum(a)/len(a);mb=sum(b)/len(b)
    return max(0,1-(sum(abs((x-ma)-(y-mb)) for x,y in zip(a,b))/len(a)+.2*abs(ma-mb))/128)
specs=[
 ('LoginError','登录连接超时',55,'Restart',(0,0),[(292,56,373,231),(398,342,162,40),(399,392,162,30)],.935),
 ('Home','主界面：开始',1,'Click',(480,22),[(426,5,111,29),(10,14,12,16)],.91),
 ('Home','主界面：开始',117,'Click',(480,22),[(426,5,111,29),(10,14,12,16)],.91),
 ('Play','模式选择：更多模式','Pasted image 20260919232632.png','Click',(825,69),[(37,106,203,65),(791,59,73,23)],.92),
 ('More','更多模式：自定游戏','Pasted image 20260919232706.png','Click',(181,325),[(36,109,170,61),(126,339,111,30)],.93),
 ('Browser','选择最稳轮换版',238,'Click',(176,368),[(22,54,207,51),(44,317,195,27)],.94),
 ('Browser','选择最稳轮换版',('native',238),'Click',(176,368),[(22,54,207,51),(44,317,195,27)],.94),
 ('Room','房间内：继续监控',222,'Ready',(0,0),[(795,464,30,31),(817,493,12,20)],.89),
]
data=[]
for name,label,ref,action,click,regions,threshold in specs:
 im=picture(ref);w,h=im.size
 if isinstance(ref,tuple):
  regions=[(x*w/960,y*h/540,rw*w/960,rh*h/540) for x,y,rw,rh in regions]
  click=(click[0]*w/960,click[1]*h/540)
 data.append(dict(name=name,label=label,action=action,click=[click[0]/w,click[1]/h],aspect=w/h,
                  threshold=threshold,anchors=[dict(roi=[x/w,y/h,rw/w,rh/h],minimum=.85 if name=='Browser' and j==0 else .84 if name=='Room' and j==1 else threshold,data=sample(im,(x,y,rw,rh))) for j,(x,y,rw,rh) in enumerate(regions)]))
im=picture('Pasted image 20260919232255.png')
launcher=[]
for rect in [(57,277,215,35),(99,im.height-113,92,31)]:
 launcher.append(sample(im,rect))
lines=['// Generated from local 2026-09-19 workflow references and 2026-09-20 recording crops.',
       '// Only menu labels, logo, error text and skill icons are embedded; no player data.']
for entry in data:
 lines.append('{ CNPage::'+entry['name']+', L"'+entry['label']+'", Action::'+entry['action']+', {'+','.join(map(str,entry['click']))+'}, '+str(entry['aspect'])+', '+str(entry['threshold'])+', {')
 for a in entry['anchors']:
  lines.append('  {{'+','.join(map(str,a['roi']))+'}, '+str(a['minimum'])+', {'+','.join(map(str,a['data']))+'}},')
 lines.append('}},')
(ROOT/'core/cn_templates.inc').write_text('\n'.join(lines)+'\n',encoding='utf-8')
(ROOT/'core/cn_launcher_templates.inc').write_text('\n'.join('{'+','.join(map(str,a))+'},' for a in launcher)+'\n',encoding='utf-8')
(args.output/'profile.json').write_text(json.dumps(data),encoding='utf-8')
if not args.check:
 print('Generated CN preset and launcher templates')
 raise SystemExit(0)
for entry in data:
 hits=[]
 for i,ref in enumerate(idx):
  im=Image.open(ref['path']).convert('RGB');w,h=im.size
  if abs(w/h/entry['aspect']-1)>.035:continue
  scores=[sim(sample(im,[a['roi'][0]*w,a['roi'][1]*h,a['roi'][2]*w,a['roi'][3]*h]),a['data']) for a in entry['anchors']]
  if all(s>=a['minimum'] for s,a in zip(scores,entry['anchors'])):hits.append((i,round(min(scores),3)))
 print(entry['name'],hits)
