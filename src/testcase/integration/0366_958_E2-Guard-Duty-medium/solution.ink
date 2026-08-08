// Translated from solution.cpp.

var maxn = 500010;

var maxk = 5010;

var oo = 1e13;

var k: dynamic;

var n: dynamic;

var a = cpp_array(maxn);

var f = cpp_array(3, maxk, 3);

var ti: dynamic;

func main()
{
  read(k, n);
  {
    var i = 1;
    while ((i <= n))
    {
      read(a[i]);
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1));
  {
    var i = 2;
    while ((i <= n))
    {
      ti.push_back(make_pair((a[i] - a[(i - 1)]), i));
      i += 1;
    }
  }
  sort(ti.begin(), ti.end());
  var le = ti.size();
  var t4k: dynamic;
  {
    var i = 0;
    while ((i < min((2 * k), le)))
    {
      t4k.insert(a[ti[i].second]);
      t4k.insert(a[(ti[i].second - 1)]);
      i += 1;
    }
  }
  n = 1;
  for (var x in t4k)
  {
    a[cpp_update(n, "++")] = x;
  }
  n -= 1;
  {
    var i = 0;
    while ((i < 2))
    {
      {
        var j = 0;
        while ((j <= k))
        {
          {
            var tt = 0;
            while ((tt < 2))
            {
              f[i][j][tt] = oo;
              tt += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  var t = 0;
  f[(1 - t)][0][0] = 0;
  {
    var tim = 2;
    while ((tim <= n))
    {
      {
        var j = 1;
        while ((j <= k))
        {
          {
            var tt = 0;
            while ((tt < 2))
            {
              f[t][j][tt] = oo;
              tt += 1;
            }
          }
          j += 1;
        }
      }
      {
        var j = 1;
        while ((j <= min(tim, k)))
        {
          f[t][j][0] = min(f[(1 - t)][j][1], f[(1 - t)][j][0]);
          f[t][j][1] = min(f[t][j][1], ((f[(1 - t)][(j - 1)][0] + a[tim]) - a[(tim - 1)]));
          f[t][j][1] = min(f[t][j][1], f[t][j][0]);
          j += 1;
        }
      }
      t = (1 - t);
      tim += 1;
    }
  }
  write(min(f[(1 - t)][k][0], f[(1 - t)][k][1]));
}
