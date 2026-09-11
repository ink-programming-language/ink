// Translated from solution.cpp.

var maxn: dynamic = 35;

var maxk: dynamic = 205;

var p: dynamic = cpp_array(maxn);

var ans: dynamic = cpp_array(maxk, maxn, maxn);

func to(x: dynamic, l: dynamic, r: dynamic) -> dynamic
{
  if (((x < l) || (x > r)))
  {
    return x;
  }
  return ((l + r) - x);
}

func solve(n: dynamic, i: dynamic, j: dynamic, k: dynamic) -> dynamic
{
  if ((k == 0))
  {
    if ((p[i] > p[j]))
    {
      return 1.0;
    } else
    {
      return 0.0;
    }
  }
  if ((ans[i][j][k] >= 0))
  {
    return ans[i][j][k];
  }
  var koef: dynamic = (2.0 / ((n * ((n + 1)))));
  var ret: dynamic = 0.0;
  {
    var l: dynamic = 1;
    while ((l <= n))
    {
      {
        var r: dynamic = l;
        while ((r <= n))
        {
          ret += (koef * solve(n, to(i, l, r), to(j, l, r), (k - 1)));
          r += 1;
        }
      }
      l += 1;
    }
  }
  ans[i][j][k] = ret;
  return ret;
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(false);
  write(fixed, setprecision(10));
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((maxn))))
    {
      {
        var j: dynamic = 0;
        while ((j < cpp_cast((maxn))))
        {
          {
            var k: dynamic = 0;
            while ((k < cpp_cast((maxk))))
            {
              ans[i][j][k] = -1.0;
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var n: dynamic = cpp_uninitialized();
  var k: dynamic = cpp_uninitialized();
  read(n, k);
  {
    var i: dynamic = 0;
    while ((i < cpp_cast((n))))
    {
      read(p[(i + 1)]);
      i += 1;
    }
  }
  var ans: dynamic = 0.0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      {
        var j: dynamic = (i + 1);
        while ((j <= n))
        {
          ans += solve(n, i, j, k);
          j += 1;
        }
      }
      i += 1;
    }
  }
  write(ans, "\n");
  return 0;
}
