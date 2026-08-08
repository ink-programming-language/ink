// Translated from solution.cpp.

var a14: dynamic;

func rd(l: dynamic, r: dynamic)
{
  return ((rand() % (((r - l) + 1))) + l);
}

var mxn = (1e5 + 3);

var sq = 333;

var to = cpp_array(mxn);

var nxt = cpp_array(mxn);

var fir = cpp_array(mxn);

var gn = 1;

var n: dynamic;

var fa = cpp_array(mxn);

var dep = cpp_array(mxn);

var siz = cpp_array(mxn);

var dfn = cpp_array(mxn);

var dn: dynamic;

var top = cpp_array(mxn);

var zs = cpp_array(mxn);

var dfa = cpp_array(mxn);

var va = cpp_array(mxn);

var m: dynamic;

var xw = cpp_array(mxn);

var ans: dynamic;

var ux = cpp_array(mxn);

var un: dynamic;

var stk = cpp_array(mxn);

var sn: dynamic;

var fb = cpp_array(mxn);

var sg = cpp_array(mxn);

var co = cpp_array(mxn);

func gadd(x: dynamic, y: dynamic)
{
  to[cpp_update(gn, "++")] = y;
  nxt[gn] = fir[x];
  fir[x] = gn;
}

func dfs0(x: dynamic, f: dynamic)
{
  fa[x] = f;
  dep[x] = (dep[f] + 1);
  siz[x] = 1;
  {
    var i = fir[x];
    while (i)
    {
      dfs0(to[i], x);
      siz[x] += siz[to[i]];
      if ((siz[to[i]] > siz[zs[x]]))
      {
        zs[x] = to[i];
      }
      i = nxt[i];
    }
  }
}

func dfs1(x: dynamic, f: dynamic)
{
  dfn[x] = cpp_update(dn, "++");
  dfa[dn] = x;
  top[x] = if ((x == zs[f])) top[f] else x;
  if (zs[x])
  {
    dfs1(zs[x], x);
  }
  {
    var i = fir[x];
    while (i)
    {
      if ((to[i] != zs[x]))
      {
        dfs1(to[i], x);
      }
      i = nxt[i];
    }
  }
}

func lca(x: dynamic, y: dynamic)
{
  while ((top[x] != top[y]))
  {
    if ((dep[top[x]] < dep[top[y]]))
    {
      swap(x, y);
    }
    x = fa[top[x]];
  }
  return if ((dep[x] < dep[y])) x else y;
}

func cmp1(x: dynamic, y: dynamic)
{
  return (dfn[x] < dfn[y]);
}

var lb = cpp_array(mxn);

var zp = cpp_array(mxn);

var sf = cpp_array(mxn);

var xl = cpp_array(mxn);

func main()
{
  a14 = scanf("%d%d", (&n), (&m));
  {
    var i = 2;
    var x: dynamic;
    while ((i <= n))
    {
      a14 = scanf("%d", (&x));
      gadd(x, i);
      i += 1;
    }
  }
  dfs0(1, 0);
  dfs1(1, 0);
  {
    var i = 1;
    var x: dynamic;
    while ((i <= n))
    {
      a14 = scanf("%d", (&x));
      va[i] = (-x);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= m))
    {
      a14 = scanf("%d", (xw + i));
      i += 1;
    }
  }
  assert((dn == n));
  {
    var L = 1;
    var R: dynamic;
    while ((R != m))
    {
      R = min(m, ((L + sq) - 1));
      {
        var i = L;
        while ((i <= R))
        {
          ux[cpp_update(un, "++")] = abs(xw[i]);
          i += 1;
        }
      }
      sort((ux + 1), ((ux + un) + 1), cmp1);
      {
        var i = (un - 1);
        while (i)
        {
          ux[cpp_update(un, "++")] = lca(ux[i], ux[(i + 1)]);
          i -= 1;
        }
      }
      sort((ux + 1), ((ux + un) + 1), cmp1);
      un = ((unique((ux + 1), ((ux + un) + 1)) - ux) - 1);
      stk[cpp_assign(sn, "=", 1)] = ux[1];
      {
        var i = 2;
        while ((i <= un))
        {
          var x = ux[i];
          var y: dynamic;
          while (cpp_comma(cpp_assign(y, "=", stk[sn]), (dfn[x] > ((dfn[y] + siz[y]) - 1))))
          {
            sn -= 1;
          }
          fb[x] = y;
          stk[cpp_update(sn, "++")] = x;
          i += 1;
        }
      }
      {
        var i = 1;
        while ((i <= sn))
        {
          fb[stk[i]] = stk[(i - 1)];
          i += 1;
        }
      }
      {
        var T = 1;
        while ((T <= un))
        {
          var x = ux[T];
          var tn = 0;
          {
            var y = x;
            while ((y != fb[x]))
            {
              xl[cpp_update(tn, "++")] = pair(va[y], y);
              y = fa[y];
            }
          }
          sort((xl + 1), ((xl + tn) + 1));
          {
            var l = 1;
            var r: dynamic;
            var c: dynamic;
            while ((l <= tn))
            {
              r = l;
              while (((r < tn) && (xl[r].first == xl[(r + 1)].first)))
              {
                r += 1;
              }
              c = 0;
              {
                var i = l;
                while ((i <= r))
                {
                  c += (co[xl[i].second] ^ 1);
                  i += 1;
                }
              }
              lb[x].push_back(pair(xl[r].first, c));
              l = (r + 1);
            }
          }
          zp[x] = upper_bound(lb[x].begin(), lb[x].end(), pair(0, 1e9));
          sf[x] = lower_bound(lb[x].begin(), lb[x].end(), pair(va[x], -1e9));
          T += 1;
        }
      }
      {
        var T = L;
        while ((T <= R))
        {
          var x = xw[T];
          if ((x > 0))
          {
            assert((!co[x]));
          } else
          {
            assert(co[(-x)]);
          }
          if ((x > 0))
          {
            co[x] ^= 1;
            {
              var y = x;
              while (y)
              {
                sg[y] += 1;
                var p = zp[y];
                if (((p != lb[y].begin()) && ((((p - 1))->first + sg[y]) > 0)))
                {
                  p -= 1;
                  ans += p->second;
                }
                y = fb[y];
              }
            }
            sf[x]->second -= 1;
            if (((va[x] + sg[x]) > 0))
            {
              ans -= 1;
            }
          } else
          {
            x = (-x);
            co[x] ^= 1;
            {
              var y = x;
              while (y)
              {
                sg[y] -= 1;
                var p = zp[y];
                if (((p != lb[y].end()) && ((p->first + sg[y]) <= 0)))
                {
                  ans -= p->second;
                  p += 1;
                }
                y = fb[y];
              }
            }
            sf[x]->second += 1;
            if (((va[x] + sg[x]) > 0))
            {
              ans += 1;
            }
          }
          printf("%d ", ans);
          T += 1;
        }
      }
      {
        var i = 1;
        var x: dynamic;
        while ((i <= un))
        {
          x = ux[i];
          {
            var j = x;
            while ((j != fb[x]))
            {
              va[j] += sg[x];
              j = fa[j];
            }
          }
          {
            var WH: dynamic;
            swap(lb[x], WH);
          }
          sg[x] = 0;
          i += 1;
        }
      }
      un = 0;
      L = (R + 1);
    }
  }
  puts("");
  return 0;
}
