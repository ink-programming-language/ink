// Translated from solution.cpp.

class segment
{
  var l: dynamic;
  var r: dynamic;
  func segment(l: dynamic = 0, r: dynamic = 0)
  {
      this->l = cpp_construct(l);
      this->r = cpp_construct(r);
    }
}

func cmp(a: dynamic, b: dynamic)
{
  return ((a.l + a.r) < (b.l + b.r));
}

func overlap(a: dynamic, b: dynamic)
{
  if (((a.r < b.l) || (b.r < a.l)))
  {
    return 0;
  }
  return ((min(a.r, b.r) - max(a.l, b.l)) + 1);
}

func main()
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic;
  var m: dynamic;
  var k: dynamic;
  read(n, m, k);
  {
    var i = 0;
    while ((i < m))
    {
      read(v[i].l, v[i].r);
      v[i].l -= 1;
      v[i].r -= 1;
      i += 1;
    }
  }
  sort(v.begin(), v.end(), cmp);
  var s = cpp_construct((m + 1), vector(n, 0));
  {
    var i = 1;
    while ((i <= m))
    {
      {
        var j = 0;
        while ((((j + k) - 1) < n))
        {
          s[i][j] = (s[(i - 1)][j] + overlap(v[(i - 1)], segment(j, ((j + k) - 1))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans = 0;
  {
    var pf = 0;
    while ((pf <= m))
    {
      var ans1 = 0;
      {
        var i = 0;
        while ((((i + k) - 1) < n))
        {
          ans1 = max(ans1, s[pf][i]);
          i += 1;
        }
      }
      var ans2 = 0;
      {
        var i = 0;
        while ((((i + k) - 1) < n))
        {
          ans2 = max(ans2, (s[m][i] - s[pf][i]));
          i += 1;
        }
      }
      ans = max(ans, (ans1 + ans2));
      pf += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
