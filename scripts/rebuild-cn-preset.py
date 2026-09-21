#!/usr/bin/env python3
"""Rebuild anchors and variants of the CN preset from native client frames.

Why this exists
---------------
The preset's samples were originally taken from two sources at two pixel sizes:
the helper's own dumps (1024x576, the real client) and the screen recorder's
frames (960x540, resampled). Mixing them produced rules that scored 1.0 on the
frame they were cut from and 0.88 on the live client -- close enough to look
like a tuning problem and far enough to fail.

This script samples *one* source: the helper's own dumps, which are the client's
pixel grid verbatim. It samples the same way the C++ does (see `sample()` in
core/core.hpp) so the scores it prints are the scores the helper will see.

What it will not do
-------------------
It does not invent frames. A page with no verified frames keeps whatever rule it
already had -- a rule built from a guess is worse than a rule that is known to be
stale, because the stale one at least fails visibly.
"""

import argparse
import glob
import json
import os
import struct
import sys

TW, TH = 32, 16


def load_bmp(path):
    """Read a 32-bit top-down BMP into a flat grayscale plane.

    The helper writes BGRA with a negative height (see saveFrameBmp), and the
    luma weights match `sample()` so the numbers below are comparable with the
    tool's own scan output.
    """
    d = open(path, "rb").read()
    off = struct.unpack_from("<I", d, 10)[0]
    w, h = struct.unpack_from("<ii", d, 18)
    bpp = struct.unpack_from("<H", d, 28)[0]
    if bpp != 32:
        raise ValueError(f"{path}: expected 32bpp, got {bpp}")
    rows, topdown = abs(h), h < 0
    px = bytearray(w * rows)
    for y in range(rows):
        base = off + (y if topdown else rows - 1 - y) * w * 4
        for x in range(w):
            b, g, r = d[base + x * 4], d[base + x * 4 + 1], d[base + x * 4 + 2]
            px[y * w + x] = (r * 30 + g * 59 + b * 11) // 100
    return px, w, rows


def sample(px, w, h, roi):
    """Four subsamples per cell, exactly as core/core.hpp does."""
    out = bytearray(TW * TH)
    for y in range(TH):
        for x in range(TW):
            total = 0
            for sy in range(2):
                for sx in range(2):
                    X = min(w - 1, max(0, int((roi[0] + roi[2] * (x + (sx + .5) / 2) / TW) * w)))
                    Y = min(h - 1, max(0, int((roi[1] + roi[3] * (y + (sy + .5) / 2) / TH) * h)))
                    total += px[Y * w + X]
            out[y * TW + x] = total // 4
    return out


def similarity(a, b):
    ma, mb = sum(a) / len(a), sum(b) / len(b)
    diff = sum(abs((a[i] - ma) - (b[i] - mb)) for i in range(len(a))) / len(a)
    return max(0.0, min(1.0, 1 - (diff + .20 * abs(ma - mb)) / 128.0))


def to_hex(t):
    return bytes(t).hex()


# core/cn_profile.hpp rejects an anchor with more than this many templates, and a
# rejected anchor means the whole profile fails to parse -- the helper then runs
# on the built-in fallback and recognises nothing. Hitting the cap is therefore a
# hard failure, not a warning.
MAX_VARIANTS = 8


def select_variants(candidates, cap=MAX_VARIANTS):
    """Keep the templates that cover every frame best, not the first N.

    A page captured many times yields many near-identical templates. Keeping
    them all would exceed the cap, and trimming arbitrarily can drop the one
    template that covers the darkest or brightest rendering of the page. Greedy
    max-min instead: start from the template with the best worst case, then
    repeatedly add whichever candidate raises the worst case the most.
    """
    if len(candidates) <= cap:
        return list(range(len(candidates))), 1.0
    scores = [[similarity(a, b) for b in candidates] for a in candidates]
    worst = [min(row) for row in scores]
    chosen = [max(range(len(candidates)), key=lambda i: worst[i])]
    while len(chosen) < cap:
        best, best_cover = None, None
        for i in range(len(candidates)):
            if i in chosen:
                continue
            cover = min(max(scores[i][j], scores[c][j]) for c in chosen
                        for j in range(len(candidates)))
            if best_cover is None or cover > best_cover:
                best, best_cover = i, cover
        if best is None:
            break
        chosen.append(best)
    coverage = min(max(scores[c][j] for c in chosen) for j in range(len(candidates)))
    return sorted(chosen), coverage


def contrast(t):
    m = sum(t) / len(t)
    return (sum((v - m) ** 2 for v in t) / len(t)) ** .5


# Pages are declared with the frames a human looked at and confirmed, plus the
# regions that identify the page rather than the content inside it. The second
# half of that sentence is the whole point: the browse page used to be anchored
# on a particular player-created lobby's title, so it stopped being recognised
# whenever that lobby was not listed.
PAGES = {
    "home": {
        "label": "主界面 · 开始",
        "frames": ["142818", "143141", "200146", "200227", "200257", "200328", "200401"],
        "anchors": [
            ("开始按钮", [0.4437, 0.0093, 0.1156, 0.0537]),
            ("顶部导航条", [0.020, 0.012, 0.330, 0.042]),
        ],
    },
    "browser": {
        "label": "浏览游戏 · 选择目标房间",
        "frames": ["021409", "021440", "022026", "022056", "022127", "022158", "173853"],
        "anchors": [
            # Title and tab row as one region: the title alone scores 0.92 on the
            # frame the lobby list is still generating, this scores 0.94.
            ("标题+标签行", [0.0229, 0.010, 0.2156, 0.185]),
        ],
    },
    # These two keep the region they already had -- it is the element the rule
    # acts on (the 更多模式 tab, the 自定游戏 card), so moving it would change what
    # the click means. Only the pixels are refreshed. One native frame each is
    # thin: there is no same-page variation to measure, so the threshold stays at
    # the floor and the separation below is the only thing verified.
    "play": {
        "label": "模式选择 · 更多模式",
        "frames": ["201703"],
        "anchors": [("模式选择标题", None), ("更多模式标签", None)],
    },
    "more": {
        "label": "更多模式 · 自定游戏",
        # Three renderings of the same page, and the difference between them is
        # the point: 201733/033507 have the 自定游戏 card hover-highlighted, 205003
        # has it dark because the pointer is elsewhere. The highlight follows the
        # mouse, so an anchor recorded only in the lit state fails the moment the
        # pointer moves -- which is what happened.
        "frames": ["201733", "033507", "205003"],
        "anchors": [("更多模式标题", None), ("自定游戏卡片", None)],
    },
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dump-dir", required=True, help="directory of miss-*.bmp")
    ap.add_argument("--profile", required=True, help="cn profile json to update in place")
    ap.add_argument("--out", help="write here instead of in place")
    ap.add_argument("--margin", type=float, default=.03,
                    help="how far above the worst same-page score to set the threshold")
    ap.add_argument("--floor", type=float, default=.90, help="never go below this")
    ap.add_argument("--min-gap", type=float, default=.06,
                    help="an anchor separating this page from the next best by less than this "
                         "is not an identifier; reject it")
    ap.add_argument("--assign", type=float, default=.95,
                    help="score at which an unlabelled dump joins a page's frame set")
    args = ap.parse_args()

    frames = {}
    for f in sorted(glob.glob(os.path.join(args.dump_dir, "*.bmp"))):
        frames[os.path.basename(f)[5:11]] = load_bmp(f)
    print(f"loaded {len(frames)} frames from {args.dump_dir}")

    profile = json.load(open(args.profile, encoding="utf-8"))
    by_page = {}
    for rule in profile["rules"]:
        by_page.setdefault(rule["page"], []).append(rule)

    # A stuck helper dumps the same page every 30 seconds, so the set of frames
    # that are *not* the page being rebuilt cannot be listed by hand -- it grows
    # while the script runs. Classify instead: a dump that matches a page's
    # confirmed seed is that page, and only the rest may act as counter-examples.
    # Without this the negatives get polluted by the page itself and every
    # threshold comes out impossible.
    page_frames = {p: [k for k in spec["frames"] if k in frames] for p, spec in PAGES.items()}

    def resolve_roi(page, index, spec_roi):
        """`None` means "keep the region this rule already uses"."""
        if spec_roi is not None:
            return spec_roi
        rules = by_page.get(page) or []
        if not rules or index >= len(rules[0]["anchors"]):
            return None
        return rules[0]["anchors"][index]["roi"]

    # Keep the seed ROI and the seed template apart. They are both "the seed", and
    # passing the template where sample() wants an roi silently samples a region
    # described by the template's first four bytes -- which classifies nothing and
    # looks exactly like "no frame matches".
    seeds, seed_rois = {}, {}
    for p, ks in page_frames.items():
        if not ks:
            continue
        roi = resolve_roi(p, 0, PAGES[p]["anchors"][0][1])
        if roi is None:
            print(f"  {p}: 无法确定种子区域，跳过分类")
            continue
        seed_rois[p] = roi
        seeds[p] = sample(*frames[ks[0]], roi)
    others = []
    for k in sorted(frames):
        if any(k in ks for ks in page_frames.values()):
            continue
        best = max(((similarity(sample(*frames[k], seed_rois[p]), s), p)
                    for p, s in seeds.items()), default=(0, None))
        if best[0] >= args.assign:
            page_frames[best[1]].append(k)
            print(f"  {k}: 归入 {best[1]}（与种子相似 {best[0]:.4f}）")
        else:
            others.append(k)
    for p, ks in page_frames.items():
        print(f"  {p}: {len(ks)} 帧  反例 {len(others)} 帧")

    refused = []
    for page, spec in PAGES.items():
        rules = by_page.get(page)
        if not rules:
            print(f"!! no existing rule for page {page}, skipping")
            continue
        rule = rules[0]
        print(f"\n=== {page} · {rule['label']} -> {spec['label']}")
        new_anchors = []
        old_anchors = list(rule["anchors"])
        for index, (name, roi) in enumerate(spec["anchors"]):
            if roi is None:
                # Reuse this rule's existing geometry, in order.
                if index >= len(old_anchors):
                    print(f"  {name}: 规则没有第 {index} 个锚点，跳过")
                    refused.append(f"{page}/{name}")
                    continue
                roi = old_anchors[index]["roi"]
            all_variants = [sample(*frames[k], roi) for k in page_frames[page] if k in frames]
            if not all_variants:
                print(f"  {name}: no frames, keeping old anchor")
                refused.append(f"{page}/{name}")
                continue
            keep, coverage = select_variants(all_variants)
            variants = [all_variants[i] for i in keep]
            if len(keep) < len(all_variants):
                print(f"  {name}: {len(all_variants)} 个样本 -> 保留 {len(keep)} 个"
                      f"（上限 {MAX_VARIANTS}），覆盖最差帧 {coverage:.4f}")
            # Threshold from measurement, not taste. "Same-page score" means what
            # the helper will actually compute: a frame passes when *any* kept
            # variant matches, so the worst case is the frame whose best variant
            # is lowest -- not each variant measured against variants[0].
            ons = [max(similarity(v, f) for v in variants) for f in all_variants]
            offs = sorted(((max(similarity(sample(*frames[k], roi), v) for v in variants), k)
                           for k in others), reverse=True)
            worst_on = min(ons)
            best_off = offs[0][0] if offs else 0
            gap = worst_on - best_off
            # Place the threshold in the middle of the gap rather than just under
            # the worst known frame. With an exact variant for every frame
            # collected, "just under the worst" lands at 0.97 and leaves nothing
            # for a rendering nobody has seen yet -- a different animation frame
            # of the same button would be rejected. Mid-gap buys equal headroom on
            # both sides.
            minimum = round(min(worst_on - args.margin, max(args.floor, (worst_on + best_off) / 2)), 3)
            print(f"  {name}: {len(variants)} 变体  同类最低={worst_on:.4f}  "
                  f"异类最高={best_off:.4f}({offs[0][1] if offs else '-'})  "
                  f"间隔={gap:+.4f}  对比度={contrast(variants[0]):.1f}  -> 阈值 {minimum}")
            if gap < args.min_gap:
                # An anchor that barely separates the page from another is not an
                # identifier. Keeping it means the rule fails whenever the other
                # page renders slightly darker, and the failure looks like a
                # threshold problem rather than a wrong anchor.
                print(f"  {name}: !! 间隔 {gap:+.4f} 小于 {args.min_gap}，与其他页面区分度不足，弃用该锚点")
                refused.append(f"{page}/{name}")
                continue
            if minimum >= worst_on:
                # Writing this would produce a rule that rejects the very frames
                # it was built from -- it would look fixed and recognise nothing.
                print(f"  {name}: !! 无法分离，保留旧锚点")
                refused.append(f"{page}/{name}")
                continue
            new_anchors.append({"roi": [round(v, 6) for v in roi],
                                "minimum": minimum,
                                "variants": [to_hex(v) for v in variants]})
        if not new_anchors:
            print(f"  {page}: 没有任何可用锚点，保持原样")
            continue
        rule["label"] = spec["label"]
        rule["anchors"] = new_anchors

    if refused:
        print(f"\n以下锚点无法从现有帧确定，未改动：{', '.join(refused)}")
        print("需要更多「其它页面」的样本来把它们和本页分开。")

    # The rule's own `minimum` is only a default for anchors that omit one; every
    # anchor we just wrote carries its own, so keep it consistent rather than
    # leaving a number that looks meaningful but is never read.
    for rule in profile["rules"]:
        rule["minimum"] = min(a["minimum"] for a in rule["anchors"])

    out = args.out or args.profile
    json.dump(profile, open(out, "w", encoding="utf-8"), ensure_ascii=False, indent=1)
    print(f"\nwrote {out}")
    print("regenerate the embedded fallback with:")
    print(f"  scripts/generate-cn-profile.py --from-json {out}")


if __name__ == "__main__":
    sys.exit(main())
