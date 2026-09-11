// Translated from solution.cpp.

var mp: dynamic = cpp_expression("#include<");

var fi: dynamic = cpp_expression("#incl");

var se: dynamic = cpp_expression("#inclu");

var MAXN: dynamic = 200000;

var que: dynamic = cpp_uninitialized();

var lst: dynamic = cpp_array((MAXN + 5));

var nxt: dynamic = cpp_array((MAXN + 5));

var N: dynamic = cpp_uninitialized();

var L: dynamic = cpp_uninitialized();

func link(x: dynamic, y: dynamic) -> dynamic
{
  lst[y] = x;
  nxt[x] = y;
}

func check(x: dynamic, y: dynamic) -> dynamic
{
  return (nxt[x] == y);
}

var lf: dynamic = cpp_array((MAXN + 5));

var rf: dynamic = cpp_array((MAXN + 5));

var f1: dynamic = cpp_array((MAXN + 5));

var f2: dynamic = cpp_array((MAXN + 5));

var v1: dynamic = cpp_uninitialized();

var v2: dynamic = cpp_uninitialized();

func solve(x: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  var tmp: dynamic = 0;
  var lt: dynamic = v2.size();
  var lb: dynamic = lst[v2[0]];
  var rb: dynamic = nxt[v2[(lt - 1)]];
  {
    var i: dynamic = 0;
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
    var i: dynamic = 0;
    while ((i < lt))
    {
      f1[v2[i]] = lf[v2[i]];
      f2[v2[i]] = rf[v2[i]];
      lf[v2[i]] = cpp_assign(rf[v2[i]], "=", 0);
      i += 1;
    }
  }
  var c: dynamic = (lt / L);
  if (c)
  {
    {
      var i: dynamic = (L - 1);
      while ((i < lt))
      {
        var t: dynamic = ((((i + 1)) / L) - 1);
        rf[v2[t]] += f2[v2[i]];
        i += 1;
      }
    }
    {
      var i: dynamic = (lt - L);
      while ((i >= 0))
      {
        var t: dynamic = (c - (((lt - i)) / L));
        lf[v2[t]] += f1[v2[i]];
        i -= 1;
      }
    }
    {
      var i: dynamic = 1;
      while ((i < c))
      {
        link(v2[(i - 1)], v2[i]);
        i += 1;
      }
    }
    link(lb, v2[0]);
    link(v2[(c - 1)], rb);
    {
      var i: dynamic = 0;
      while ((i < c))
      {
        que.push(mp((x + 1), v2[i]));
        i += 1;
      }
    }
    tmp = 0;
    {
      var i: dynamic = 0;
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

func main() -> dynamic
{
  scanf("%d%d", (&N), (&L));
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      var x: dynamic = cpp_uninitialized();
      scanf("%d", (&x));
      que.push(mp(x, i));
      link(i, (i + 1));
      lf[i] = cpp_assign(rf[i], "=", 1);
      i += 1;
    }
  }
  var ans: dynamic = 0;
  link(0, 1);
  while ((!que.empty()))
  {
    var x: dynamic = que.top().fi;
    v1.clear();
    while (((!que.empty()) && (que.top().fi == x)))
    {
      v1.push_back(que.top().se);
      que.pop();
    }
    v2.clear();
    v2.push_back(v1[0]);
    {
      var i: dynamic = 1;
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
