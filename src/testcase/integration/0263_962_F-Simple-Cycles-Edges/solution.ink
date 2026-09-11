// Translated from solution.cpp.

var N: dynamic = 300005;

class edge
{
  var to: dynamic = cpp_uninitialized();
  var next: dynamic = cpp_uninitialized();
}

var e: dynamic = cpp_array((N << 1));

var h: dynamic = cpp_array(N);

var xb: dynamic = cpp_uninitialized();

var dfn: dynamic = cpp_array(N);

var low: dynamic = cpp_array(N);

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

var x: dynamic = cpp_uninitialized();

var y: dynamic = cpp_uninitialized();

var w: dynamic = cpp_uninitialized();

var stx: dynamic = cpp_array(N);

var sty: dynamic = cpp_array(N);

var ste: dynamic = cpp_array(N);

var ans: dynamic = cpp_uninitialized();

var b: dynamic = cpp_array(N);

var cant: dynamic = cpp_array(N);

func addedge(x: dynamic, y: dynamic) -> dynamic
{
  e[cpp_update(xb, "++")] = [y, h[x]];
  h[x] = xb;
  e[cpp_update(xb, "++")] = [x, h[y]];
  h[y] = xb;
}

func dfs(x: dynamic, fa: dynamic) -> dynamic
{
  dfn[x] = cpp_assign(low[x], "=", cpp_update(xb, "++"));
  var i: dynamic = h[x];
  var j: dynamic = cpp_uninitialized();
  {
    while (i)
    {
      if ((e[i].to != fa))
      {
        if ((!dfn[e[i].to]))
        {
          stx[cpp_update(w, "++")] = x;
          sty[w] = e[i].to;
          ste[w] = i;
          dfs(e[i].to, x);
          if ((low[e[i].to] < low[x]))
          {
            low[x] = low[e[i].to];
          }
          if ((low[e[i].to] >= dfn[x]))
          {
            var cnt: dynamic = 0;
            var ow: dynamic = w;
            {
              while (((stx[w] != x) || (sty[w] != e[i].to)))
              {
                if ((!b[stx[w]]))
                {
                  cnt += 1;
                  b[stx[w]] = 1;
                }
                if ((!b[sty[w]]))
                {
                  cnt += 1;
                  b[sty[w]] = 1;
                }
                w -= 1;
              }
            }
            if ((!b[stx[w]]))
            {
              cnt += 1;
              b[stx[w]] = 1;
            }
            if ((!b[sty[w]]))
            {
              cnt += 1;
              b[sty[w]] = 1;
            }
            if (((((ow - w) + 1) > cnt) || (cnt == 2)))
            {
              {
                j = w;
                while ((j <= ow))
                {
                  cant[(ste[j] >> 1)] = 1;
                  j += 1;
                }
              }
            }
            {
              j = w;
              while ((j <= ow))
              {
                b[stx[j]] = cpp_assign(b[sty[j]], "=", 0);
                j += 1;
              }
            }
            w -= 1;
          }
        } else
        {
          if ((dfn[e[i].to] < dfn[x]))
          {
            stx[cpp_update(w, "++")] = x;
            sty[w] = e[i].to;
            ste[w] = i;
          }
          if ((dfn[e[i].to] < low[x]))
          {
            low[x] = dfn[e[i].to];
          }
        }
      }
      i = e[i].next;
    }
  }
}

func main() -> dynamic
{
  read(n, m);
  xb = 1;
  {
    i = 1;
    while ((i <= m))
    {
      read(x, y);
      addedge(x, y);
      i += 1;
    }
  }
  xb = 0;
  {
    i = 1;
    while ((i <= n))
    {
      if ((!dfn[i]))
      {
        dfs(i, 0);
      }
      i += 1;
    }
  }
  {
    i = 1;
    while ((i <= m))
    {
      if ((!cant[i]))
      {
        ans.push_back(i);
      }
      i += 1;
    }
  }
  write(ans.size(), cpp_char("\n"));
  {
    i = 0;
    while ((i < int_cpp(ans.size())))
    {
      write(ans[i], cpp_char(" "));
      i += 1;
    }
  }
  return 0;
}
