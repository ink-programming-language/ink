// Translated from solution.cpp.

func read()
{
  var res = 0;
  var fh = 1;
  var ch = getchar();
  while (((((ch > cpp_char("9")) || (ch < cpp_char("0")))) && (ch != cpp_char("-"))))
  {
    ch = getchar();
  }
  if ((ch == cpp_char("-")))
  {
    fh = -1;
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    res = (((res * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (fh * res);
}

var maxn = 16;

var n: dynamic;

var m: dynamic;

var mxs: dynamic;

var sum = cpp_array((((1 << 16)) + 12));

var a = cpp_array(maxn, maxn);

var f = cpp_array(maxn, ((1 << maxn)));

func Max(a: dynamic, b: dynamic)
{
  return if ((a > b)) a else b;
}

func main()
{
  n = read();
  m = read();
  mxs = (((1 << n)) - 1);
  {
    var i = 1;
    while ((i <= m))
    {
      var x = (read() - 1);
      var y = (read() - 1);
      var z = read();
      a[x][y] = cpp_assign(a[y][x], "=", z);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i <= mxs))
    {
      sum[i] = 0;
      {
        var j = 0;
        while ((j < n))
        {
          if (((i & ((1 << j)))))
          {
            {
              var k = (j + 1);
              while ((k < n))
              {
                if (((i & ((1 << k)))))
                {
                  sum[i] += a[j][k];
                }
                k += 1;
              }
            }
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  memset(f, -1, cpp_sizeof(f));
  f[1][0] = 0;
  {
    var i = 1;
    while ((i <= mxs))
    {
      {
        var j = 0;
        while ((j < n))
        {
          if ((f[i][j] == -1))
          {
            j += 1;
            continue;
          }
          if ((!((i & ((1 << j))))))
          {
            j += 1;
            continue;
          }
          {
            var k = 0;
            while ((k < n))
            {
              if ((a[j][k] && ((!((i & ((1 << k))))))))
              {
                f[(i | ((1 << k)))][k] = Max(f[(i | ((1 << k)))][k], (f[i][j] + a[j][k]));
              }
              k += 1;
            }
          }
          var c = (((mxs - i)) | ((1 << j)));
          {
            var k = c;
            while ((k >= 0))
            {
              f[(i | k)][j] = Max((f[i][j] + sum[k]), f[(i | k)][j]);
              k = (if ((k == 0)) -1 else (((k - 1)) & c));
            }
          }
          j += 1;
        }
      }
      i += 2;
    }
  }
  var ans = 0;
  {
    var i = (((mxs >> 1)) + 2);
    while ((i <= mxs))
    {
      ans = Max(ans, f[i][(n - 1)]);
      i += 2;
    }
  }
  printf("%d\n", (sum[mxs] - ans));
  return 0;
}
