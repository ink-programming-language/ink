// Translated from solution.cpp.

func Getint()
{
  var ch = getchar();
  var x = 0;
  var fh = 1;
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

var N = 2000005;

var n: dynamic;

var p: dynamic;

var M: dynamic;

var m: dynamic;

var tot: dynamic;

var G = cpp_array(N);

func Addside(x: dynamic, y: dynamic)
{
  G[x].push_back(y);
}

var dfn = cpp_array(N);

var llk = cpp_array(N);

var tim: dynamic;

var blk: dynamic;

var ist = cpp_array(N);

var st = cpp_array(N);

var col = cpp_array(N);

func Tarjan(u: dynamic)
{
  dfn[u] = cpp_assign(llk[u], "=", cpp_update(tim, "++"));
  st[cpp_update(st[0], "++")] = u;
  ist[u] = 1;
  {
    var i = 0;
    while ((i <= (int_cpp(G[u].size()) - 1)))
    {
      var v = G[u][i];
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
    var x: dynamic;
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

func main()
{
  var x: dynamic;
  var y: dynamic;
  var z: dynamic;
  n = Getint();
  p = Getint();
  M = Getint();
  m = Getint();
  {
    var i = 1;
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
    var i = 1;
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
    var i = 1;
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
    var i = 1;
    while ((i <= M))
    {
      Addside((((p + i)) << 1), ((((p + i) + 1)) << 1));
      Addside((((((p + i) + 1)) << 1) | 1), ((((p + i)) << 1) | 1));
      i += 1;
    }
  }
  tot = ((p + M) + 1);
  {
    var i = 2;
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
    var i = 1;
    while ((i <= tot))
    {
      if ((col[(i << 1)] == col[((i << 1) | 1)]))
      {
        return cpp_comma(puts("-1"), 0);
      }
      i += 1;
    }
  }
  var Ans: dynamic;
  Ans.clear();
  var f = 1;
  {
    var i = 1;
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
    var i = 0;
    while ((i <= (int_cpp(Ans.size()) - 1)))
    {
      write(Ans[i], cpp_char(" "));
      i += 1;
    }
  }
  write(cpp_char("\n"));
  return 0;
}
