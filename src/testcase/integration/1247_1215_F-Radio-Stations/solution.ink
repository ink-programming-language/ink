// Translated from solution.cpp.

func Getint() -> dynamic
{
  var ch: dynamic = getchar();
  var x: dynamic = 0;
  var fh: dynamic = 1;
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      fh = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    (cpp_assign(x, "*=", 10)) += (ch ^ 48);
    ch = getchar();
  }
  return (x * fh);
}

var N: dynamic = 2000005;

var n: dynamic = cpp_uninitialized();

var p: dynamic = cpp_uninitialized();

var M: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var tot: dynamic = cpp_uninitialized();

var G: dynamic = cpp_array(N);

func Addside(x: dynamic, y: dynamic) -> dynamic
{
  G[x].push_back(y);
}

var dfn: dynamic = cpp_array(N);

var llk: dynamic = cpp_array(N);

var tim: dynamic = cpp_uninitialized();

var blk: dynamic = cpp_uninitialized();

var ist: dynamic = cpp_array(N);

var st: dynamic = cpp_array(N);

var col: dynamic = cpp_array(N);

func Tarjan(u: dynamic) -> dynamic
{
  dfn[u] = cpp_assign(llk[u], "=", cpp_update(tim, "++"));
  st[cpp_update(st[0], "++")] = u;
  ist[u] = 1;
  {
    var i: dynamic = 0;
    while ((i <= (int_cpp(G[u].size()) - 1)))
    {
      var v: dynamic = G[u][i];
      if ((!dfn[v]))
      {
        Tarjan(v);
        llk[u] = min(llk[u], llk[v]);
      } else if (ist[v])
      {
        llk[u] = min(llk[u], dfn[v]);
      }
      i += 1;
    }
  }
  if ((llk[u] == dfn[u]))
  {
    blk += 1;
    var x: dynamic = cpp_uninitialized();
    while (true)
    {
      x = st[cpp_update(st[0], "--")];
      ist[x] = 0;
      col[x] = blk;
      if (!(((x != u))))
      {
        break;
      }
    }
  }
}

func main() -> dynamic
{
  var x: dynamic = cpp_uninitialized();
  var y: dynamic = cpp_uninitialized();
  var z: dynamic = cpp_uninitialized();
  n = Getint();
  p = Getint();
  M = Getint();
  m = Getint();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      x = Getint();
      y = Getint();
      Addside((x << 1), ((y << 1) | 1));
      Addside((y << 1), ((x << 1) | 1));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= p))
    {
      x = Getint();
      y = Getint();
      Addside(((i << 1) | 1), ((((p + x)) << 1) | 1));
      Addside((((p + x)) << 1), (i << 1));
      Addside(((i << 1) | 1), ((((p + y) + 1)) << 1));
      Addside((((((p + y) + 1)) << 1) | 1), (i << 1));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      x = Getint();
      y = Getint();
      Addside(((x << 1) | 1), (y << 1));
      Addside(((y << 1) | 1), (x << 1));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= M))
    {
      Addside((((p + i)) << 1), ((((p + i) + 1)) << 1));
      Addside((((((p + i) + 1)) << 1) | 1), ((((p + i)) << 1) | 1));
      i += 1;
    }
  }
  tot = ((p + M) + 1);
  {
    var i: dynamic = 2;
    while ((i <= (((tot << 1) | 1))))
    {
      if ((!dfn[i]))
      {
        Tarjan(i);
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= tot))
    {
      if ((col[(i << 1)] == col[((i << 1) | 1)]))
      {
        return cpp_comma(puts("-1"), 0);
      }
      i += 1;
    }
  }
  var Ans: dynamic = cpp_uninitialized();
  Ans.clear();
  var f: dynamic = 1;
  {
    var i: dynamic = 1;
    while ((i <= tot))
    {
      if ((col[(i << 1)] < col[((i << 1) | 1)]))
      {
        i += 1;
        continue;
      }
      if ((i <= p))
      {
        Ans.push_back(i);
      } else
      {
        f = max(f, (i - p));
      }
      i += 1;
    }
  }
  write(Ans.size(), cpp_char(" "), f, cpp_char("\n"));
  {
    var i: dynamic = 0;
    while ((i <= (int_cpp(Ans.size()) - 1)))
    {
      write(Ans[i], cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
  return 0;
}
