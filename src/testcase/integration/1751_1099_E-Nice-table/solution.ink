// Translated from solution.cpp.

var maxn = (3e5 + 5);

var choice = [[cpp_char("A"), cpp_char("C")], [cpp_char("A"), cpp_char("G")], [cpp_char("A"), cpp_char("T")], [cpp_char("C"), cpp_char("G")], [cpp_char("C"), cpp_char("T")], [cpp_char("G"), cpp_char("T")]];

var n: dynamic;

var m: dynamic;

var str = cpp_array(maxn);

var ord = cpp_array(6, maxn, 2);

var cnt = cpp_array(maxn, 2);

var out = cpp_array(maxn);

func print(rc: dynamic, k: dynamic)
{
  {
    var i = 0;
    while ((i < n))
    {
      out[i] = "";
      i += 1;
    }
  }
  if ((rc == 0))
  {
    {
      var i = 0;
      while ((i < n))
      {
        {
          var j = 0;
          while ((j < m))
          {
            out[i] += choice[if (((i & 1))) (5 - k) else k][(((j & 1)) ^ ord[0][i][k])];
            j += 1;
          }
        }
        i += 1;
      }
    }
  } else
  {
    {
      var j = 0;
      while ((j < m))
      {
        {
          var i = 0;
          while ((i < n))
          {
            out[i] += choice[if (((j & 1))) (5 - k) else k][(((i & 1)) ^ ord[1][j][k])];
            i += 1;
          }
        }
        j += 1;
      }
    }
  }
  {
    var i = 0;
    while ((i < n))
    {
      write(out[i], "\n");
      i += 1;
    }
  }
}

func main()
{
  read(n, m);
  {
    var i = 0;
    while ((i < n))
    {
      read(str[i]);
      i += 1;
    }
  }
  memset(cnt, 0, cpp_sizeof((cnt)));
  {
    var i = 0;
    while ((i < n))
    {
      {
        var k = 0;
        while ((k < 6))
        {
          var now1 = 0;
          var now2 = 0;
          {
            var j = 0;
            while ((j < m))
            {
              now1 += ((str[i][j] != choice[if (((i & 1))) (5 - k) else k][(j & 1)]));
              now2 += ((str[i][j] != choice[if (((i & 1))) (5 - k) else k][(((j & 1)) ^ 1)]));
              j += 1;
            }
          }
          ord[0][i][k] = if ((now1 < now2)) 0 else 1;
          cnt[0][k] += min(now1, now2);
          k += 1;
        }
      }
      i += 1;
    }
  }
  {
    var j = 0;
    while ((j < m))
    {
      {
        var k = 0;
        while ((k < 6))
        {
          var now1 = 0;
          var now2 = 0;
          {
            var i = 0;
            while ((i < n))
            {
              now1 += ((str[i][j] != choice[if (((j & 1))) (5 - k) else k][(i & 1)]));
              now2 += ((str[i][j] != choice[if (((j & 1))) (5 - k) else k][(((i & 1)) ^ 1)]));
              i += 1;
            }
          }
          ord[1][j][k] = if ((now1 < now2)) 0 else 1;
          cnt[1][k] += min(now1, now2);
          k += 1;
        }
      }
      j += 1;
    }
  }
  var ans = 0x3f3f3f3f;
  var RC: dynamic;
  var K: dynamic;
  {
    var rc = 0;
    while ((rc <= 1))
    {
      {
        var k = 0;
        while ((k < 6))
        {
          if ((cnt[rc][k] < ans))
          {
            ans = cnt[rc][k];
            RC = rc;
            K = k;
          }
          k += 1;
        }
      }
      rc += 1;
    }
  }
  print(RC, K);
}
