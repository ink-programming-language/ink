// Translated from solution.cpp.

class segment
{
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  func segment(l: dynamic = 0, r: dynamic = 0) -> dynamic
  {
      self->l = cpp_construct(l);
      self->r = cpp_construct(r);
    }
}

func cmp(a: dynamic, b: dynamic) -> dynamic
{
  return ((a.l + a.r) < (b.l + b.r));
}

func overlap(a: dynamic, b: dynamic) -> dynamic
{
  if (((a.r < b.l) || (b.r < a.l)))
  {
    return 0;
  }
  return ((min(a.r, b.r) - max(a.l, b.l)) + 1);
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  cin.tie(0);
  cout.tie(0);
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, m, k);
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      read(v[i].l, v[i].r);
      v[i].l -= 1;
      v[i].r -= 1;
      i += 1;
    }
  }
  sort(v.begin(), v.end(), cmp);
  var s: dynamic = cpp_construct((m + 1), vector(n, 0));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      {
        var j: dynamic = 0;
        while ((((j + k) - 1) < n))
        {
          s[i][j] = (s[(i - 1)][j] + overlap(v[(i - 1)], segment(j, ((j + k) - 1))));
          j += 1;
        }
      }
      i += 1;
    }
  }
  var ans: dynamic = 0;
  {
    var pf: dynamic = 0;
    while ((pf <= m))
    {
      var ans1: dynamic = 0;
      {
        var i: dynamic = 0;
        while ((((i + k) - 1) < n))
        {
          ans1 = max(ans1, s[pf][i]);
          i += 1;
        }
      }
      var ans2: dynamic = 0;
      {
        var i: dynamic = 0;
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
