// Translated from solution.cpp.

var val: dynamic = cpp_array(200005);

var e: dynamic = cpp_array(300005);

var del: dynamic = cpp_array(300005);

var qT: dynamic = cpp_array(500005);

var qV: dynamic = cpp_array(500005);

var color: dynamic = cpp_array(200005);

func repr(x: dynamic) -> dynamic
{
  return  ((color[x] == x)) ? x : (cpp_assign(color[x], "=", repr(color[x])));
}

var f: dynamic = cpp_array(200005);

var op: dynamic = cpp_array(500005);

func join(x: dynamic, y: dynamic, ind: dynamic) -> dynamic
{
  x = repr(x);
  y = repr(y);
  if ((x != y))
  {
    if ((f[x].size() < f[y].size()))
    {
      swap(x, y);
    }
    f[x].insert(f[x].end(), f[y].begin(), f[y].end());
    color[y] = x;
    if ((ind != -1))
    {
      op[ind] = [x, y];
    }
  }
}

var root: dynamic = cpp_array(200005);

var ff: dynamic = cpp_array(200005);

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var q: dynamic = cpp_uninitialized();
  scanf("%d%d%d", (&n), (&m), (&q));
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      read(val[i]);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      scanf("%d%d", (&e[i].first), (&e[i].second));
      e[i].first -= 1;
      e[i].second -= 1;
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      scanf("%d%d", (&qT[i]), (&qV[i]));
      qV[i] -= 1;
      if ((qT[i] == 2))
      {
        del[qV[i]] = true;
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      color[i] = i;
      f[i].push_back(i);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < m))
    {
      if (del[i])
      {
        i += 1;
        continue;
      }
      join(e[i].first, e[i].second, -1);
      i += 1;
    }
  }
  {
    var i: dynamic = (q - 1);
    while ((i >= 0))
    {
      if ((qT[i] == 2))
      {
        join(e[qV[i]].first, e[qV[i]].second, i);
      }
      i -= 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < n))
    {
      ff[repr(i)].insert([(-val[i]), i]);
      root[i] = repr(i);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < q))
    {
      if ((qT[i] == 1))
      {
        var p: dynamic = root[qV[i]];
        if ((ff[p].size() > 0))
        {
          printf("%d\n", (-ff[p].begin()->first));
          val[ff[p].begin()->second] = 0;
          ff[p].erase(ff[p].begin());
        } else
        {
          puts("0");
        }
      } else if ((op[i].first != op[i].second))
      {
        for (var t: dynamic in f[op[i].second])
        {
          root[t] = op[i].second;
          if ((val[t] > 0))
          {
            ff[op[i].first].erase([(-val[t]), t]);
            ff[op[i].second].insert([(-val[t]), t]);
          }
        }
      }
      i += 1;
    }
  }
  return 0;
}
