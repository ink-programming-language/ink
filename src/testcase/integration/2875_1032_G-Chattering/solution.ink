// Translated from solution.cpp.

var mxN: dynamic = 1e5;

var n: dynamic = cpp_uninitialized();

var l: dynamic = cpp_array(mxN, 17, 17);

var r: dynamic = cpp_array(mxN, 17, 17);

func qry(i: dynamic, l2: dynamic, r2: dynamic) -> dynamic
{
  var b: dynamic = (31 - builtin_clz(((r2 - l2) + 1)));
  return [max(l[i][b][l2], (l[i][b][((((r2 - ((1 << b))) + 1)) % n)] - ((((r2 - ((1 << b))) + 1) - l2)))), max(r[i][b][((((r2 - ((1 << b))) + 1)) % n)], (r[i][b][l2] - ((((r2 - ((1 << b))) + 1) - l2))))];
}

func main() -> dynamic
{
  ios_base.sync_with_stdio(0);
  cin.tie(0);
  read(n);
  if ((n == 1))
  {
    write(0);
    return 0;
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(l[0][0][i]);
      r[0][0][i] = l[0][0][i];
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < 17))
    {
      var a: dynamic = (1 << ((i - 1)));
      {
        var j: dynamic = 0;
        while ((j < n))
        {
          l[0][i][j] = max(l[0][(i - 1)][j], (l[0][(i - 1)][(((j + a)) % n)] - a));
          r[0][i][j] = max(r[0][(i - 1)][(((j + a)) % n)], (r[0][(i - 1)][j] - a));
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i < 17))
    {
      {
        var j: dynamic = 0;
        while ((j < 17))
        {
          var a: dynamic = (1 << j);
          {
            var k: dynamic = 0;
            while ((k < n))
            {
              var l2: dynamic = (k - l[(i - 1)][j][k]);
              var r2: dynamic = (((k + a) - 1) + r[(i - 1)][j][k]);
              if (((r2 - l2) >= (n - 1)))
              {
                l[i][j][k] = cpp_assign(r[i][j][k], "=", n);
                k += 1;
                continue;
              }
              if ((l2 < 0))
              {
                l2 += n;
                r2 += n;
              }
              tie(l[i][j][k], r[i][j][k]) = qry((i - 1), l2, r2);
              l[i][j][k] += l[(i - 1)][j][k];
              r[i][j][k] += r[(i - 1)][j][k];
              k += 1;
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      var l2: dynamic = i;
      var r2: dynamic = i;
      var ans: dynamic = 1;
      var l3: dynamic = cpp_uninitialized();
      var r3: dynamic = cpp_uninitialized();
      {
        var j: dynamic = 16;
        while ((j >= 0))
        {
          tie(l3, r3) = qry(j, l2, r2);
          l3 = (l2 - l3);
          r3 += r2;
          if (((r3 - l3) < (n - 1)))
          {
            ans += (1 << j);
            l2 = l3;
            r2 = r3;
            if ((l2 < 0))
            {
              l2 += n;
              r2 += n;
            }
          }
          j -= 1;
        }
      }
      write(ans, " ");
      i += 1;
    }
  }
}
