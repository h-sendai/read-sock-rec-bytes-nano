# read-sock-histo

read()で読んだバイト数のヒストグラムデータを作る。

ヒストグラムデータを作るのにGNU Scientific Library (gsl)を使ってる。
入っていなければCentOS:
```
root# yum install gsl-devel
```
Debian:
```
root# apt install libgsl-dev
```
でセットしておく。

## 使い方

```
./read-sock-histo [-b bufsize] [-B n_bin] [-t run_sec] remote_host:port

```

- -b bufsize: デフォルト 2MB
- -B n_bin: ビン数。ヒストグラム範囲は[0, 1460*n_bin)になる。
- -t run_sec: 走らせる秒数。デフォルト10秒。

ヒストグラムデータはstdoutに、ランサマリはstderrにでる。

```
./read-sock-histo remote_host:port > histo.data 2>run.log
```
