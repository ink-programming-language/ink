// Translated from solution.cpp.

var mp = cpp_expression("#include<");

var fi = cpp_expression("#incl");

var se = cpp_expression("#inclu");

var MAXN = 200000;

var que: dynamic;

var lst = cpp_array((MAXN + 5));

var nxt = cpp_array((MAXN + 5));

var N: dynamic;

var L: dynamic;

func link(x: dynamic, y: dynamic)
{
  lst[y] = x;
  nxt[x] = y;
}

func check(x: dynamic, y: dynamic)
{
  return (nxt[x] == y);
}

var lf = cpp_array((MAXN + 5));

var rf = cpp_array((MAXN + 5));

var f1 = cpp_array((MAXN + 5));

var f2 = cpp_array((MAXN + 5));

var v1: dynamic;

var v2: dynamic;

func solve(x: dynamic)
{
  var ret = 0;
  var tmp = 0;
  var lt = v2.size();
  var lb = lst[v2[0]];
  var rb = nxt[v2[(lt - 1)]];
  {
    var i = 0;
    while ((i < lt))
    {
      if ((((i - L) + 1) >= 0))
      {
        tmp += lf[v2[((i - L) + 1)]];
      }
      ret += (tmp * rf[v2[i]]);
      i += 1;
    }
  }
  {
    var i = 0;
    while ((i < lt))
    {
      f1[v2[i]] = lf[v2[i]];
      f2[v2[i]] = rf[v2[i]];
      lf[v2[i]] = cpp_assign(rf[v2[i]], "=", 0);
      i += 1;
    }
  }
  var c = (lt / L);
  if (c)
  {
    {
      var i = (L - 1);
      while ((i < lt))
      {
        var t = ((((i + 1)) / L) - 1);
        rf[v2[t]] += f2[v2[i]];
        i += 1;
      }
    }
    {
      var i = (lt - L);
      while ((i >= 0))
      {
        var t = (c - (((lt - i)) / L));
        lf[v2[t]] += f1[v2[i]];
        i -= 1;
      }
    }
    {
      var i = 1;
      while ((i < c))
      {
        link(v2[(i - 1)], v2[i]);
        i += 1;
      }
    }
    link(lb, v2[0]);
    link(v2[(c - 1)], rb);
    {
      var i = 0;
      while ((i < c))
      {
        que.push(mp((x + 1), v2[i]));
        i += 1;
      }
    }
    tmp = 0;
    {
      var i = 0;
      while ((i < c))
      {
        if ((((i - L) + 1) >= 0))
        {
          tmp += lf[v2[((i - L) + 1)]];
        }
        ret -= (tmp * rf[v2[i]]);
        i += 1;
      }
    }
  } else
  {
    nxt[lb] = (N + 1);
    lst[rb] = 0;
  }
  v2.clear();
  return ret;
}

func main()
{
  scanf("%d%d", (&N), (&L));
  {
    var i = 1;
    while ((i <= N))
    {
      var x: dynamic;
      scanf("%d", (&x));
      que.push(mp(x, i));
      link(i, (i + 1));
      lf[i] = cpp_assign(rf[i], "=", 1);
      i += 1;
    }
  }
  var ans = 0;
  link(0, 1);
  while ((!que.empty()))
  {
    var x = que.top().fi;
    v1.clear();
    while (((!que.empty()) && (que.top().fi == x)))
    {
      v1.push_back(que.top().se);
      que.pop();
    }
    v2.clear();
    v2.push_back(v1[0]);
    {
      var i = 1;
      while ((i < v1.size()))
      {
        if ((!check(v1[(i - 1)], v1[i])))
        {
          ans += solve(x);
        }
        v2.push_back(v1[i]);
        i += 1;
      }
    }
    ans += solve(x);
  }
  printf("%lld\n", (ans + N));
}
