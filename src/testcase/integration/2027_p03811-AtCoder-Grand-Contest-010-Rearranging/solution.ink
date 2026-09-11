// Translated from solution.cpp.

var N: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(2001);

var G: dynamic = cpp_array(2001, 2001);

var q: dynamic = cpp_array(2002);

var ue: dynamic = cpp_array(2002);

var cpp_ref: dynamic = cpp_array(2002);

var v: dynamic = cpp_array(2001);

var L: dynamic = cpp_uninitialized();

var d: dynamic = cpp_array(2001);

var E: dynamic = cpp_array(2001);

func gcd(a: dynamic, b: dynamic) -> dynamic
{
  var r: dynamic = cpp_uninitialized();
  while (b)
  {
    r = (a % b);
    a = b;
    b = r;
  }
  return a;
}

func main() -> dynamic
{
  scanf("%d", (&N));
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      scanf("%d", (a + i));
      i += 1;
    }
  }
  sort((a + 1), ((a + N) + 1));
  {
    var i: dynamic = 1;
    while ((i < N))
    {
      {
        var j: dynamic = (i + 1);
        while ((j <= N))
        {
          if ((gcd(a[i], a[j]) != 1))
          {
            G[i][j] = cpp_assign(G[j][i], "=", 1);
          }
          j += 1;
        }
      }
      i += 1;
    }
  }
  {
    var i: dynamic = 1;
    while ((i <= N))
    {
      G[0][i] = cpp_assign(G[i][0], "=", 1);
      i += 1;
    }
  }
  var D: dynamic = 1;
  q[1] = 0;
  ue[1] = 0;
  v[0] = 1;
  cpp_ref[cpp_assign(L, "=", 1)] = 0;
  while (D)
  {
    while (((ue[D] <= N) && (((!G[q[D]][ue[D]]) || v[ue[D]]))))
    {
      ue[D] += 1;
    }
    if ((ue[D] <= N))
    {
      var To: dynamic = cpp_update(ue[D], "++");
      E[q[D]].push_back(To);
      d[To] += 1;
      q[cpp_update(D, "++")] = To;
      ue[D] = 0;
      v[cpp_assign(cpp_ref[cpp_update(L, "++")], "=", To)] = 1;
    } else
    {
      D -= 1;
    }
  }
  var O: dynamic = cpp_uninitialized();
  var S: dynamic = cpp_uninitialized();
  {
    var i: dynamic = 0;
    while ((i <= N))
    {
      if ((!d[i]))
      {
        S.insert(i);
      }
      i += 1;
    }
  }
  while ((!S.empty()))
  {
    var it: dynamic = S.end();
    var u: dynamic = (*cpp_update(it, "--"));
    S.erase(it);
    O.push_back(u);
    for (var v: dynamic in E[u])
    {
      if ((!cpp_update(d[v], "--")))
      {
        S.insert(v);
      }
    }
  }
  for (var i: dynamic in O)
  {
    if (i)
    {
      printf("%d ", a[i]);
    }
  }
  puts("");
  return 0;
}
