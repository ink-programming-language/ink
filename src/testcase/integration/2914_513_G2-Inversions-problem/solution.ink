// Translated from solution.cpp.

var maxn = 35;

var maxk = 205;

var p = cpp_array(maxn);

var ans = cpp_array(maxk, maxn, maxn);

func to(x: dynamic, l: dynamic, r: dynamic)
{
  if (((x < l) || (x > r)))
  {
    return x;
  }
  return ((l + r) - x);
}

func solve(n: dynamic, i: dynamic, j: dynamic, k: dynamic)
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
  var koef = (2.0 / ((n * ((n + 1)))));
  var ret = 0.0;
  {
    var l = 1;
    while ((l <= n))
    {
      {
        var r = l;
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

func main()
{
  ios_base.sync_with_stdio(false);
  write(fixed, setprecision(10));
  {
    var i = 0;
    while ((i < cpp_cast((maxn))))
    {
      {
        var j = 0;
        while ((j < cpp_cast((maxn))))
        {
          {
            var k = 0;
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
  var n: dynamic;
  var k: dynamic;
  read(n, k);
  {
    var i = 0;
    while ((i < cpp_cast((n))))
    {
      read(p[(i + 1)]);
      i += 1;
    }
  }
  var ans = 0.0;
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = (i + 1);
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
