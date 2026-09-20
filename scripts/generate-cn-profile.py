"""Generate the recognition profile from the original local references.

The profile in ``profiles/cn-16x9.json`` is the source of truth for the CN
preset; ``core/cn_default_profile.inc`` is the same document embedded as the
fallback the executable uses when the file is missing. Both are written here.

This mode rebuilds the preset from the original workflow screenshots and the
chronological recording index, so it needs Pillow and the private local evidence;
neither is shipped. It never sends input or starts or stops a game.

The pre-0.3.1 preset was compiled into the binary as ``core/cn_templates.inc``.
That migration is done and those files are gone; recover them from git history if
the mapping ever needs auditing again.
"""
import argparse
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PROFILE_JSON = ROOT / 'profiles/cn-16x9.json'
PROFILE_INC = ROOT / 'core/cn_default_profile.inc'

def merge_variants(rules):
    """Collapse rules that differ only in template bytes into anchor variants.

    The old preset held the same page twice whenever it was sampled at two
    window pixel sizes; that is one requirement with two observations, and the
    profile expresses it as variants of a single anchor. Geometry is compared
    with a tolerance because the two copies were derived from images of
    different sizes and differ in the last floating-point digit.
    """
    def same(a, b):
        return abs(a - b) <= 1e-9

    def same_rect(a, b):
        return all(same(x, y) for x, y in zip(a, b))

    merged = []
    for rule in rules:
        for existing in merged:
            if (existing['page'] == rule['page'] and existing['label'] == rule['label'] and
                    existing['action'] == rule['action'] and
                    same_rect(existing['click'], rule['click']) and
                    same(existing['aspect'], rule['aspect']) and
                    same(existing['minimum'], rule['minimum']) and
                    len(existing['anchors']) == len(rule['anchors']) and
                    all(same_rect(a['roi'], b['roi']) and same(a['minimum'], b['minimum'])
                        for a, b in zip(existing['anchors'], rule['anchors']))):
                for target, source in zip(existing['anchors'], rule['anchors']):
                    for variant in source['variants']:
                        if variant not in target['variants']:
                            target['variants'].append(variant)
                break
        else:
            merged.append(rule)
    return merged


def build_from_images(references, index_path):
    from PIL import Image

    index = json.loads(index_path.read_text(encoding='utf-8'))
    if len(index) != 239:
        raise SystemExit('the dated preset recipe requires the original 239-frame index')

    def picture(ref):
        if isinstance(ref, tuple):
            return picture(ref[1]).resize((1024, 576), Image.Resampling.BILINEAR)
        if isinstance(ref, int):
            return Image.open(index[ref]['path']).convert('RGB')
        image = Image.open(references / ref).convert('RGB')
        return image.crop((0, 30, image.width, image.height)) if '232255' not in ref else image

    def sample(image, roi):
        w, h = image.size
        x, y, rw, rh = roi
        pixels = image.load()
        out = []
        for gy in range(16):
            for gx in range(32):
                total = 0
                for sy in range(2):
                    for sx in range(2):
                        px = min(w - 1, max(0, int((x / w + rw / w * (gx + (sx + .5) / 2) / 32) * w)))
                        py = min(h - 1, max(0, int((y / h + rh / h * (gy + (sy + .5) / 2) / 16) * h)))
                        r, g, b = pixels[px, py]
                        total += (r * 30 + g * 59 + b * 11) // 100
                out.append(total // 4)
        return out

    specs = [
        ('login_error', '登录连接超时', 55, 'restart', (0, 0),
         [(292, 56, 373, 231), (398, 342, 162, 40), (399, 392, 162, 30)], .935),
        ('home', '主界面：开始', 1, 'click', (480, 22),
         [(426, 5, 111, 29), (10, 14, 12, 16)], .91),
        ('home', '主界面：开始', 117, 'click', (480, 22),
         [(426, 5, 111, 29), (10, 14, 12, 16)], .91),
        ('play', '模式选择：更多模式', 'Pasted image 20260919232632.png', 'click', (825, 69),
         [(37, 106, 203, 65), (791, 59, 73, 23)], .92),
        ('more', '更多模式：自定游戏', 'Pasted image 20260919232706.png', 'click', (181, 325),
         [(36, 109, 170, 61), (126, 339, 111, 30)], .93),
        ('browser', '选择最稳轮换版', 238, 'click', (176, 368),
         [(22, 54, 207, 51), (44, 317, 195, 27)], .94),
        ('browser', '选择最稳轮换版', ('native', 238), 'click', (176, 368),
         [(22, 54, 207, 51), (44, 317, 195, 27)], .94),
        ('room', '房间内：继续监控', 222, 'ready', (0, 0),
         [(795, 464, 30, 31), (817, 493, 12, 20)], .89),
    ]
    rules = []
    for page, label, ref, action, click, regions, threshold in specs:
        image = picture(ref)
        w, h = image.size
        if isinstance(ref, tuple):
            regions = [(x * w / 960, y * h / 540, rw * w / 960, rh * h / 540)
                       for x, y, rw, rh in regions]
            click = (click[0] * w / 960, click[1] * h / 540)
        anchors = []
        for order, (x, y, rw, rh) in enumerate(regions):
            # The browser title and the room skill icon tolerate a lower match
            # than the unique menu labels they accompany.
            minimum = (.85 if page == 'browser' and order == 0 else
                       .84 if page == 'room' and order == 1 else threshold)
            anchors.append({
                'roi': [x / w, y / h, rw / w, rh / h],
                'minimum': minimum,
                'variants': [bytes(sample(image, (x, y, rw, rh))).hex()],
            })
        rules.append({'page': page, 'label': label, 'action': action,
                      'click': [click[0] / w, click[1] / h], 'aspect': w / h,
                      'minimum': threshold, 'anchors': anchors})
    rules = merge_variants(rules)
    launcher_image = picture('Pasted image 20260919232255.png')
    launcher = []
    for rect, click in (((57, 277, 215, 35), None),
                        ((99, launcher_image.height - 113, 92, 31), [145, -100])):
        entry = {'rect': list(rect), 'minimum': .94,
                 'variants': [bytes(sample(launcher_image, rect)).hex()]}
        if click:
            entry['click'] = click
        launcher.append(entry)
    return {
        'format': 'owh-profile',
        'version': 1,
        'name': '国服 16:9 预设',
        'rules': rules,
        'join': {
            'selection_roi': [34 / 960, 316 / 540, 8 / 960, 125 / 540],
            'selection_minimum': .08,
            'button_roi': [483 / 960, 472 / 540, 70 / 960, 21 / 540],
            'button_minimum': .45,
            'click': [520 / 960, 483 / 540],
            'label': '最稳轮换版：自动加入',
        },
        'launcher': launcher,
    }


def write(profile):
    PROFILE_JSON.parent.mkdir(parents=True, exist_ok=True)
    text = json.dumps(profile, ensure_ascii=False, indent=2)
    PROFILE_JSON.write_text(text + '\n', encoding='utf-8')
    PROFILE_INC.write_text(
        '// Generated by scripts/generate-cn-profile.py. Do not edit by hand.\n'
        '// Embedded fallback used when profiles/cn-16x9.json is missing or invalid.\n'
        'R"owhprofile(\n' + text + '\n)owhprofile"\n', encoding='utf-8')
    variants = sum(len(a['variants']) for r in profile['rules'] for a in r['anchors'])
    print(f'rules={len(profile["rules"])} anchors='
          f'{sum(len(r["anchors"]) for r in profile["rules"])} variants={variants} '
          f'launcher={len(profile["launcher"])}')
    print(f'wrote {PROFILE_JSON.relative_to(ROOT)} and {PROFILE_INC.relative_to(ROOT)}')


def build_from_json():
    """Re-emit the embedded fallback from the shipped file.

    ``owh-profile add`` edits the JSON directly, so the compiled-in copy has to
    be regenerated from it or the two would drift apart.
    """
    return json.loads(PROFILE_JSON.read_text(encoding='utf-8'))


parser = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
parser.add_argument('--from-json', action='store_true',
                    help='Only re-emit the embedded fallback from profiles/cn-16x9.json')
parser.add_argument('--references', type=Path, help='Folder with the original workflow PNGs')
parser.add_argument('--index', type=Path, help='Original chronological image-index.json')
args = parser.parse_args()
if args.from_json:
    write(build_from_json())
else:
    if not args.references or not args.index:
        parser.error('--references and --index are required, or pass --from-json')
    write(build_from_images(args.references, args.index))
