// Translated from solution.cpp.

func main() -> dynamic
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic = cpp_uninitialized();
  var M: dynamic = cpp_uninitialized();
  while ((((cin >> N) >> M) && N))
  {
    var s: dynamic = cpp_uninitialized();
    for (var x: dynamic in S)
    {
      read(x);
    }
    var inf: dynamic = 1e9;
    var calc: dynamic = __cpp_lambda_1;
    var mi: dynamic = S[0];
    if ((calc(mi) < M))
    {
      write(-1, "\n");
      continue;
    }
    var l: dynamic = mi;
    var r: dynamic = S[(N - 1)];
    while (((l + 1) < r))
    {
      var m: dynamic = (((l + r)) / 2);
      if ((calc(m) >= M))
      {
        l = m;
      } else
      {
        r = m;
      }
    }
    write(l, "\n");
  }
}

func __cpp_lambda_1(x: dynamic) -> dynamic
{
  var cnt: dynamic = 0;
  var now: dynamic = 1;
  while (((now + x) <= S[(N - 1)]))
  {
    var ma: dynamic = -1;
    var it: dynamic = upper_bound(S.begin(), S.end(), now);
    if (((*it) < (now + x)))
    {
      ma = max(1, (x - abs((now - (*it)))));
    }
    if ((it != S.begin()))
    {
      it -= 1;
      if (((*it) < (now + x)))
      {
        ma = max(ma, max((x - abs((now - (*it)))), 1));
      }
    }
    if ((ma == -1))
    {
      return inf;
    }
    cnt += 1;
    now += ma;
  }
  return (cnt + 1);
}
