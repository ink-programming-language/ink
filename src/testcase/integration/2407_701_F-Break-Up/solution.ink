// Translated from solution.cpp.

func down(a: dynamic, b: dynamic) -> dynamic
{
  if ((a > b))
  {
    a = b;
  }
}

var maxn: dynamic = 2100;

var maxm: dynamic = 110000;

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var S: dynamic = cpp_uninitialized();

var T: dynamic = cpp_uninitialized();

var e: dynamic = cpp_array(3, maxm);

var ok: dynamic = cpp_array(maxm);

var t: dynamic = cpp_array(maxm);

var tp: dynamic = cpp_uninitialized();

class edge
{
  var y: dynamic = cpp_uninitialized();
  var i: dynamic = cpp_uninitialized();
  var nex: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(maxm);

var len: dynamic = cpp_uninitialized();

var fir: dynamic = cpp_array(maxn);

func ins(x: dynamic, y: dynamic, i: dynamic) -> dynamic
{
  a[cpp_update(len, "++")] = [y, i, fir[x]];
  fir[x] = len;
}

var dfn: dynamic = cpp_array(maxn);

var low: dynamic = cpp_array(maxn);

var dfi: dynamic = cpp_uninitialized();

var ib: dynamic = cpp_array(maxm);

var fa: dynamic = cpp_array(maxn);

var fai: dynamic = cpp_array(maxn);

func init() -> dynamic
{
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      dfn[i] = cpp_assign(fa[i], "=", cpp_assign(fai[i], "=", 0));
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      ib[i] = 0;
      i += 1;
    }
  }
  dfi = 0;
}

func tarjan(x: dynamic) -> dynamic
{
  dfn[x] = cpp_assign(low[x], "=", cpp_update(dfi, "++"));
  {
    var k: dynamic = fir[x];
    var y: dynamic = a[k].y;
    while (k)
    {
      if (((a[k].i != fai[x]) && (!ok[a[k].i])))
      {
        if ((!dfn[y]))
        {
          fai[y] = a[k].i;
          fa[y] = x;
          tarjan(y);
          down(low[x], low[y]);
          if ((low[y] == dfn[y]))
          {
            ib[a[k].i] = 1;
          }
        } else
        {
          down(low[x], dfn[y]);
        }
      }
      k = a[k].nex;
      y = a[k].y;
    }
  }
}

var ans: dynamic = cpp_uninitialized();

var ansn: dynamic = cpp_uninitialized();

var re: dynamic = cpp_array(10);

func main() -> dynamic
{
  scanf("%d%d", (&n), (&m));
  scanf("%d%d", (&S), (&T));
  {
    var i: dynamic = 1;
    while ((i <= m))
    {
      scanf("%d%d%d", (&e[i][0]), (&e[i][1]), (&e[i][2]));
      ins(e[i][0], e[i][1], i);
      ins(e[i][1], e[i][0], i);
      i += 1;
    }
  }
  init();
  tarjan(S);
  if ((!dfn[T]))
  {
    puts("0");
    puts("0");
    putchar(cpp_char("\n"));
    return 0;
  }
  ans = INT_MAX;
  {
    var i: dynamic = T;
    while ((i != S))
    {
      var k: dynamic = fai[i];
      if (ib[k])
      {
        if ((ans > e[k][2]))
        {
          ans = e[k][2];
          re[cpp_assign(ansn, "=", 1)] = k;
        }
      } else
      {
        t[cpp_update(tp, "++")] = k;
      }
      i = fa[i];
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= tp))
    {
      ok[t[i]] = 1;
      init();
      tarjan(S);
      {
        var j: dynamic = T;
        while ((j != S))
        {
          var k: dynamic = fai[j];
          if (ib[k])
          {
            if ((ans > (e[t[i]][2] + e[k][2])))
            {
              ans = (e[t[i]][2] + e[k][2]);
              ansn = 2;
              re[1] = t[i];
              re[2] = k;
            }
          }
          j = fa[j];
        }
      }
      ok[t[i]] = 0;
      i += 1;
    }
  }
  if ((ans == INT_MAX))
  {
    puts("-1");
  } else
  {
    printf("%d\n", ans);
    printf("%d\n", ansn);
    {
      var i: dynamic = 1;
      while ((i <= ansn))
      {
        printf("%d ", re[i]);
        i += 1;
      }
    }
  }
  return 0;
}
