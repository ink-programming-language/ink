// Translated from solution.cpp.

class line
{
  var k: dynamic = cpp_uninitialized();
  var b: dynamic = cpp_uninitialized();
  func value(x: dynamic) -> dynamic
  {
      return ((k * x) + b);
    }
}

func intersect(a: dynamic, b: dynamic) -> dynamic
{
  return ((1.0 * ((b.b - a.b))) / ((a.k - b.k)));
}

class convex_hull_trick
{
  var lines: dynamic = cpp_uninitialized();
  var pts: dynamic = cpp_uninitialized();
  func convex_hull_trick() -> dynamic
  {
    }
  func add(l: dynamic) -> dynamic
  {
      if (lines.empty())
      {
        lines.push_back(l);
        return;
      }
      while (((lines.size() > 1) && (l.value(pts.back()) < lines.back().value(pts.back()))))
      {
        lines.pop_back();
        pts.pop_back();
      }
      pts.push_back(intersect(l, lines.back()));
      lines.push_back(l);
    }
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var p: dynamic = cpp_uninitialized();
  read(n, m, p);
  var d: dynamic = cpp_construct((n - 1));
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(((n - 1)))))
    {
      read(d[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((m))))
    {
      read(h[i], t[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast(((n - 1)))))
    {
      D[(i + 1)] = (D[i] + d[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((m))))
    {
      S[i] = (t[i] - D[(h[i] - 1)]);
      i += 1;
    }
  }
  iota(ind.begin(), ind.end(), 0);
  sort(ind.begin(), ind.end(), __cpp_lambda_1);
  var Ss: dynamic = cpp_construct((m + 1));
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((m))))
    {
      Ss[(i + 1)] = (Ss[i] + S[ind[i]]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((m))))
    {
      write(S[ind[i]], " ");
      i += 1;
    }
  }
  write("\n");
  {
    var ip: dynamic = 0;
    while ((ip < cpp_cast((m))))
    {
      var i: dynamic = ind[ip];
      dp[ip] = ((((ip + 1)) * S[i]) - Ss[(ip + 1)]);
      ip += 1;
    }
  }
  {
    var ip: dynamic = 0;
    while ((ip < cpp_cast((m))))
    {
      write(dp[ip], " ");
      ip += 1;
    }
  }
  write("\n");
  {
    var q: dynamic = cpp_cast((2));
    while ((q < cpp_cast(((p + 1)))))
    {
      var cht: dynamic = cpp_uninitialized();
      var j: dynamic = 0;
      {
        var ip: dynamic = 0;
        while ((ip < cpp_cast((m))))
        {
          var i: dynamic = ind[ip];
          cht.add([(-((ip + 1))), (Ss[(ip + 1)] + dp[ip])]);
          if ((ip > 0))
          {
            j = min(j, (cht.pts.size() - 1));
            while (((j < (cpp_cast(cht.pts.size()) - 1)) && (cht.pts[(j + 1)] < S[i])))
            {
              j += 1;
            }
            dp[ip] = min(dp[ip], (((S[i] * ((ip + 1))) - Ss[(ip + 1)]) + cht.lines[j].value(S[i])));
          }
          ip += 1;
        }
      }
      q += 1;
    }
  }
  write(dp[(m - 1)], "\n");
  return 0;
}

func __cpp_lambda_1(i: dynamic, j: dynamic) -> dynamic
{
  return (S[i] < S[j]);
}
