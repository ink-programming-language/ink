// Translated from solution.cpp.

var maxn = (5000 + 10);

var a = cpp_array(maxn);

var b = cpp_array(maxn);

var c = cpp_array(maxn);

var cnt = cpp_array(maxn);

var n: dynamic;

var m: dynamic;

var mark = cpp_array(maxn);

var q = cpp_array(5, maxn);

var Bomb: dynamic;

func main()
{
  read(n, m);
  {
    var i = 1;
    while ((i <= m))
    {
      read(q[i][1]);
      if ((q[i][1] == 1))
      {
        read(q[i][2], q[i][3], q[i][4]);
        {
          var j = q[i][2];
          while ((j <= q[i][3]))
          {
            cnt[j] += q[i][4];
            j += 1;
          }
        }
      } else
      {
        read(q[i][2], q[i][3], q[i][4]);
        var flag = 0;
        {
          var j = q[i][2];
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
                var t = (b[j] + cnt[j]);
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
    var i = 1;
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
      var i = 1;
      while ((i <= m))
      {
        if ((q[i][1] == 1))
        {
          {
            var j = q[i][2];
            while ((j <= q[i][3]))
            {
              a[j] += q[i][4];
              j += 1;
            }
          }
        } else
        {
          var mx = -1000000001;
          {
            var j = q[i][2];
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
      var F = 0;
      {
        var i = 1;
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
        var i = 1;
        while ((i <= n))
        {
          write(c[i], " ");
          i += 1;
        }
      }
    }
  }
}
