// Translated from solution.cpp.

var N = 300005;

class edge
{
  var to: dynamic;
  var next: dynamic;
}

var e = cpp_array((N << 1));

var h = cpp_array(N);

var xb: dynamic;

var dfn = cpp_array(N);

var low = cpp_array(N);

var n: dynamic;

var m: dynamic;

var i: dynamic;

var j: dynamic;

var x: dynamic;

var y: dynamic;

var w: dynamic;

var stx = cpp_array(N);

var sty = cpp_array(N);

var ste = cpp_array(N);

var ans: dynamic;

var b = cpp_array(N);

var cant = cpp_array(N);

func addedge(x: dynamic, y: dynamic)
{
  e[cpp_update(xb, "++")] = [y, h[x]];
  h[x] = xb;
  e[cpp_update(xb, "++")] = [x, h[y]];
  h[y] = xb;
}

func dfs(x: dynamic, fa: dynamic)
{
  dfn[x] = cpp_assign(low[x], "=", cpp_update(xb, "++"));
  var i = h[x];
  var j: dynamic;
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
            var cnt = 0;
            var ow = w;
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

func main()
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
