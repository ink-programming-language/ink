// Translated from solution.cpp.

var N: dynamic = 1000100;

var L: dynamic = 5000500;

var BufL: dynamic = 220;

var ch: dynamic = [];

var buf: dynamic = [];

var n: dynamic = cpp_uninitialized();

var r: dynamic = cpp_uninitialized();

var c: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var a: dynamic = [];

var p: dynamic = [];

var f: dynamic = [];

var g: dynamic = [];

func main() -> dynamic
{
  gets((buf + 1));
  sscanf((buf + 1), "%d%d%d", (&n), (&r), (&c));
  gets((ch + 1));
  l = strlen((ch + 1));
  {
    var i: dynamic = 1;
    var tot: dynamic = 0;
    while ((i <= l))
    {
      if (isalpha(ch[i]))
      {
        p[cpp_update(tot, "++")] = i;
        while (((i <= l) && isalpha(ch[i])))
        {
          a[tot] += 1;
          i += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= (n + 1)))
    {
      g[i] = i;
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    var t: dynamic = 0;
    var s: dynamic = -1;
    while ((i <= n))
    {
      while (((t <= n) && (s <= c)))
      {
        s += (a[cpp_update(t, "++")] + 1);
      }
      f[i] = t;
      s -= (a[cpp_update(i, "++")] + 1);
    }
  }
  f[(n + 1)] = (n + 1);
  {
    while (r)
    {
      if ((r & 1))
      {
        {
          var i: dynamic = 1;
          while ((i <= n))
          {
            g[i] = f[g[i]];
            i += 1;
          }
        }
      }
      {
        var i: dynamic = 1;
        while ((i <= n))
        {
          f[i] = f[f[i]];
          i += 1;
        }
      }
      r >>= 1;
    }
  }
  var ans: dynamic = 0;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (((g[i] - i) > (g[ans] - ans)))
      {
        ans = i;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = ans;
    var s: dynamic = c;
    while ((i < g[ans]))
    {
      if ((((s + a[i]) + 1) <= c))
      {
        s += (a[i] + 1);
        putchar(cpp_char(" "));
      } else
      {
        if ((i > ans))
        {
          puts("");
        }
        s = a[i];
      }
      {
        var j: dynamic = p[i];
        while ((j < (p[i] + a[i])))
        {
          putchar(ch[j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  return 0;
}
