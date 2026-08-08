// Translated from solution.cpp.

class line
{
  var k: dynamic;
  var b: dynamic;
  func value(x: dynamic)
  {
      return ((k * x) + b);
    }
}

func intersect(a: dynamic, b: dynamic)
{
  return ((1.0 * ((b.b - a.b))) / ((a.k - b.k)));
}

class convex_hull_trick
{
  var lines: dynamic;
  var pts: dynamic;
  func convex_hull_trick()
  {
    }
  func add(l: dynamic)
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

func main()
{
  ios_base.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var m: dynamic;
  var p: dynamic;
  read(n, m, p);
  var d = cpp_construct((n - 1));
  {
    var i = 0;
    while ((i < cpp_cast(((n - 1)))))
    {
      read(d[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cpp_cast((m))))
    {
      read(h[i], t[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cpp_cast(((n - 1)))))
    {
      D[(i + 1)] = (D[i] + d[i]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cpp_cast((m))))
    {
      S[i] = (t[i] - D[(h[i] - 1)]);
      i += 1;
    }
  }
  iota(ind.begin(), ind.end(), 0);
  sort(ind.begin(), ind.end(), __cpp_lambda_1);
  var Ss = cpp_construct((m + 1));
  {
    var i = 0;
    while ((i < cpp_cast((m))))
    {
      Ss[(i + 1)] = (Ss[i] + S[ind[i]]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < cpp_cast((m))))
    {
      write(S[ind[i]], " ");
      i += 1;
    }
  }
  write("\n");
  {
    var ip = 0;
    while ((ip < cpp_cast((m))))
    {
      var i = ind[ip];
      dp[ip] = ((((ip + 1)) * S[i]) - Ss[(ip + 1)]);
      ip += 1;
    }
  }
  {
    var ip = 0;
    while ((ip < cpp_cast((m))))
    {
      write(dp[ip], " ");
      ip += 1;
    }
  }
  write("\n");
  {
    var q = cpp_cast((2));
    while ((q < cpp_cast(((p + 1)))))
    {
      var cht: dynamic;
      var j = 0;
      {
        var ip = 0;
        while ((ip < cpp_cast((m))))
        {
          var i = ind[ip];
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

func __cpp_lambda_1(i: dynamic, j: dynamic)
{
  return (S[i] < S[j]);
}
