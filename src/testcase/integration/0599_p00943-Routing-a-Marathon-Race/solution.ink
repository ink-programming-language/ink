// Translated from solution.cpp.

var ll = dynamic;

class state
{
  var s: dynamic;
  var v: dynamic;
  var len: dynamic;
}

class comp
{
  func operator_call(a: dynamic, b: dynamic)
  {
      return (a.len > b.len);
    }
}

var que: dynamic;

var n: dynamic;

var m: dynamic;

var c = cpp_array(50);

var ans: dynamic;

var G = cpp_array(50);

var d = cpp_array(50);

var mask = cpp_array(50);

func add_edge(f: dynamic, t: dynamic)
{
  G[f].push_back(t);
  G[t].push_back(f);
}

func main()
{
  scanf("%d%d", (&n), (&m));
  {
    var i = 1;
    while ((i <= n))
    {
      scanf("%d", (&c[i]));
      i += 1;
    }
  }
  {
    var i = 1;
    var a: dynamic;
    var b: dynamic;
    while ((i <= m))
    {
      scanf("%d%d", (&a), (&b));
      add_edge(a, b);
      mask[a] |= (1 << b);
      mask[b] |= (1 << a);
      i += 1;
    }
  }
  {
    var i = 1;
    while ((i <= n))
    {
      mask[i] |= (1 << i);
      if ((mask[1] & ((1 << i))))
      {
        ans += c[i];
      }
      i += 1;
    }
  }
  que.push([mask[1], 1, ans]);
  d[1][mask[1]] = ans;
  while ((!que.empty()))
  {
    var S = que.top();
    que.pop();
    if ((S.v == n))
    {
      printf("%d\n", S.len);
      return 0;
    }
    {
      var i = 0;
      while ((i < G[S.v].size()))
      {
        var New = [0, 0, 0];
        New.v = G[S.v][i];
        New.s = (S.s | mask[New.v]);
        {
          var j = 1;
          while ((j <= n))
          {
            if ((New.s & ((1 << j))))
            {
              New.len += c[j];
            }
            j += 1;
          }
        }
        if (((d[New.v].find(New.s) == d[New.v].end()) || (d[New.v][New.s] > New.len)))
        {
          que.push(New);
          d[New.v][New.s] = New.len;
        }
        i += 1;
      }
    }
  }
  return 0;
}
