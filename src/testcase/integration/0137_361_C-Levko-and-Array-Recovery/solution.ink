// Translated from solution.cpp.

var maxn: dynamic = (5000 + 10);

var a: dynamic = cpp_array(maxn);

var b: dynamic = cpp_array(maxn);

var c: dynamic = cpp_array(maxn);

var cnt: dynamic = cpp_array(maxn);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var mark: dynamic = cpp_array(maxn);

var q: dynamic = cpp_array(5, maxn);

var Bomb: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      read(q[i][1]);
      if ((q[i][1] == 1))
      {
        read(q[i][2], q[i][3], q[i][4]);
        {
          var j: dynamic = q[i][2];
          while ((j <= q[i][3]))
          {
            cnt[j] += q[i][4];
            j += 1;
          }
        }
      } else
      {
        read(q[i][2], q[i][3], q[i][4]);
        var flag: dynamic = 0;
        {
          var j: dynamic = q[i][2];
          while ((j <= q[i][3]))
          {
            if ((!mark[j]))
            {
              flag = true;
              a[j] = (q[i][4] - cnt[j]);
              b[j] = q[i][4];
              mark[j] = true;
              cnt[j] = 0;
            } else
            {
              if (((cnt[j] + b[j]) >= q[i][4]))
              {
                flag = true;
                var t: dynamic = (b[j] + cnt[j]);
                t -= q[i][4];
                a[j] -= t;
                b[j] = q[i][4];
              } else
              {
                b[j] += cnt[j];
              }
              cnt[j] = 0;
            }
            j += 1;
          }
        }
        if ((!flag))
        {
          Bomb = true;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      c[i] = a[i];
      i += 1;
    }
  }
  if (Bomb)
  {
    write("NO");
    return 0;
  } else
  {
    {
      var i: dynamic = 1;
      while ((i <= m))
      {
        if ((q[i][1] == 1))
        {
          {
            var j: dynamic = q[i][2];
            while ((j <= q[i][3]))
            {
              a[j] += q[i][4];
              j += 1;
            }
          }
        } else
        {
          var mx: dynamic = -1000000001;
          {
            var j: dynamic = q[i][2];
            while ((j <= q[i][3]))
            {
              mx = max(mx, a[j]);
              j += 1;
            }
          }
          if ((mx != q[i][4]))
          {
            Bomb = true;
          }
        }
        i += 1;
      }
    }
    if (Bomb)
    {
      write("NO");
      return 0;
    } else
    {
      var F: dynamic = 0;
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          if (((c[i] > 1000000000) || (c[i] < -1000000000)))
          {
            F = true;
          }
          i += 1;
        }
      }
      if (F)
      {
        write("NO");
        return 0;
      }
      write("YES", "\n");
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          write(c[i], " ");
          i += 1;
        }
      }
    }
  }
}
