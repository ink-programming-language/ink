// Translated from solution.cpp.

func main()
{
  cin.tie(0);
  ios.sync_with_stdio(false);
  var N: dynamic;
  var M: dynamic;
  while ((((cin >> N) >> M) && N))
  {
    var s: dynamic;
    for (var x in S)
    {
      read(x);
    }
    var inf = 1e9;
    var calc = __cpp_lambda_1;
    var mi = S[0];
    if ((calc(mi) < M))
    {
      write(-1, "\n");
      continue;
    }
    var l = mi;
    var r = S[(N - 1)];
    while (((l + 1) < r))
    {
      var m = (((l + r)) / 2);
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

func __cpp_lambda_1(x: dynamic)
{
  var cnt = 0;
  var now = 1;
  while (((now + x) <= S[(N - 1)]))
  {
    var ma = -1;
    var it = upper_bound(S.begin(), S.end(), now);
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
