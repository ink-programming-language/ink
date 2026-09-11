// Translated from solution.cpp.

var maxn: dynamic = (3e5 + 5);

var choice: dynamic = [[cpp_char("A"), cpp_char("C")], [cpp_char("A"), cpp_char("G")], [cpp_char("A"), cpp_char("T")], [cpp_char("C"), cpp_char("G")], [cpp_char("C"), cpp_char("T")], [cpp_char("G"), cpp_char("T")]];

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var str: dynamic = cpp_array(maxn);

var ord: dynamic = cpp_array(6, maxn, 2);

var cnt: dynamic = cpp_array(maxn, 2);

var out: dynamic = cpp_array(maxn);

func print(rc: dynamic, k: dynamic) -> dynamic
{
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      out[i] = "";
      i += 1;
    }
  }
  if ((rc == 0))
  {
    {
      var i: dynamic = 0;
      while ((i < n))
      {
        {
          var j: dynamic = 0;
          while ((j < m))
          {
            out[i] += choice[ (((i & 1))) ? (5 - k) : k][(((j & 1)) ^ ord[0][i][k])];
            j += 1;
          }
        }
        i += 1;
      }
    }
  } else
  {
    {
      var j: dynamic = 0;
      while ((j < m))
      {
        {
          var i: dynamic = 0;
          while ((i < n))
          {
            out[i] += choice[ (((j & 1))) ? (5 - k) : k][(((i & 1)) ^ ord[1][j][k])];
            i += 1;
          }
        }
        j += 1;
      }
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      write(out[i], "\n");
      i += 1;
    }
  }
}

func main() -> dynamic
{
  read(n, m);
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(str[i]);
      i += 1;
    }
  }
  memset(cnt, 0, cpp_sizeof((cnt)));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      {
        var k: dynamic = 0;
        while ((k < 6))
        {
          var now1: dynamic = 0;
          var now2: dynamic = 0;
          {
            var j: dynamic = 0;
            while ((j < m))
            {
              now1 += ((str[i][j] != choice[ (((i & 1))) ? (5 - k) : k][(j & 1)]));
              now2 += ((str[i][j] != choice[ (((i & 1))) ? (5 - k) : k][(((j & 1)) ^ 1)]));
              j += 1;
            }
          }
          ord[0][i][k] =  ((now1 < now2)) ? 0 : 1;
          cnt[0][k] += min(now1, now2);
          k += 1;
        }
      }
      i += 1;
    }
  }
  {
    var j: dynamic = 0;
    while ((j < m))
    {
      {
        var k: dynamic = 0;
        while ((k < 6))
        {
          var now1: dynamic = 0;
          var now2: dynamic = 0;
          {
            var i: dynamic = 0;
            while ((i < n))
            {
              now1 += ((str[i][j] != choice[ (((j & 1))) ? (5 - k) : k][(i & 1)]));
              now2 += ((str[i][j] != choice[ (((j & 1))) ? (5 - k) : k][(((i & 1)) ^ 1)]));
              i += 1;
            }
          }
          ord[1][j][k] =  ((now1 < now2)) ? 0 : 1;
          cnt[1][k] += min(now1, now2);
          k += 1;
        }
      }
      j += 1;
    }
  }
  var ans: dynamic = 0x3f3f3f3f;
  var RC: dynamic = cpp_uninitialized();
  var K: dynamic = cpp_uninitialized();
  {
    var rc: dynamic = 0;
    while ((rc <= 1))
    {
      {
        var k: dynamic = 0;
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
