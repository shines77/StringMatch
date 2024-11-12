# StringMatch

## 简介

各种常见的字符串匹配算法的基准测试。

包含下列算法：

- strstr(): C 标准库自带的 strstr() 函数；
- strstr_sse42() 系列函数: 使用 SSE 4.2 的 _mm_cmpistri 指令；
- A_strstr_sse42() 系列函数: 使用 SSE 4.2 的 _mm_cmpistri 指令，并结合 bsf 指令，使用 yasm 内联汇编；
- std::search(): C++ 标准库自带的函数；
- Kmp: KMP 字符串匹配算法；
- BoyerMoore: Boyer Moore 字符串匹配算法；
- BM Tuned: 来自 [Tuned Boyer-Moore algorithm](http://www-igm.univ-mlv.fr/~lecroq/string/tunedbm.html#SECTION00195)
- Sunday: 来自 [https://blog.csdn.net/q547550831/article/details/51860017](https://blog.csdn.net/q547550831/article/details/51860017)
- Horspool: 来自 [Horspool algorithm](http://www-igm.univ-mlv.fr/~lecroq/string/node18.html#SECTION00180)
- QuickSearch: 常规的快速排序算法；
- ShiftOr: 来自 [Shift Or algorithm](http://www-igm.univ-mlv.fr/~lecroq/string/node6.html#SECTION0060)
- ShiftAnd: 由 ShiftOr 算法演变而来；
- Volnitsky: 来自 [https://github.com/ox/Volnitsky-ruby/blob/master/volnitsky.cc](https://github.com/ox/Volnitsky-ruby/blob/master/volnitsky.cc)，[原出处](http://volnitsky.com/project/str_search/index.html) 已失效。
- WordHash：来自 [https://blog.csdn.net/liangzhao_jay/article/details/8792486](https://blog.csdn.net/liangzhao_jay/article/details/8792486)
- Rabin-Karp: 来自 [Karp-Rabin algorithm](http://www-igm.univ-mlv.fr/~lecroq/string/node5.html)
- memmem, fast_strstr: 仿 C 标准库 memmem() 函数写的代码；
- strstr_glibc, strstr_glibc_old, my_strstr: 仿 glibc 库 strstr() 非 SIMD 版写的代码；
- AhoCorasick: AC 自动机算法 (未使用，因为太慢了)；

关于字符串匹配，有一个法国著名的网站：

[EXACT STRING MATCHING ALGORITHMS](http://www-igm.univ-mlv.fr/~lecroq/string/index.html)

## 在 Linux 上编译

需要先安装 yasm 汇编，执行命令：

```bash
apt install yasm
```

## 在 Windows 上编译

需要先配置和安装 yasm 汇编，可参阅：[在 VS 2010/2012/2013/2015 中集成 yasm 1.3.0](https://www.cnblogs.com/shines77/p/5656101.html)

配置完成以后，才能正常打开 `StringMatch.sln` 工程文件。

## 测试结果

测试环境：Intel i5-4210M @ 2.60GHz，Win10 64bit，VC++ 2015 。

注：这是 Windows 上的测试，在 Linux 上的测试更准确一点。

```text
  Algorithm Name           CheckSum    Rand   Preprocessing   Search Time    Full Search Time
-------------------------------------------------------------------------------------------------
  strstr()                 234210207    (7)       -----         79.236 ms       -----
  strstr_sse42()           234210207    (4)       -----         74.664 ms       -----
  strstr_sse42_v2()        234210207    (5)       -----         61.163 ms       -----
  A_strstr_sse42()         234210207    (1)       -----         51.614 ms       -----
  A_strstr_sse42_v0()      234210207    (0)       -----         57.056 ms       -----
  A_strstr_sse42_v2()      234210207    (8)       -----         60.705 ms       -----
  strstr_glibc()           234210207    (8)       -----        944.284 ms       -----
  strstr_glibc_old()       234210207    (9)       -----        342.792 ms       -----
  my_strstr()              234210207    (0)       -----        381.443 ms       -----

  memmem()                 234210207    (1)       -----        405.126 ms       -----
  fast_strstr()            234210207    (9)       -----        472.494 ms       -----

  std::search()            234210207    (6)       -----        767.148 ms       -----

  Kmp                      234210207    (7)     198.270 ms     629.297 ms     827.567 ms

  BoyerMoore               234210207    (0)     638.237 ms     477.340 ms    1115.576 ms
  BM Tuned                 234210207    (3)      37.245 ms     512.681 ms     549.926 ms
  Sunday                   234210207    (8)      94.170 ms     369.613 ms     463.783 ms
  Horspool                 234210207    (8)      76.466 ms     303.335 ms     379.801 ms
  QuickSearch              234210207    (6)      87.401 ms     291.850 ms     379.251 ms

  ShiftAnd                 234210207    (3)      96.081 ms     689.746 ms     785.827 ms
  ShiftOr                  234210207    (1)      35.372 ms     539.851 ms     575.224 ms
  WordHash                 234210207    (2)    4969.401 ms     149.882 ms    5119.283 ms
  Volnitsky                234210207    (5)    5202.787 ms     116.321 ms    5319.108 ms
  Rabin-Karp 2             234210207    (6)      29.871 ms     470.199 ms     500.070 ms
  Rabin-Karp 31            234210207    (2)      11.819 ms     924.943 ms     936.761 ms

-------------------------------------------------------------------------------------------------
```
