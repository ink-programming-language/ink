// Translated from solution.cpp.

var maxn = 4111;

var maxm = 257;

var inf = 0x3f3f3f3f;

var f = cpp_array(maxn, maxn);

var A = cpp_array(maxn);

var B = cpp_array(maxn);

var pos = cpp_array(maxm, 2);

var t = cpp_array(5);

var n: dynamic;

var m: dynamic;

func get()
{
  {
    var i = 0;
    while ((i < 4))
    {
      if ((1 != scanf("%d", (t + i))))
      {
        return 0;
      }
      i += 1;
    }
  }
  scanf("%s%s", (A + 1), (B + 1));
  return 1;
}

func update(a: dynamic, b: dynamic)
{
  if ((b < a))
  {
    a = b;
  }
}

func work()
{
  n = strlen((A + 1));
  m = strlen((B + 1));
  {
    var i = 0;
    while ((i <= n))
    {
      {
        var j = 0;
        while ((j <= m))
        {
          f[i][j] = inf;
          j += 1;
        }
      }
      i += 1;
    }
  }
  f[0][0] = 0;
  {
    var i = 1;
    while ((i <= n))
    {
      f[i][0] = (i * t[1]);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      f[0][i] = (i * t[0]);
      i += 1;
    }
  }
  memset(pos, 0, cpp_sizeof(pos));
  {
    var i = 1;
    while ((i <= n))
    {
      {
        var j = 1;
        while ((j <= m))
        {
          update(f[i][j], (f[i][(j - 1)] + t[0]));
          update(f[i][j], (f[(i - 1)][j] + t[1]));
          update(f[i][j], (f[(i - 1)][(j - 1)] + (((A[i] != B[j])) * t[2])));
          var a: dynamic;
          var b: dynamic;
          if (((cpp_assign(b, "=", pos[1][A[i]])) && (cpp_assign(a, "=", pos[0][B[j]]))))
          {
            update(f[i][j], (((f[(a - 1)][(b - 1)] + t[3]) + ((((i - a) - 1)) * t[1])) + ((((j - b) - 1)) * t[0])));
          }
          pos[1][B[j]] = j;
          j += 1;
        }
      }
      pos[0][A[i]] = i;
      memset(pos[1], 0, cpp_sizeof(pos[1]));
      i += 1;
    }
  }
  write(f[n][m], "\n");
}

func main()
{
  while (get())
  {
    work();
  }
  return 0;
}
