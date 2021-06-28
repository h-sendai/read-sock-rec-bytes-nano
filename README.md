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
./read-sock-rec-bytes remote_host:port > histo.data 2>run.log
```
