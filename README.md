# read-sock-rec-bytes

read()で読んだバイト数をメモリ上に記録し最後に出力する。
max_read_count回、あるいは一定時間経過したら終了する。

## 使い方

```
./read-sock-rec-bytes [-b bufsize] [-N max_read_count] [-t run_sec] remote_host:port

```

- -b bufsize: デフォルト 2MB
- -N max_read_count: read()する最大回数。デフォルト10^6回。
- -t run_sec: 走らせる秒数。デフォルト10秒。

データはstdoutに、ランサマリはstderrにでる。

```
./read-sock-rec-bytes remote_host:port > read.data 2>run.log
```

## 出力例

```
0.000998 14480
0.001076 28960
0.001092 23168
0.001126 20272
0.001464 344624
0.001483 46336
0.001516 52128
0.001534 20272
0.001559 30408
0.001578 20272
```

最初の数値は開始からの経過時間、2番目はread()の戻り値（読んだバイト数）（単位はバイト）。

read()に使うバッファをデフォルトの2MBにしておくと10GbEで
Linuxサーバーから読むと10秒間で150000回程度read()する。
