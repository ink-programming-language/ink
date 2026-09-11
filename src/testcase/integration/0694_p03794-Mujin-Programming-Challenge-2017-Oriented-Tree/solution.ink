// Translated from solution.cpp.

func debug() -> dynamic
{
  return cpp_expression("#include <cstdio> #include <");
}

func getchar() -> dynamic
{
  return cpp_expression("#include <cstdi");
}

func putchar(x: dynamic) -> dynamic
{
  return cpp_expression("#include <cstdio");
}

var IN_BUF: dynamic = (1 << 23);

var OUT_BUF: dynamic = (1 << 23);

func myGetchar() -> dynamic
{
  var buf: dynamic = cpp_array(IN_BUF);
  var ps: dynamic = buf;
  var pt: dynamic = buf;
  if ((ps == pt))
  {
    ps = buf;
    pt = (buf + fread(buf, 1, IN_BUF, stdin));
  }
  return  ((ps == pt)) ? EOF : (*cpp_update(ps, "++"));
}

func read(x: dynamic) -> dynamic
{
  var op: dynamic = 0;
  var ch: dynamic = getchar();
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

func readStr(s: dynamic) -> dynamic
{
  var n: dynamic = 0;
  var ch: dynamic = getchar();
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

func myPutchar(x: dynamic) -> dynamic
{
  var pbuf: dynamic = cpp_array(OUT_BUF);
  var pp: dynamic = pbuf;
  cpp_statement("struct _flusher { ~_flusher() { fwrite(pbuf, 1, pp - pbuf, stdout); } }");
  var outputFlusher: dynamic = cpp_uninitialized();
  if ((pp == (pbuf + OUT_BUF)))
  {
    fwrite(pbuf, 1, OUT_BUF, stdout);
    pp = pbuf;
  }
  (*cpp_update(pp, "++")) = x;
}

func print(x: dynamic) -> dynamic
{
  if ((x == 0))
  {
    putchar(cpp_char("0"));
    return;
  }
  var num: dynamic = cpp_uninitialized();
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

func print(x: dynamic, ch: dynamic = cpp_char("\n")) -> dynamic
{
  print(x);
  putchar(ch);
}

func printStr(s: dynamic, n: dynamic = -1) -> dynamic
{
  if ((n == -1))
  {
    n = strlen(s);
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      putchar(s[i]);
      i += 1;
    }
  }
}

func printStr(s: dynamic, n: dynamic = -1, ch: dynamic = cpp_char("\n")) -> dynamic
{
  printStr(s, n);
  putchar(ch);
}

var N: dynamic = 5005;

var P: dynamic = 1000000007;

var n: dynamic = cpp_uninitialized();

var type_cpp: dynamic = cpp_uninitialized();

var D: dynamic = cpp_uninitialized();

var E: dynamic = cpp_array(N);

var fa: dynamic = cpp_array(N);

func dfs(u: dynamic, fa: dynamic = 0) -> dynamic
{
  var res: dynamic = cpp_construct(0, u);
  for (var v: dynamic in E[u])
  {
    if ((v != fa))
    {
      var tmp: dynamic = dfs(v, u);
      tmp.first += 1;
      res = max(res, tmp);
    }
  }
  return res;
}

func getfa(u: dynamic) -> dynamic
{
  for (var v: dynamic in E[u])
  {
    if ((v != fa[u]))
    {
      fa[v] = u;
      getfa(v);
    }
  }
}

var f: dynamic = cpp_array((N << 1), N);

func DP(u: dynamic, fa: dynamic, d: dynamic) -> dynamic
{
  {
    var i: dynamic = (-D);
    while ((i <= D))
    {
      f[u][(i + D)] = (((-d) <= i) && (i <= d));
      i += 1;
    }
  }
  for (var v: dynamic in E[u])
  {
    if ((v != fa))
    {
      DP(v, u, (d - 1));
      {
        var i: dynamic = 0;
        while ((i <= (2 * D)))
        {
          f[u][i] = (((1 * f[u][i]) * ((( ((i == 0)) ? 0 : f[v][(i - 1)]) + ( ((i == (2 * D))) ? 0 : f[v][(i + 1)])))) % P);
          i += 1;
        }
      }
    }
  }
}

func main() -> dynamic
{
  read(n);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u);
      read(v);
      E[u].push_back(v);
      E[v].push_back(u);
      i += 1;
    }
  }
  var S: dynamic = dfs(1).second;
  var T: dynamic = cpp_uninitialized();
  var tmp: dynamic = dfs(S);
  T = tmp.second;
  D = tmp.first;
  getfa(S);
  if ((D & 1))
  {
    D = (((D + 1)) >> 1);
    var s: dynamic = cpp_uninitialized();
    var t: dynamic = T;
    var ans: dynamic = 0;
    {
      var i: dynamic = 1;
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
      var i: dynamic = 0;
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
      var i: dynamic = 0;
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
      var i: dynamic = 0;
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
    var r: dynamic = T;
    var ans: dynamic = 0;
    {
      var i: dynamic = 1;
      while ((i <= D))
      {
        r = fa[r];
        i += 1;
      }
    }
    DP(r, 0, D);
    {
      var i: dynamic = 0;
      while ((i <= (2 * D)))
      {
        ans = (((ans + f[r][i])) % P);
        i += 1;
      }
    }
    print(ans);
  }
}
