// Translated from solution.cpp.

var N: dynamic;

var a = cpp_array(2001);

var G = cpp_array(2001, 2001);

var q = cpp_array(2002);

var ue = cpp_array(2002);

var ref = cpp_array(2002);

var v = cpp_array(2001);

var L: dynamic;

var d = cpp_array(2001);

var E = cpp_array(2001);

func gcd(a: dynamic, b: dynamic)
{
  var r: dynamic;
  while (b)
  {
    r = (a % b);
    a = b;
    b = r;
  }
  return a;
}

func main()
{
  scanf("%d", (&N));
  {
    var i = 1;
    while ((i <= N))
    {
      scanf("%d", (a + i));
      i += 1;
    }
  }
  sort((a + 1), ((a + N) + 1));
  {
    var i = 1;
    while ((i < N))
    {
      {
        var j = (i + 1);
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
    var i = 1;
    while ((i <= N))
    {
      G[0][i] = cpp_assign(G[i][0], "=", 1);
      i += 1;
    }
  }
  var D = 1;
  q[1] = 0;
  ue[1] = 0;
  v[0] = 1;
  ref[cpp_assign(L, "=", 1)] = 0;
  while (D)
  {
    while (((ue[D] <= N) && (((!G[q[D]][ue[D]]) || v[ue[D]]))))
    {
      ue[D] += 1;
    }
    if ((ue[D] <= N))
    {
      var To = cpp_update(ue[D], "++");
      E[q[D]].push_back(To);
      d[To] += 1;
      q[cpp_update(D, "++")] = To;
      ue[D] = 0;
      v[cpp_assign(ref[cpp_update(L, "++")], "=", To)] = 1;
    } else
    {
      D -= 1;
    }
  }
  var O: dynamic;
  var S: dynamic;
  {
    var i = 0;
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
    var it = S.end();
    var u = (*cpp_update(it, "--"));
    S.erase(it);
    O.push_back(u);
    for (var v in E[u])
    {
      if ((!cpp_update(d[v], "--")))
      {
        S.insert(v);
      }
    }
  }
  for (var i in O)
  {
    if (i)
    {
      printf("%d ", a[i]);
    }
  }
  puts("");
  return 0;
}
