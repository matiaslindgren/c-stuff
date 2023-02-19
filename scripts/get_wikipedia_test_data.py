import argparse
import pathlib
import time

import requests

parser = argparse.ArgumentParser()
parser.add_argument("out_dir", type=pathlib.Path)
out_dir = parser.parse_args().out_dir


def article_url(lang, article):
    return f"https://{lang}.wikipedia.org/api/rest_v1/page/summary/{article}"


lang2article = {
    "ar": "%D9%85%D8%A7%D8%A1",
    "bg": "%D0%92%D0%BE%D0%B4%D0%B0",
    "cs": "Voda",
    "de": "Wasser",
    "el": "%CE%9D%CE%B5%CF%81%CF%8C",
    "fa": "%D8%A2%D8%A8",
    "fi": "Vesi",
    "fr": "Eau",
    "he": "%D7%9E%D7%99%D7%9D",
    "hi": "%E0%A4%9C%E0%A4%B2",
    "is": "Vatn",
    "ja": "%E6%B0%B4",
    "ka": "%E1%83%AC%E1%83%A7%E1%83%90%E1%83%9A%E1%83%98",
    "ki": "Mai",
    "ko": "%EB%AC%BC",
    "ku": "Av",
    "lt": "Vanduo",
    "lv": "%C5%AAdens",
    "nah": "Atl",
    "nqo": "%DF%96%DF%8C",
    "pl": "Woda",
    "pt": "%C3%81gua",
    "ru": "%D0%92%D0%BE%D0%B4%D0%B0",
    "shi": "Aman",
    "sl": "Voda",
    "szl": "Woda",
    "ta": "%E0%AE%A8%E0%AF%80%E0%AE%B0%E0%AF%8D",
    "tr": "Su",
    "uk": "%D0%92%D0%BE%D0%B4%D0%B0",
    "vep": "Vezi",
    "vi": "N%C6%B0%E1%BB%9Bc",
    "zh": "%E6%B0%B4",
}

for lang, article in lang2article.items():
    url = article_url(lang, article)
    print(f"requesting {url}")
    response = requests.get(url)
    response.raise_for_status()
    summary = response.json()
    assert "extract" in summary, (lang, article)
    content = summary["extract"] + "\n"

    out_path = out_dir.joinpath(f"water_{lang}.txt")
    print(f"writing {out_path}")
    out_path.write_text(content, encoding="utf-8")

    out_path = out_dir.joinpath(f"water_{lang}_codepoints.txt")
    print(f"writing {out_path}")
    codepoints = "\n".join(str(ord(ch)) for ch in content)
    out_path.write_text(codepoints, encoding="utf-8")

    time.sleep(0.2)
