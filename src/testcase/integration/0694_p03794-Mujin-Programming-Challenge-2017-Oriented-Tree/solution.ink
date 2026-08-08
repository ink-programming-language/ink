// Translated from solution.cpp.

func debug()
{
  return cpp_expression("#include <cstdio> #include <");
}

func getchar()
{
  return cpp_expression("#include <cstdi");
}

func putchar(x: dynamic)
{
  return cpp_expression("#include <cstdio");
}

var IN_BUF = (1 << 23);

var OUT_BUF = (1 << 23);

func myGetchar()
{
  var buf = cpp_array(IN_BUF);
  var ps = buf;
  var pt = buf;
  if ((ps == pt))
  {
    ps = buf;
    pt = (buf + fread(buf, 1, IN_BUF, stdin));
  }
  return if ((ps == pt)) EOF else (*cpp_update(ps, "++"));
}

func read(x: dynamic)
{
  var op = 0;
  var ch = getchar();
  x = 0;
  {
    while (((!isdigit(ch)) && (ch != EOF)))
    {
      op ^= ((ch == cpp_char("-")));
      ch = getchar();
    }
  }
  if ((ch == EOF))
  {
    return false;
  }
  {
    while (isdigit(ch))
    {
      x = ((x * 10) + ((ch ^ cpp_char("0"))));
      ch = getchar();
    }
  }
  if (op)
  {
    x = (-x);
  }
  return true;
}

func readStr(s: dynamic)
{
  var n = 0;
  var ch = getchar();
  {
    while ((isspace(ch) && (ch != EOF)))
    {
      ch = getchar();
    }
  }
  {
    while (((!isspace(ch)) && (ch != EOF)))
    {
      s[cpp_update(n, "++")] = ch;
      ch = getchar();
    }
  }
  s[n] = cpp_char("\u{0}");
  return n;
}

func myPutchar(x: dynamic)
{
  var pbuf = cpp_array(OUT_BUF);
  var pp = pbuf;
  cpp_statement("struct _flusher { ~_flusher() { fwrite(pbuf, 1, pp - pbuf, stdout); } }");
  var outputFlusher: dynamic;
  if ((pp == (pbuf + OUT_BUF)))
  {
    fwrite(pbuf, 1, OUT_BUF, stdout);
    pp = pbuf;
  }
  (*cpp_update(pp, "++")) = x;
}

func print(x: dynamic)
{
  if ((x == 0))
  {
    putchar(cpp_char("0"));
    return;
  }
  var num: dynamic;
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  {
    while (x)
    {
      num.push_back((x % 10));
      x /= 10;
    }
  }
  while ((!num.empty()))
  {
    putchar((num.back() ^ cpp_char("0")));
    num.pop_back();
  }
}

func print(x: dynamic, ch: dynamic = cpp_char("\n"))
{
  print(x);
  putchar(ch);
}

func printStr(s: dynamic, n: dynamic = -1)
{
  if ((n == -1))
  {
    n = strlen(s);
  }
  {
    var i = 0;
    while ((i < n))
    {
      putchar(s[i]);
      i += 1;
    }
  }
}

func printStr(s: dynamic, n: dynamic = -1, ch: dynamic = cpp_char("\n"))
{
  printStr(s, n);
  putchar(ch);
}

var N = 5005;

var P = 1000000007;

var n: dynamic;

var type_cpp: dynamic;

var D: dynamic;

var E = cpp_array(N);

var fa = cpp_array(N);

func dfs(u: dynamic, fa: dynamic = 0)
{
  var res = cpp_construct(0, u);
  for (var v in E[u])
  {
    if ((v != fa))
    {
      var tmp = dfs(v, u);
      tmp.first += 1;
      res = max(res, tmp);
    }
  }
  return res;
}

func getfa(u: dynamic)
{
  for (var v in E[u])
  {
    if ((v != fa[u]))
    {
      fa[v] = u;
      getfa(v);
    }
  }
}

var f = cpp_array((N << 1), N);

func DP(u: dynamic, fa: dynamic, d: dynamic)
{
  {
    var i = (-D);
    while ((i <= D))
    {
      f[u][(i + D)] = (((-d) <= i) && (i <= d));
      i += 1;
    }
  }
  for (var v in E[u])
  {
    if ((v != fa))
    {
      DP(v, u, (d - 1));
      {
        var i = 0;
        while ((i <= (2 * D)))
        {
          f[u][i] = (((1 * f[u][i]) * (((if ((i == 0)) 0 else f[v][(i - 1)]) + (if ((i == (2 * D))) 0 else f[v][(i + 1)])))) % P);
          i += 1;
        }
      }
    }
  }
}

func main()
{
  read(n);
  {
    var i = 1;
    while ((i < n))
    {
      var u: dynamic;
      var v: dynamic;
      read(u);
      read(v);
      E[u].push_back(v);
      E[v].push_back(u);
      i += 1;
    }
  }
  var S = dfs(1).second;
  var T: dynamic;
  var tmp = dfs(S);
  T = tmp.second;
  D = tmp.first;
  getfa(S);
  if ((D & 1))
  {
    D = (((D + 1)) >> 1);
    var s: dynamic;
    var t = T;
    var ans = 0;
    {
      var i = 1;
      while ((i < D))
      {
        t = fa[t];
        i += 1;
      }
    }
    s = fa[t];
    DP(s, t, D);
    DP(t, s, (D - 1));
    {
      var i = 0;
      while ((i <= (2 * D)))
      {
        ans = (((ans + ((1 * f[s][i]) * f[t][(i + 1)]))) % P);
        ans = (((ans + ((1 * f[t][i]) * f[s][(i + 1)]))) % P);
        i += 1;
      }
    }
    DP(s, t, (D - 1));
    DP(t, s, D);
    {
      var i = 0;
      while ((i <= (2 * D)))
      {
        ans = (((ans + ((1 * f[s][i]) * f[t][(i + 1)]))) % P);
        ans = (((ans + ((1 * f[t][i]) * f[s][(i + 1)]))) % P);
        i += 1;
      }
    }
    DP(s, t, (D - 1));
    DP(t, s, (D - 1));
    {
      var i = 0;
      while ((i <= (2 * D)))
      {
        ans = (((ans - ((2 * f[s][i]) * f[t][i]))) % P);
        ans = (((ans - ((1 * f[s][i]) * f[t][(i + 2)]))) % P);
        ans = (((ans - ((1 * f[t][i]) * f[s][(i + 2)]))) % P);
        i += 1;
      }
    }
    print((((ans + P)) % P));
  } else
  {
    D >>= 1;
    var r = T;
    var ans = 0;
    {
      var i = 1;
      while ((i <= D))
      {
        r = fa[r];
        i += 1;
      }
    }
    DP(r, 0, D);
    {
      var i = 0;
      while ((i <= (2 * D)))
      {
        ans = (((ans + f[r][i])) % P);
        i += 1;
      }
    }
    print(ans);
  }
}
